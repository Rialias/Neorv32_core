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

size_t message_length;
bool status;
char token[10];
bool send_request(int fd, int request_type);
bool receive_response(pb_byte_t *message, size_t length);

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
    tty.c_cc[VMIN] = 30;

    cfsetispeed(&tty, B19200);
    cfsetospeed(&tty, B19200);

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }
    while (1)
    {
        int request_type = 0;
        printf("\nChoose an option: \n1. Get Device Info \n2.Claim the device \n3.Reclaim the device \n4.Unlcaim the device \n5.set LEd \n");
        scanf("%d", &request_type);
        send_request(serial_port, request_type);

        /* Clear the input buffer*/
        if (tcflush(serial_port, TCIFLUSH) != 0)
        {
            perror("tcflush");
            close(serial_port);
            return -1;
        }
        char key = 'y';
        do


        {
            int num_bytes = read(serial_port, &key, 1);
            if (num_bytes < 0)
            {
                printf("Error reading: %s\n", strerror(errno));
                return 1;
            }
        } while (key != 'a');
        char message_length = 'z';
        read(serial_port, &message_length, sizeof(message_length));
        printf("Received message Length: %d\n", message_length);
        size_t length = (size_t)message_length;
        char read_buf[length+3];
        uint8_t buffer[length];

        int num_bytes = read(serial_port, &read_buf, length+3);
        if (num_bytes <= 0)
        {
            printf("Error reading: %s", strerror(errno));
            return 1;
        }
        int i;
        int j = 0;
        for (i = 0; i < length+3; i++)
        {
            if ((uint8_t)read_buf[i] != 13)
            {
                buffer[j++] = (uint8_t)read_buf[i];
            }
        }
        receive_response(buffer, length);
    }
    close(serial_port);
    return 0;
}

bool send_request(int fd, int request_type)
{
    size_t i;
    char msg_buffer[256];
    size_t j = 0;
    uint8_t buffer[256];
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
        int32_t id;
        uint32_t color;
        printf("1. Enter the color to set \n");
        scanf("%x", &color);
        printf("2. Enter the id to set \n");
        scanf("%d", &id);
        request.request_type.set_smartled.color = color;
        request.request_type.set_smartled.id = id;
        strcpy(request.request_type.set_smartled.token, token);
    }
    else
    {
        printf("Wrong option \n");
        return 1;
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
        j += sprintf(&msg_buffer[j], "%d ", buffer[i]);
    }
    msg_buffer[j - 1] = '\r';

    write(fd, msg_buffer, strlen(msg_buffer));

    printf("\n");
    printf("Sucessfully sent data. Message length = %ld \n", message_length);
    return 0;
}

bool receive_response(pb_byte_t *message, size_t length)
{
    Response response = Response_init_zero;
    pb_istream_t istream = pb_istream_from_buffer(message, length);
    if (!pb_decode_delimited(&istream, Response_fields, &response))
    {
        printf("Failed to decode response: %s\n", PB_GET_ERROR(&istream));
        return 1;
    }
    printf("\n");
    if (response.which_response_type == Response_get_device_info_tag)
    {
        printf("Device Info\n");
        printf("Type = %s \n", response.response_type.get_device_info.type);
        printf("Product: %s\n", response.response_type.get_device_info.product);
        printf("IP: %s\n", response.response_type.get_device_info.ip);
    }
    else if (response.which_response_type == Response_claim_tag)
    {
        printf("claim\n");
        printf("Token : %s\n", response.response_type.claim.token);
        strcpy(token, response.response_type.claim.token);
    }
    else if (response.which_response_type == Response_reclaim_tag)
    {
        printf("Reclaimed the device\n");
        strcpy(token, response.response_type.reclaim.token);
        printf("Token : %s\n", response.response_type.reclaim.token);       
    }
    else if (response.which_response_type == Response_unclaim_tag)
    {
        printf("Unclaimed the device \n");
        printf("Token : %s\n", response.response_type.unclaim.token);
        strcpy(token, response.response_type.unclaim.token);
    }
    else if (response.which_response_type == Response_led_tag)
    {
        printf("Set Neopixel LED \n");
        /*printf("Token : %s\n", response.response_type.led.token); */
        if (strcmp(token, response.response_type.led.token))
        {
            printf("Token doesnt match, LED is not set \n");
        }
    }
    else
    {
        printf("Wrong Response \n");
    }
    return 0;
}