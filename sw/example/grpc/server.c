/**********************************************************************/
/**
 * @file hello_world/main.c
 * @author Stephan Nolting
 * @brief Classic 'hello world' demo prog
 **************************************************************************/
#include <neorv32.h>
#include <stdio.h>
#include <string.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "main.pb.h"

/**********************************************************************/
/**
 * @name User configuration
 **************************************************************************/
/**@{*/
/** UART BAUD rate */
#define BAUD_RATE 19200
/** Number of RGB LEDs in stripe (24-bit data) */
#define NUM_LEDS_24BIT (12)
#define MAX_INTENSITY (16)
/**@}*/

uint8_t buffer[128];
size_t message_length;
bool status;
char key[30];

bool handle_request();
/*User defined functions
// int get_device_info();
// int claim();
// int setLED();
int unclaim(); */

static bool uart_write_callback(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
    neorv32_uart0_printf((const char *)buf); 
    return true;
}

static bool uart_read_callback(pb_istream_t *stream, uint8_t *buf, size_t count)
{
    neorv32_uart0_scan((char *)buf, 40, 1); 
    return true;
}

pb_ostream_t pb_ostream_from_uart()
{   
    pb_ostream_t stream = {&uart_write_callback, NULL, SIZE_MAX, 0};
    return stream;
}

pb_istream_t pb_istream_from_uart()
{
    pb_istream_t stream = {&uart_read_callback, NULL, SIZE_MAX};
    return stream;
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

    // capture all exceptions and give debug info via UART
    // this is not required, but keeps us safe
    neorv32_rte_setup(); 

    // setup UART at default baud rate, no interrupts
     neorv32_uart0_setup(BAUD_RATE, 0);

    // print project logo via UART
     neorv32_aux_print_logo(); 

    while (1)
    {
        handle_request();
    }

    return 0;
}

bool handle_request()
{
    /* Decode the message from the client and open the requested directory. */
    {
        get_device_infoRequest request = get_device_infoRequest_init_zero;
        pb_istream_t stream = pb_istream_from_uart();

        /* Now we are ready to decode the message. */
        status = pb_decode(&stream, get_device_infoRequest_fields, &request);

        /* Check for errors... */
        if (!status)
        {
            neorv32_uart0_printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
            return false;
        }

        neorv32_uart0_printf("Requesting for device info \n");
    }

    {
        get_device_infoResponse response = get_device_infoResponse_init_zero;
        /* Create a stream that will write to our buffer. */
        pb_ostream_t stream = pb_ostream_from_uart();

        /* Create a stream that will write to our buffer. */
        strcpy(response.type, "x3xx");
        strcpy(response.product, "X310");
        strcpy(response.ip, "192.10.2.0");

        /* Now we are ready to encode the message! */
        status = pb_encode(&stream, get_device_infoResponse_fields, &response);
        message_length = stream.bytes_written;

        /* Then just check for any errors.. */
        if (!status)
        {
            neorv32_uart0_printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
            return false;
        }
    }
    return true;
}
