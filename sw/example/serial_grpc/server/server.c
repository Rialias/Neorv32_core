/**********************************************************************/
/**
 * @file hello_world/main.c
 * @author Stephan Nolting
 * @brief Classic 'hello world' demo prog
 **************************************************************************/
#include <neorv32.h>
#include <unistd.h>
//#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <pb_encode.h>
#include <pb_decode.h>

#include "fileproto.pb.h"

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
    uint8_t read_buffer[128];
    /* capture all exceptions and give debug info via UART
    // this is not required, but keeps us safe */
    neorv32_rte_setup();

    /* setup UART at default baud rate, no interrupts */
    neorv32_uart0_setup(BAUD_RATE, 0);

    /* print project logo via UART */
    neorv32_aux_print_logo();
    size_t length = 0;
    for(;;) {

        neorv32_uart0_printf("Waiting for request");
        length = neorv32_uart0_scan((char *)read_buffer, sizeof(read_buffer), 1);

        if (!length) 
            continue;
        
        get_device_infoRequest request = get_device_infoRequest_init_zero;

        pb_istream_t stream = pb_istream_from_buffer(read_buffer, length);

        status = pb_decode(&stream, get_device_infoRequest_fields, &request);

        if (!status)
        {
            neorv32_uart0_printf("Decoding failed: %s\n", PB_GET_ERROR(&stream)); 
            return 1;
        }

        if(request.dummy_field == 'a')
        {
            handle_request();
        }
    }

    return 0;
}

bool handle_request()
{

    {
        get_device_infoResponse response = get_device_infoResponse_init_zero;
        /* Create a stream that will write to our buffer. */
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

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
        size_t i;
        for (i = 0; i < message_length; i++)
        {
            neorv32_uart0_putc(buffer[i]);
        }
    }
    return true;
}
