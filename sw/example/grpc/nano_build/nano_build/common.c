#include <pb_encode.h>
#include <pb_decode.h>

#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
// #include <linux/termios.h> // Contains POSIX terminal control definitions
#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <unistd.h> // write(), read(), close()

#include "common.h"

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
    pb_ostream_t stream = {&serial_write_callback, (void*)(intptr_t)fd, SIZE_MAX};
    return stream;
}

pb_istream_t pb_istream_from_serial(int fd)
{
    pb_istream_t stream = {&serial_read_callback, (void*)(intptr_t)fd, SIZE_MAX};
    return stream;
}