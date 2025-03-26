#include <neorv32.h>
#include <string.h>

/**@{*/
/** UART BAUD rate */
#define BAUD_RATE 19200
/**@}*/

#define EEPROM_ADDRESS 0x54 // Example EEPROM I2C address

void write_eeprom(uint8_t address, uint8_t data);

void read_eeprom(uint8_t address, uint8_t *data);
void print_hex_byte(uint8_t data);

int main()
{
    uint8_t data;
    // Initialize UART
    neorv32_rte_setup();
    if (neorv32_uart0_available() == 0)
    {
        return 1;
    }
    neorv32_uart0_setup(BAUD_RATE, 0);

    // Initialize TWI
    if (neorv32_twi_available() == 0)
    {
        neorv32_uart0_printf("ERROR! TWI controller not available!");
        return 1;
    }
    neorv32_twi_setup(CLK_PRSC_2048, 15, 0);

    // Device details to write
    char device_details[] = {'a', 'b'};
   
    // Write each character to EEPROM
    for (int i = 0; i < sizeof(device_details); i++)
    {
        write_eeprom(i, (uint8_t)device_details[i]);
    }

    neorv32_uart0_printf("Device details written to EEPROM.\n");

    // Buffer to store read data
    uint8_t read_device;

    // Read each character from EEPROM
    for (int i = 0; i < sizeof(device_details); i++)
    {
        read_eeprom(i, &read_device);
        neorv32_uart0_printf("Read byte %x from EEPROM\n", read_device);
    }

    neorv32_uart0_printf("Device details read from EEPROM.\n");

    return 0;
}

void write_eeprom(uint8_t address, uint8_t data)
{
    uint8_t eeprom_address = (EEPROM_ADDRESS << 1) | 0; // EEPROM address with write flag
    neorv32_twi_generate_start();
    if (neorv32_twi_trans(&eeprom_address, 1) != 0) // Check ACK/NACK
    {
        neorv32_uart0_printf("NACK received for EEPROM address!\n");
        return;
    }
    if (neorv32_twi_trans(&address, 1) != 0) // Check ACK/NACK
    {
        neorv32_uart0_printf("NACK received for memory address!\n");
        return;
    }

    neorv32_uart0_printf("Writing byte %x to EEPROM\n", data); // Debugging message
    if (neorv32_twi_trans(&data, 1) != 0)                      // Check ACK/NACK
    {
        neorv32_uart0_printf("NACK received for data byte %x!\n", data);
        return;
    }

    neorv32_twi_generate_stop();
}

void read_eeprom(uint8_t address, uint8_t *data)
{
    uint8_t eeprom_address = (EEPROM_ADDRESS << 1) | 0;      // EEPROM address with write flag
    uint8_t eeprom_read_address = (EEPROM_ADDRESS << 1) | 1; // EEPROM address with read flag

    neorv32_twi_generate_start();
    if (neorv32_twi_trans(&eeprom_address, 1) != 0) // Check ACK/NACK
    {
        neorv32_uart0_printf("NACK received for EEPROM address!\n");
        return;
    }
    if (neorv32_twi_trans(&address, 1) != 0) // Check ACK/NACK
    {
        neorv32_uart0_printf("NACK received for memory address!\n");
        return;
    }

    neorv32_twi_generate_start();                        // Generate repeated START condition
    if (neorv32_twi_trans(&eeprom_read_address, 1) != 0) // Check ACK/NACK
    {
        neorv32_uart0_printf("NACK received for EEPROM read address!\n");
        return;
    }

    uint8_t dummy = 0xFF;                  // Send 0xFF to initiate read
    if (neorv32_twi_trans(&dummy, 1) != 0) // Check ACK/NACK
    {
        neorv32_uart0_printf("NACK received for dummy byte!\n");
        return;
    }
    if (neorv32_twi_trans(data, 0) == 1) // Check ACK/NACK
    {
        neorv32_uart0_printf("Read byte %x from EEPROM\n", *data); // Debugging message
        //print_hex_byte(*data);
    }
    else if (neorv32_twi_trans(data, 0) == 0)
    {
        neorv32_uart0_printf("ACK recived");
    }
    else
    {
        neorv32_uart0_printf("No data available");
    }

    neorv32_twi_generate_stop();

    /*neorv32_uart0_printf("Address 0x");
    print_hex_byte(address);
    neorv32_uart0_printf("READ Address 0x");
    print_hex_byte(eeprom_read_address);
    neorv32_uart0_printf("WRITE Address 0x");
    print_hex_byte(eeprom_address); */
}

/**********************************************************************/ /**
                                                                          * Print HEX byte.
                                                                          *
                                                                          * @param[in] byte Byte to be printed as 2-cahr hex value.
                                                                          **************************************************************************/
void print_hex_byte(uint8_t byte)
{

    static const char symbols[] = "0123456789abcdef";

    neorv32_uart0_putc(symbols[(byte >> 4) & 0x0f]);
    neorv32_uart0_putc(symbols[(byte >> 0) & 0x0f]);
}
