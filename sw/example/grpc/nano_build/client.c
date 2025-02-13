
#include <stdio.h>
#include <string.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include <pb_decode.h>
#include "main.pb.h"

#include <fcntl.h> 
#include <errno.h> 
#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**********************************************************************/
/**
 * @name User configuration
 **************************************************************************/

static bool serial_write_callback(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
    int fd = (intptr_t)stream->state;
    return write(fd, buf, count) == count;
}

static bool serial_read_callback(pb_istream_t *stream, uint8_t *buf, size_t count)
{
    int fd = (intptr_t)stream->state;

    int num_bytes = read(fd, buf, count);

    if (num_bytes < 0)
    {
        printf("Error reading: %s", strerror(errno));
        return false;
    }

    return num_bytes == count;
}

pb_ostream_t pb_ostream_from_serial(int fd)
{
    pb_ostream_t stream = {&serial_write_callback, (void *)(intptr_t)fd, SIZE_MAX};
    return stream;
}

pb_istream_t pb_istream_from_serial(int fd)
{
    pb_istream_t stream = {&serial_read_callback, (void *)(intptr_t)fd, SIZE_MAX};
    return stream;
}

bool get_device_info(int fd);

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

    get_device_info(fd);

    return 0;
}

bool get_device_info(int fd)
{

    /* Construct and send the request to server */
    {

        /* Create a message to send */
        get_device_infoRequest deviceinfo = {};
        pb_ostream_t output = pb_ostream_from_serial(fd);

        if (!pb_encode_delimited(&output, get_device_infoRequest_fields, &deviceinfo))
        {
            fprintf(stderr, "Encoding failed: %s\n", PB_GET_ERROR(&output));
            return false;
        }
    }

    /* Read back the response from server */
    {
        get_device_infoRequest response = {};
        pb_istream_t input = pb_istream_from_serial(fd);

        if (!pb_decode_delimited(&input, get_device_infoRequest_fields, &response))
        {
            fprintf(stderr, "Decode failed: %s\n", PB_GET_ERROR(&input));
            return false;
        }
    }

    return true;
}
