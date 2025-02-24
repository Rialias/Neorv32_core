/**********************************************************************/
/**
 * @file hello_world/main.c
 * @author Stephan Nolting
 * @brief Classic 'hello world' demo prog
 **************************************************************************/
#include <neorv32.h>
#include <unistd.h>
// #include <dirent.h>
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

// #define BUFFER_SIZE 256

int message_length;
bool status;
char key[30];

bool handle_request(pb_istream_t *istream, pb_ostream_t *ostream);
/*User defined functions
// int get_device_info();
// int claim();

int unclaim(); */
// int setLED();
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
    // char read_buffer[BUFFER_SIZE];
    /* capture all exceptions and give debug info via UART
    // this is not required, but keeps us safe */
    neorv32_rte_setup();

    /* setup UART at default baud rate, no interrupts */
    neorv32_uart0_setup(BAUD_RATE, 0);

    /* print project logo via UART */
    /* neorv32_aux_print_logo(); */
    // size_t length = 0;
    neorv32_cpu_delay_ms(3000);
    // handle_request();
    char buffer[128];
    size_t length = 0;

    for (;;)
    {
        neorv32_uart0_printf("waiting for message");
        length = neorv32_uart0_scan(buffer, sizeof(buffer), 1);
        neorv32_uart0_printf("\n");

        if (!length)
        {
            neorv32_uart0_printf("empty \n");
            continue;
        } // nothing to be done

        neorv32_uart0_printf("Received something %d \n", length);
        for (int i = 0; i < 200; i++)
        {
            neorv32_uart0_printf("%d\n", i);
        }
        neorv32_cpu_delay_ms(3000);
        uint8_t message[128];

        for (int j = 0; j < length; j++)
        {
            neorv32_uart0_printf("%c", buffer[j]);
        }
        neorv32_uart0_printf("\n");
        // Convert the buffer to uint8_t values
        int msg_index = 0;
        int num = 0;
        for (int j = 0; j < length; j++)
        {
            if (buffer[j] >= '0' && buffer[j] <= '9')
            {
                num = num * 10 + (buffer[j] - '0');
            }
            else if (buffer[j] == ' ' || j == length - 1)
            {
                if (buffer[j] != ' ')
                {
                    num = num * 10 + (buffer[j] - '0');
                }
                message[msg_index++] = (uint8_t)num;
                num = 0;
            }
        }

        // Print the converted values
        for (int j = 0; j < msg_index; j++)
        {
            neorv32_uart0_printf("%d\n", message[j]);
        }
        neorv32_uart0_printf("\n");

        for (int j = 0; j < msg_index; j++)
        {
            neorv32_uart0_printf("%d \n", message[j]);
        }
        neorv32_uart0_printf("\n");

        // Print raw bytes of the message
        for (int j = 0; j < length; j++)
        {
            neorv32_uart0_printf("%X ", message[j]);
        }

        neorv32_uart0_printf("%d", length);

        pb_istream_t istream = pb_istream_from_buffer(message, length);
        pb_ostream_t ostream = pb_ostream_from_buffer(message, sizeof(message));
        neorv32_cpu_delay_ms(5000);
        handle_request(&istream, &ostream);
    }
    return 0;
}

bool handle_request(pb_istream_t *istream, pb_ostream_t *ostream)
{
    Request request = Request_init_zero;
    Response response = Response_init_zero;

    if (!pb_decode_delimited(istream, Request_fields, &request))
    {
        neorv32_uart0_printf("Failed to decode response: %s\n", PB_GET_ERROR(istream));
        return 1;
    }

    if (request.which_request_type == Request_get_device_info_tag)
    {
        neorv32_uart0_printf("Device Info\n");
        strcpy(response.response_type.get_device_info.type, "x3xx");
        strcpy(response.response_type.get_device_info.product, "X310");
        strcpy(response.response_type.get_device_info.ip, "192.10.1.1");
        response.which_response_type = Response_get_device_info_tag;
    }
    else if (request.which_request_type == Request_claim_tag)
    {
        neorv32_uart0_printf("claim\n");
        strcpy(response.response_type.claim.token, "password");
        response.which_response_type = Response_claim_tag;
    }
    else if (request.which_request_type == Request_reclaim_tag)
    {
        neorv32_uart0_printf("reclaim\n");
        strcpy(response.response_type.claim.token, "password");
        response.which_response_type = Response_claim_tag;
    }
    else if (request.which_request_type == Request_unclaim_tag)
    {
        neorv32_uart0_printf("unclaim\n");
        strcpy(response.response_type.claim.token, "0000");
        response.which_response_type = Response_claim_tag;
    }
    else if (request.which_request_type == Request_set_smartled_tag)
    {
        neorv32_uart0_printf("samrtled\n");
        /*uint32_t color = request.request_type.set_smartled.color;
        int32_t id = request.request_type.set_smartled.id;
        char token[20];
        strcpy(token, request.request_type.set_smartled.token);
        setLED(); */
    }
    else
    {
        neorv32_uart0_printf("Wrong request \n");
    }

    return 0;
}