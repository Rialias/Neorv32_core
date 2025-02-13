// ================================================================================ //
// The NEORV32 RISC-V Processor - https://github.com/stnolting/neorv32              //
// Copyright (c) NEORV32 contributors.                                              //
// Copyright (c) 2020 - 2025 Stephan Nolting. All rights reserved.                  //
// Licensed under the BSD-3-Clause license, see LICENSE for details.                //
// SPDX-License-Identifier: BSD-3-Clause                                            //
// ================================================================================ //


// #include <neorv32.h>
//  C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>       // Contains file controls like O_RDWR
#include <errno.h>       // Error integer and strerror() function
//#include <linux/termios.h> // Contains POSIX terminal control definitions
#include <asm-generic/termbits.h>
#include <sys/ioctl.h>
#include <unistd.h> // write(), read(), close()

/**********************************************************************/ 
/**
* @name User configuration
**************************************************************************/
/**@{*/
/** UART BAUD rate */
// #define BAUD_RATE 19200
/**@}*/

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

  // capture all exceptions and give debug info via UART
  // this is not required, but keeps us safe
  // neorv32_rte_setup();

  // setup UART at default baud rate, no interrupts
  // neorv32_uart0_setup(BAUD_RATE, 0);

  // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
  int fd = open("/dev/ttyUSB0", O_RDWR);
  if (fd < 0)
  {
    printf("Error %d opening /dev/ttyUSB0: %s\n", errno, strerror(errno));
    return -1;
  }
  // Create new termios struct, we call it 'tty' for convention
  struct termios2 tty;

  // Read in existing settings, and handle any error
  if (ioctl(fd, TCGETS2, &tty) != 0)
  {
    printf("Error %d from ioctl TCGETS2: %s\n", errno, strerror(errno));
    return -1;
  }

  tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
  tty.c_cflag |= CS8;            // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;                                                         // Enable echo
  tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
  tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
  tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  tty.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 19200
  tty.c_ispeed = 19200; // What a custom baud rate!
  tty.c_ospeed = 19200;

  // Save tty settings, also checking for error
  if (ioctl(fd, TCSETS2, &tty) != 0)
  {
    printf("Error %d from ioctl TCSETS2: %s\n", errno, strerror(errno));
    return -1;
  }

  // Example: Write data to the serial port
  const char *msg = "Hello, serial port!";
  write(fd, msg, strlen(msg));

  // Allocate memory for read buffer, set size according to your needs
  char read_buf[256];

  // Normally you wouldn't do this memset() call, but since we will just receive
  // ASCII data for this example, we'll set everything to 0 so we can
  // call printf() easily.
  // memset(&read_buf, '\0', sizeof(read_buf));

  // Read bytes. The behaviour of read() (e.g. does it block?,
  // how long does it block for?) depends on the configuration
  // settings above, specifically VMIN and VTIME
  int num_bytes = read(fd, &read_buf, sizeof(read_buf));

  // n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
  if (num_bytes < 0)
  {
    printf("Error reading: %s", strerror(errno));
    return 1;
  }

  // Here we assume we received ASCII data, but you might be sending raw bytes (in that case, don't try and
  // print it to the screen like this!)
  printf("Read %i bytes. Received message: %s", num_bytes, read_buf);

  close(fd);
  return 0; // success
};
