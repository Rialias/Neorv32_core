#include <neorv32.h>
#include <string.h>

/** UART BAUD rate */
#define BAUD_RATE 19200
/** EEPROM I2C address */
#define EEPROM_I2C_ADDRESS 0x54
/** EEPROM size (in bytes) */
#define EEPROM_SIZE 256

void scan_twi(void);
void set_clock(void);
void read_eeprom(void);
void print_hex_byte(uint8_t data);

int main()
{

    char buffer[8];
    int length = 0;

    // capture all exceptions and give debug info via UART
    neorv32_rte_setup();

    // check if UART unit is implemented at all
    if (neorv32_uart0_available() == 0)
    {
        return 1;
    }

    // setup UART at default baud rate, no interrupts
    neorv32_uart0_setup(BAUD_RATE, 0);

    // check if TWI unit is implemented at all
    if (neorv32_twi_available() == 0)
    {
        neorv32_uart0_printf("ERROR! TWI controller not available!");
        return 1;
    }

    // intro
    neorv32_uart0_printf("\n--- TWI Bus Explorer ---\n\n");
    neorv32_uart0_printf("This program allows to create TWI transfers by hand.\n"
                         "Execute 'help' to see the help menu.\n\n");

    // configure TWI, second slowest clock, no clock stretching allowed
    neorv32_twi_setup(CLK_PRSC_2048, 15, 0);

    // Main menu
    for (;;)
    {
        neorv32_uart0_printf("TWI_EXPLORER:> ");
        length = neorv32_uart0_scan(buffer, 8, 1);
        neorv32_uart0_printf("\n");

        if (!length) // nothing to be done
            continue;

        // decode input and execute command
        if (!strcmp(buffer, "help"))
        {
            neorv32_uart0_printf("Available commands:\n"
                                 " help  - show this text\n"
                                 " setup - configure bus clock (will reset TWI module)\n"
                                 " scan  - scan bus for devices\n"
                                 " start - generate (repeated) START condition\n"
                                 " stop  - generate STOP condition\n"
                                 " read  - read data from EEPROM\n"
                                 " sense - show current SCL and SDA bus levels\n"
                                 "Start a new transmission by generating a START condition. Next, transfer the 7-bit device address\n"
                                 "and the R/W flag. After that, transfer your data to be written or send a 0xFF if you want to read\n"
                                 "data from the bus. Finish the transmission by generating a STOP condition.\n");
        }
        else if (!strcmp(buffer, "start"))
        {
            neorv32_uart0_printf("Sending START condition...\n");
            neorv32_twi_generate_start(); // generate START condition
        }
        else if (!strcmp(buffer, "stop"))
        {
            neorv32_uart0_printf("Sending STOP condition...\n");
            neorv32_twi_generate_stop(); // generate STOP condition
        }
        else if (!strcmp(buffer, "scan"))
        {
            scan_twi();
        }
        else if (!strcmp(buffer, "setup"))
        {
            set_clock();
        }
        else if (!strcmp(buffer, "read"))
        {
            read_eeprom();
        }
        else if (!strcmp(buffer, "sense"))
        {
            neorv32_uart0_printf(" SCL: %u\n", neorv32_twi_sense_scl());
            neorv32_uart0_printf(" SDA: %u\n", neorv32_twi_sense_sda());
        }
        else
        {
            neorv32_uart0_printf("Invalid command. Type 'help' to see all commands.\n");
        }
    }

    return 0;
}

/**********************************************************************/ /**
                                                                          * TWI clock setup
                                                                          **************************************************************************/
