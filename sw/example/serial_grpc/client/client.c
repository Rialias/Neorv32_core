#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <pb_encode.h>
#include <pb_decode.h>

#include "fileproto.pb.h"

#include <fcntl.h>
#include <errno.h>
#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**********************************************************************/
/**
 * @name User configuration
 **************************************************************************/
/* This is the buffer where we will store our message. */
uint8_t buffer[256];
size_t message_length;
bool status;

bool send_device_info_request(int fd);
bool get_device_info_response(uint8_t *buffer, size_t length);

void setup_serial(int fd)
{
    struct termios2 tty;
    if (ioctl(fd, TCGETS2, &tty) != 0)
    {
        printf("Error %d from ioctl TCGETS2: %s\n", errno, strerror(errno));
        return;
    }

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;
    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;
    tty.c_ispeed = 19200;
    tty.c_ospeed = 19200;

    if (ioctl(fd, TCSETS2, &tty) != 0)
    {
        printf("Error %d from ioctl TCSETS2: %s\n", errno, strerror(errno));
        return;
    }
}

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

    int fd = open("/dev/ttyUSB0", O_RDWR);
    if (fd < 0)
    {
        printf("Error %d opening /dev/ttyUSB0: %s\n", errno, strerror(errno));
        return -1;
    }

    setup_serial(fd);
    printf("Serial Communication set successfully \n");

    send_device_info_request(fd);

    uint8_t read_buf[256];
    while (1)
    {

        int num_bytes = read(fd, &read_buf, sizeof(read_buf));
        if (num_bytes > 0)
        {
            get_device_info_response(read_buf, num_bytes);
        }
        else
        {
            printf("Error reading: %s", strerror(errno));
        }
    }

    return 0;
}

bool send_device_info_request(int fd)
{

    /* Construct and send the request to server */
    {
        /* Create a message to send */
        get_device_infoRequest deviceinfo = {};

        /* Create a stream that will write to our buffer. */
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

        deviceinfo.dummy_field = 'a';

        /* Now we are ready to encode the message! */
        status = pb_encode(&stream, get_device_infoRequest_fields, &deviceinfo);

        message_length = stream.bytes_written;

        /* Then just check for any errors.. */
        if (!status)
        {
            printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
            return 1;
        }

        write(fd, buffer, message_length);
        printf("Sucessfully sent data \n");
    }
    return true;
}

bool get_device_info_response(uint8_t *buffer, size_t length)
{
    /* Read back the response from server */
    {
        get_device_infoResponse response = {};

        /* Create a stream that reads from the buffer. */
        pb_istream_t stream = pb_istream_from_buffer(buffer, length);

        if (!pb_decode(&stream, get_device_infoResponse_fields, &response))
        {
            fprintf(stderr, "Decode failed: %s\n", PB_GET_ERROR(&stream));
            return false;
        }
        printf("Device type: %s\n", response.type);
        printf("Product: %s\n", response.product);
        printf("IP: %s\n", response.ip);
    }
    return true;
}
