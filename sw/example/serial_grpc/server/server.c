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
#include <stdlib.h>
#include <time.h>
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

char randomString[10];
size_t message_length;
bool status;

bool handle_request(pb_istream_t *istream);

int setLED(uint32_t color, int32_t id, char *token);

void generateRandomString(char *str, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    if (length) {
        --length;
        for (size_t n = 0; n < length; n++) {
            int key = rand() % (int)(sizeof(charset) - 1);
            str[n] = charset[key];
        }
        str[length] = '\0';
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
    neorv32_rte_setup();
    /* setup UART at default baud rate, no interrupts */
    neorv32_uart0_setup(BAUD_RATE, 0);
    // neorv32_cpu_delay_ms(3000);
    char buffer[128];
    size_t length = 0;
    srand(time(0));
    for (;;)
    {
        neorv32_uart0_printf("waiting for message");
        length = neorv32_uart0_scan(buffer, sizeof(buffer), 1);
        neorv32_uart0_printf("\n");

        if (!length)
        {
            neorv32_uart0_printf("empty \n");
            continue;
        }
        uint8_t message[256];
        int msg_index = 0;
        char *token = strtok(buffer, " ");
        while (token != NULL)
        {
            message[msg_index++] = (uint8_t)atoi(token);
            token = strtok(NULL, " ");
        }
        pb_istream_t istream = pb_istream_from_buffer(message, length);
        handle_request(&istream);
    }
    return 0;
}

bool handle_request(pb_istream_t *istream)
{
    uint8_t response_buffer[128];

    Request request = Request_init_zero;
    Response response = Response_init_zero;
    neorv32_gpio_port_set(0xFE);
    memset(&response_buffer, 0, sizeof(response_buffer));

    /* Create a stream that will write to our buffer. */
    pb_ostream_t ostream = pb_ostream_from_buffer(response_buffer, sizeof(response_buffer));

    if (!pb_decode_delimited(istream, Request_fields, &request))
    {
        neorv32_uart0_printf("Failed to decode request: %s\n", PB_GET_ERROR(istream));
        return 1;
    }
    neorv32_uart0_putc('a');

    if (request.which_request_type == Request_get_device_info_tag)
    {
        // neorv32_uart0_printf("Device Info\n");
        strcpy(response.response_type.get_device_info.type, "x3xx");
        strcpy(response.response_type.get_device_info.product, "X310");
        strcpy(response.response_type.get_device_info.ip, "192.10.1.1");
        response.which_response_type = Response_get_device_info_tag;
    }
    else if (request.which_request_type == Request_claim_tag)
    {
        // neorv32_uart0_printf("claim\n");
        uint32_t current_state = neorv32_gpio_port_get(); // Read the current state of the GPIO port
        current_state &= 0xFC;                           // Clear the second bit (bit 1) to set it to 0 (turn on the second LED)
        neorv32_gpio_port_set(current_state);
        generateRandomString(randomString, 11);
        strcpy(response.response_type.claim.token, randomString);
        response.which_response_type = Response_claim_tag;
    }
    else if (request.which_request_type == Request_reclaim_tag)
    {
        // neorv32_uart0_printf("reclaim\n");
        strcpy(response.response_type.reclaim.token, randomString);
        response.which_response_type = Response_reclaim_tag;
    }
    else if (request.which_request_type == Request_unclaim_tag)
    {
        // neorv32_uart0_printf("unclaim\n");
        uint32_t current_state = neorv32_gpio_port_get(); // Read the current state of the GPIO port
        current_state |= 0x02;                            // Set the second bit to 1 (turn off the second LED)
        neorv32_gpio_port_set(current_state);
        strcpy(response.response_type.unclaim.token, "no_token");
        response.which_response_type = Response_unclaim_tag;
    }
    else if (request.which_request_type == Request_set_smartled_tag)
    {
        // neorv32_uart0_printf("samrtled\n");
        strcpy(response.response_type.led.token, randomString);
        uint32_t color = request.request_type.set_smartled.color;
        int32_t id = request.request_type.set_smartled.id;
        char token[20];
        strcpy(token, request.request_type.set_smartled.token);
        setLED(color, id, token);
        response.which_response_type = Response_led_tag;
    }
    else
    {
        neorv32_uart0_printf("Wrong request \n");
        return 1;
    }
    
    status = pb_encode_delimited(&ostream, Response_fields, &response);
    message_length = ostream.bytes_written;
    if (!status)
    {
        neorv32_uart0_printf("Encoding failed: %s\n", PB_GET_ERROR(&ostream));
        return 1;
    }
    char response_message[message_length];
    memset(&response_message, '\0', sizeof(response_message));
    // neorv32_uart0_printf("%d", message_length);
    neorv32_uart0_putc((char)message_length);

    for (size_t i = 0; i < message_length; i++)
    {
        response_message[i] = (char)response_buffer[i];
    }
    neorv32_uart0_puts(response_message);
    uint32_t current_state = neorv32_gpio_port_get();
    current_state |= 0xFD; // Set the first bit to 1 (turn off the first LED)
    neorv32_gpio_port_set(current_state);
    return 0;
}

int setLED(uint32_t color, int32_t id, char *token)
{
    // check if NEOLED unit is implemented at all, abort if not
    if (neorv32_neoled_available() == 0)
    {
        // neorv32_uart0_printf("Error! No NEOLED unit synthesized!\n");
        return 1;
    }
    set_smartledRequest led = set_smartledRequest_init_zero;
    if (strcmp(token, randomString))
    {
        // neorv32_uart0_printf("\n Device is not claimed. Token doesn't match \n\n");
    }
    else
    {

        neorv32_neoled_setup_ws2812(0); // interrupt configuration = fire IRQ if TX FIFO is empty (not used here)
        neorv32_neoled_set_mode(0);     // mode = 0 = 24-bit
        // clear all LEDs
        // neorv32_uart0_printf("\n Clearing all LEDs...\n");
        int i;
        for (i = 0; i < NUM_LEDS_24BIT; i++)
        {
            neorv32_neoled_write_blocking(0);
        }

        neorv32_cpu_delay_ms(500);
        led.id = id;
        led.color = color;
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
    }
    return 0;
}
