// ================================================================================ //
// The NEORV32 RISC-V Processor - https://github.com/stnolting/neorv32              //
// Copyright (c) NEORV32 contributors.                                              //
// Copyright (c) 2020 - 2025 Stephan Nolting. All rights reserved.                  //
// Licensed under the BSD-3-Clause license, see LICENSE for details.                //
// SPDX-License-Identifier: BSD-3-Clause                                            //
// ================================================================================ //

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

// prototypes
uint32_t hsv2rgb(int h, int v);

uint8_t buffer[128];
size_t message_length;
bool status;
char key[30];

// User defined functions
int get_device_info();
int claim();
int setLED();
int unclaim();

bool encode_message(const pb_msgdesc_t *fields, const void *src_struct)
{
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    /* Now we are ready to encode the message! */
    status = pb_encode(&stream, fields, src_struct);
    message_length = stream.bytes_written;
    /* Then just check for any errors.. */
    if (!status)
    {
        neorv32_uart0_printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    return true;
}

bool decode_message(const pb_msgdesc_t *fields, void *dest_struct)
{
    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

    /* Now we are ready to decode the message. */
    status = pb_decode(&stream, fields, dest_struct);

    /* Check for errors... */
    if (!status)
    {
        neorv32_uart0_printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    return true;
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

    char buf[8];
    // int option = 0;
    while (1)
    {
        neorv32_uart0_printf("Enter the number to perform an action.\n");
        neorv32_uart0_printf("1. Get device info\n");
        neorv32_uart0_printf("2. Claim\n");
        neorv32_uart0_printf("3. Reclaim\n");
        neorv32_uart0_printf("4. Unclaim\n");
        neorv32_uart0_printf("5. Set smart LED\n");
        neorv32_uart0_scan(buf, 8, 1);
        neorv32_uart0_printf("\n");
        int option = atoi(buf);

        switch (option)
        {
        case 1:
            neorv32_uart0_printf("\n Getting Device Info \n");
            get_device_info();
            break;

        case 2:
            neorv32_uart0_printf("\n Claiming the device \n");
            claim();
            break;

        case 3:
            neorv32_uart0_printf("\n Reclaim the  device \n");
            claim();
            break;
        case 4:
            neorv32_uart0_printf("\n Unclaim the device \n");
            unclaim();
            break;

        case 5:
            neorv32_uart0_printf("\n set the smartLED \n");
            setLED();
            break;
        default:
            neorv32_uart0_printf("\n Wrong option \n");
        }
    }
    return 0;
}

int get_device_info()
{
    get_device_infoResponse deviceinfo = get_device_infoResponse_init_zero;

    /* Create a stream that will write to our buffer. */
    strcpy(deviceinfo.type, "x3xx");
    strcpy(deviceinfo.product, "X310");
    strcpy(deviceinfo.ip, "192.10.2.0");
    if (!encode_message(get_device_infoResponse_fields, &deviceinfo))
    {
        return 1;
    }
    // Decoding the messsage
    get_device_infoResponse decoded_deviceinfo = get_device_infoResponse_init_zero;
    if (!decode_message(get_device_infoResponse_fields, &decoded_deviceinfo))
    {
        return 1;
    }

    neorv32_uart0_printf("Device type: %s\n", decoded_deviceinfo.type);
    neorv32_uart0_printf("Product: %s\n", decoded_deviceinfo.product);
    neorv32_uart0_printf("IP: %s\n", decoded_deviceinfo.ip);
    return 0;
}

int claim()
{
    claimResponse token = claimResponse_init_zero;
    strcpy(token.token, "new_token");
    if (!encode_message(claimResponse_fields, &token))
    {
        return 1;
    }
    // Decoding the messsage
    claimResponse decoded_token = claimResponse_init_zero;
    if (!decode_message(claimResponse_fields, &decoded_token))
    {
        return 1;
    }

    /* Print the data contained in the message. */
    neorv32_uart0_printf("\n Token: %s\n", (char *)decoded_token.token);
    strcpy(key, decoded_token.token);
    return 0;
}

int unclaim()
{
    unclaimRequest token = unclaimRequest_init_zero;
    strcpy(token.token, "0000");
    if (!encode_message(unclaimRequest_fields, &token))
    {
        return 1;
    }
    // Decoding the messsage
    unclaimRequest decoded_token = claimResponse_init_zero;
    if (!decode_message(unclaimRequest_fields, &decoded_token))
    {
        return 1;
    }

    /* Print the data contained in the message. */
    neorv32_uart0_printf("\n Token: %s\n", (char *)decoded_token.token);
    strcpy(key, decoded_token.token);
    return 0;
}

int setLED()
{
    // check if NEOLED unit is implemented at all, abort if not
    if (neorv32_neoled_available() == 0)
    {
        neorv32_uart0_printf("Error! No NEOLED unit synthesized!\n");
        return 1;
    }
    set_smartledRequest led = set_smartledRequest_init_zero;
    if (strcmp(key, "new_token"))
    {
        neorv32_uart0_printf("\n Device is not claimed. Token doesn't match \n\n");
    }
    else
    {
        char temp[8];
        char color[8];
        neorv32_neoled_setup_ws2812(0); // interrupt configuration = fire IRQ if TX FIFO is empty (not used here)
        neorv32_neoled_set_mode(0);     // mode = 0 = 24-bit
        // clear all LEDs
        neorv32_uart0_printf("\n Clearing all LEDs...\n");
        int i;
        for (i = 0; i < NUM_LEDS_24BIT; i++)
        {
            neorv32_neoled_write_blocking(0);
        }

        neorv32_cpu_delay_ms(500);

        neorv32_uart0_printf("\n Enter the Led ID to set \n");
        neorv32_uart0_scan(temp, 8, 1);
        neorv32_uart0_printf("\n Enter the Led color to set \n");
        neorv32_uart0_scan(color, 8, 1);
        led.id = atoi(temp);
        led.color = strtol(color, NULL, 16);
        strcpy(led.token, key);
        if (!encode_message(set_smartledRequest_fields, &led))
        {
            return 1;
        }
        // Decoding the messsage
        set_smartledRequest decoded_led = set_smartledRequest_init_zero;
        if (!decode_message(set_smartledRequest_fields, &decoded_led))
        {
            return 1;
        }
        for (int i = 0; i < NUM_LEDS_24BIT; i++)
        {
            if (i == led.id)
            {
                // Set the specific LED to the desired color
                neorv32_neoled_write_blocking(led.color);
            }
            else
            {
                neorv32_neoled_write_blocking(0); // Turn off other LEDs or set them to a default color
            }
        }
        neorv32_neoled_strobe_blocking();

        /* Print the data contained in the message. */
        neorv32_uart0_printf("Token: %s\n", (char *)decoded_led.token);
    }
    return 0;
}
