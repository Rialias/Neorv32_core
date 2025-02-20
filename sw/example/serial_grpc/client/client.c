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
/* bool send_device_info_request(int fd); */
/* bool get_device_info_response(char* message_buf); */

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
    tty.c_cc[VMIN] = 24;

    cfsetispeed(&tty, B19200);
    cfsetospeed(&tty, B19200);

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    char read_buf[256];
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
    printf("Received 'a', continuing to read data...\n");
    memset(&read_buf, '\0', sizeof(read_buf));

    for (;;)
    {
        int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
        if (num_bytes < 0)
        {
            printf("Error reading: %s", strerror(errno));
            return 1;
        }
        /* printf("Read %i bytes. Received message:%s", num_bytes, read_buf);*/
        int i;
        int j = 0; 
        for (i = 0; i < num_bytes; i++)
        {
            if ((uint8_t)read_buf[i] != 13)
            { 
                buffer[j++] = (uint8_t)read_buf[i];
            }
        }

        for (i = 0; i < num_bytes; i++)
        {
            printf("%d", (uint8_t)read_buf[i]);
        }
        printf("\n");
        for (i = 0; i < 25; i++)
        {
            printf("%d", buffer[i]);
        }
        get_device_infoResponse response = get_device_infoResponse_init_zero;
        pb_istream_t message_stream = pb_istream_from_buffer(buffer, 24);
        if (!pb_decode(&message_stream, get_device_infoResponse_fields, &response))
        {
            printf("Failed to decode response: %s\n", PB_GET_ERROR(&message_stream));
        }
        else
        {
            printf("Device type: %s\n", response.type);
            printf("Product: %s\n", response.product);
            printf("IP: %s\n", response.ip);
        }
    }

    close(serial_port);
    return 0;
}