void set_clock(void)
{

    const uint32_t PRSC_LUT[8] = {2, 4, 8, 64, 128, 1024, 2048, 4096};
    char terminal_buffer[2];

    // clock prescaler
    neorv32_uart0_printf("Select new clock prescaler (0..7; one hex char): ");
    neorv32_uart0_scan(terminal_buffer, 2, 1); // 1 hex char plus '\0'
    int prsc = (int)neorv32_aux_hexstr2uint64(terminal_buffer, strlen(terminal_buffer));

    if ((prsc < 0) || (prsc > 7))
    { // invalid?
        neorv32_uart0_printf("\nInvalid selection!\n");
        return;
    }

    // clock divider
    neorv32_uart0_printf("\nSelect new clock divider (0..15; one hex char): ");
    neorv32_uart0_scan(terminal_buffer, 2, 1); // 1 hex char plus '\0'
    int cdiv = (int)neorv32_aux_hexstr2uint64(terminal_buffer, strlen(terminal_buffer));

    if ((cdiv < 0) || (cdiv > 15))
    { // invalid?
        neorv32_uart0_printf("\nInvalid selection!\n");
        return;
    }

    // clock stretching
    neorv32_uart0_printf("\nEnable clock stretching (y/n)? ");
    int clkstr = 0;
    char tmp = neorv32_uart0_getc();
    neorv32_uart0_putc(tmp);

    if ((tmp != 'y') && (tmp != 'n'))
    { // invalid?
        neorv32_uart0_printf("\nInvalid selection!\n");
        return;
    }

    if (tmp == 'y')
    {
        clkstr = 1;
    }

    // set new configuration
    neorv32_twi_setup(prsc, cdiv, clkstr);

    // print new clock frequency
    uint32_t clock = neorv32_sysinfo_get_clk() / (4 * PRSC_LUT[prsc] * (1 + cdiv));
    neorv32_uart0_printf("\nNew I2C clock: %u Hz\n", clock);

    // check if bus lines are OK
    if (neorv32_twi_sense_scl() != 1)
    {
        neorv32_uart0_printf("WARNING! SCL bus line is not idle-high! Pull-up missing?\n");
    }
    if (neorv32_twi_sense_sda() != 1)
    {
        neorv32_uart0_printf("WARNING! SDA bus line is not idle-high! Pull-up missing?\n");
    }
}

/**********************************************************************/ /**
                                                                          * Scan 7-bit TWI address space and print results
                                                                          **************************************************************************/
void scan_twi(void)
{

    uint8_t i;
    int num_devices = 0, twi_ack = 0;

    neorv32_uart0_printf("Scanning TWI bus...\n");

    for (i = 0; i < 128; i++)
    {
        neorv32_twi_generate_start();
        uint8_t tmp = 2 * i + 1;
        twi_ack = neorv32_twi_trans(&tmp, 0);
        neorv32_twi_generate_stop();

        if (twi_ack == 0)
        {
            neorv32_uart0_printf(" + Found device at write-address 0x");
            print_hex_byte(2 * i);
            neorv32_uart0_printf("\n");
            num_devices++;
        }
    }

    if (num_devices == 0)
    {
        neorv32_uart0_printf("No devices found.\n");
    }
    else
    {
        neorv32_uart0_printf("Devices found: %i\n", num_devices);
    }
}

/**********************************************************************/ /**
                                                                          * Read data from EEPROM
                                                                          **************************************************************************/
void read_eeprom(void)
{

    uint8_t data;
    uint8_t address = 0x00; // EEPROM memory address to start reading from

    neorv32_uart0_printf("Reading EEPROM data...\n");

    for (uint16_t i = 0; i < EEPROM_SIZE; i++)
    {
        // Send start condition
        neorv32_twi_generate_start();

        // Send EEPROM address with write bit
        uint8_t eeprom_address = (EEPROM_I2C_ADDRESS << 1) | 0;
        neorv32_twi_trans(&eeprom_address, 0);

        // Send memory address to read from
        neorv32_twi_trans(&address, 0);

        // Send repeated start condition
        neorv32_twi_generate_start();

        // Send EEPROM address with read bit
        eeprom_address = (EEPROM_I2C_ADDRESS << 1) | 1;
        neorv32_twi_trans(&eeprom_address, 0);

        // Read data from EEPROM
        // uint8_t dummy = 0xFF;
        neorv32_twi_trans(&data, 1);
        

        // Send stop condition
        neorv32_twi_generate_stop();

        // Print the received data
        neorv32_uart0_printf("Address 0x");
        print_hex_byte(address);
        neorv32_uart0_printf(": 0x");
        print_hex_byte(data);
        neorv32_uart0_printf("\n");

        // Increment address
        address++;
    }
}

/**********************************************************************/ /**
                                                                          * Print byte as hex chars via UART0.
                                                                          *
                                                                          * @param data 8-bit data to be printed as two hex chars.
                                                                          **************************************************************************/
void print_hex_byte(uint8_t data)
{

    static const char symbols[] = "0123456789abcdef";

    neorv32_uart0_putc(symbols[(data >> 4) & 15]);
    neorv32_uart0_putc(symbols[(data >> 0) & 15]);
}