#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <pb_encode.h>
#include <pb_decode.h>

#include "fileproto.pb.h"

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 256

/**********************************************************************/
/**
 * @name User configuration
 **************************************************************************/
/* This is the buffer where we will store our message. */
uint8_t buffer[BUFFER_SIZE];
size_t message_length;
bool status;
bool send_request(int fd, int request_type);

/**********************************************************************/
/**
 * Main function; prints some fancy stuff via UART.
 *
 * @note This program requires the UART interface to be synthesized.
 *
 * @return 0 if execution was successful
 **************************************************************************/
int main()
{

    int serial_port = open("/dev/ttyUSB3", O_RDWR);
    struct termios tty;

    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;

    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;

    cfsetispeed(&tty, B19200);
    cfsetospeed(&tty, B19200);

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    int request_type = 0;
    printf("1. Get Device Info \n 2. Claim the device \n 3. Reclaim the device \n 4. Unlcaim the device \n 5. set LEd \n");
    scanf("%d", &request_type);

    send_request(serial_port, request_type);
    close(serial_port);
    return 0;
}

bool send_request(int fd, int request_type)
{
    size_t i;
    char msg_buffer[128];
    size_t j = 0;
    uint8_t buffer[128];
    Request request = Request_init_zero;
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    
    if (request_type == 1)
    {
        request.which_request_type = Request_get_device_info_tag;
    }
    else if (request_type == 2)
    {
        request.which_request_type = Request_claim_tag;
    }
    else if (request_type == 3)
    {
        request.which_request_type = Request_reclaim_tag;
    }
    else if (request_type == 4)
    {
        request.which_request_type = Request_unclaim_tag;
    }
    else if (request_type == 5)
    {
        request.which_request_type = Request_set_smartled_tag;
        char token[20];
        int32_t id;
        uint32_t color;
        printf("1. Enter the color to set \n");
        scanf("%d", &color);
        printf("1. Enter the id to set \n");
        scanf("%d", &id);
        printf("1. Enter the token \n");
        scanf("%s", token);
        request.request_type.set_smartled.color = color;
        request.request_type.set_smartled.id = id;
        strcpy(request.request_type.set_smartled.token, token);
    }
    else
    {
        printf("Wrong option \n");
    }

    status = pb_encode_delimited(&ostream, Request_fields, &request);
    message_length = ostream.bytes_written;
    if (!status)
    {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&ostream));
        return 1;
    }
    
    for (i = 0; i < message_length; i++)
    {
        printf("%d\n", buffer[i]);
    }
    printf("\n");
    for (i = 0; i < message_length; i++)
    {
        j += sprintf(&msg_buffer[j],"%d ", buffer[i]);
    }
    msg_buffer[j-1] = '\0';
    printf("j value = %ld\n", j);
    int written = write(fd, msg_buffer, strlen(msg_buffer));
    printf("written = %d\n", written);
    printf("send value = %s\n", msg_buffer);

    printf("\n");
    printf("Sucessfully sent data. Message length = %ld \n", message_length);

    /*pb_istream_t istream = pb_istream_from_buffer(buffer, message_length);
    Request request1 = Request_init_zero;
    Response response = Response_init_zero;

    if (!pb_decode_delimited(&istream, Request_fields, &request1))
    {
        printf("Failed to decode response: %s\n", PB_GET_ERROR(&istream));
        return 1;
    }

    if (request1.which_request_type == Request_get_device_info_tag)
    {
        printf("Device Info\n");
        strcpy(response.response_type.get_device_info.type, "x3xx");
        strcpy(response.response_type.get_device_info.product, "X310");
        strcpy(response.response_type.get_device_info.ip, "192.10.1.1");
        response.which_response_type = Response_get_device_info_tag;
    }
    else if (request1.which_request_type == Request_claim_tag)
    {
        printf("claim\n");
        strcpy(response.response_type.claim.token, "password");
        response.which_response_type = Response_claim_tag;
    }
    else if (request1.which_request_type == Request_reclaim_tag)
    {
        printf("reclaim\n");
        strcpy(response.response_type.claim.token, "password");
        response.which_response_type = Response_claim_tag;
    }
    else if (request1.which_request_type == Request_unclaim_tag)
    {
        printf("unclaim\n");
        strcpy(response.response_type.claim.token, "0000");
        response.which_response_type = Response_claim_tag;
    }
    else if (request1.which_request_type == Request_set_smartled_tag)
    {
        printf("samrtled\n");
    }
    else
    {
        printf("Wrong request \n");
    } */
    return 0;
}