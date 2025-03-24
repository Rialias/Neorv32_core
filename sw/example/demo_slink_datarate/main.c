#include <neorv32.h>
#include <string.h>
#include <inttypes.h>

/**********************************************************************//**
 * @name User configuration
 **************************************************************************/
/**@{*/
/** UART BAUD rate */
#define BAUD_RATE 19200
#define CPU_FREQUENCY 125000000 
/**@}*/

// Prototypes
void slink_firq_handler(void);
uint8_t recv_buffer[10];

/**********************************************************************//**
 * Simple SLINK demo program.
 *
 * @note This program requires the UART0 and the SLINK to be synthesized.
 *
 * @return =! 0 if execution failed.
 **************************************************************************/
int main() {

  int i, slink_rc;
  uint64_t start_time, end_time, duration;
  uint32_t slink_data;


  // capture all exceptions and give debug info via UART0
  neorv32_rte_setup();

  // setup UART at default baud rate, no interrupts
  neorv32_uart0_setup(BAUD_RATE, 0);

  // check if UART0 unit is implemented at all
  if (neorv32_uart0_available() == 0) {
    return -1; // abort if not implemented
  }


  // intro
  neorv32_uart0_printf("\n<<< SLINK Demo Program >>>\n\n");

  // check if SLINK is implemented at all
  if (neorv32_slink_available() == 0) {
    neorv32_uart0_printf("ERROR! SLINK module not implemented.");
    return -1;
  }

  // show SLINK FIFO configuration
  int rx_depth = neorv32_slink_get_rx_fifo_depth();
  int tx_depth = neorv32_slink_get_tx_fifo_depth();
  neorv32_uart0_printf("RX FIFO depth: %u\n"
                       "TX FIFO depth: %u\n\n",
                       rx_depth, tx_depth);


  // setup SLINK module, no interrupts
  neorv32_slink_setup(0, 0);

  // IRQ demo
  neorv32_uart0_printf("\n------ RX IRQ Demo -------\n");

  // reconfigure SLINK module
  neorv32_slink_setup(1 << SLINK_CTRL_IRQ_RX_NEMPTY, 0); // interrupt if RX data available
  neorv32_slink_rx_clear();
  neorv32_slink_tx_clear();

  // NEORV32 runtime environment: install SLINK FIRQ handler
  neorv32_rte_handler_install(SLINK_RX_RTE_ID, slink_firq_handler);
  neorv32_cpu_csr_set(CSR_MIE, 1 << SLINK_RX_FIRQ_ENABLE); // enable SLINK FIRQ
  neorv32_cpu_csr_set(CSR_MSTATUS, 1 << CSR_MSTATUS_MIE); // enable machine-mode interrupts

  uint8_t buffer[10];
  for (i=0; i < 10; i++)
  {
    buffer[i] = i+1;
  }

  neorv32_uart0_printf("calculating start time\n");
  start_time = neorv32_cpu_get_cycle();
  for (i=0; i<10; i++) {
    slink_data = buffer[i];
    //neorv32_uart0_printf("[%i] Sending %i... ", i, slink_data);

    slink_rc = neorv32_slink_tx_status();
    if (slink_rc == SLINK_FIFO_FULL) {
      //neorv32_uart0_printf("FAILED! TX FIFO full!\n");
      break;
    }
    else {
      neorv32_slink_put(slink_data);
      //neorv32_uart0_printf("ok\n");
    }
  }

  end_time = neorv32_cpu_get_cycle();
  duration = end_time - start_time;

  
  // Print the duration as two 32-bit unsigned integers
  neorv32_uart0_printf("\nData transfer completed in cycles %u \n",(duration));
  neorv32_uart0_printf("\nData transfer completed in cycles (high part: %u, low part: %u).\n",  (uint32_t)(duration >> 32), (uint32_t)(duration));

 // Calculate and print data rate in bytes per cycle
 // uint64_t data_rate = 10 / duration;
 // neorv32_uart0_printf("Data rate: %u bytes per cycle (high part: %u, low part: %u).\n", (uint32_t)(data_rate >> 32), (uint32_t)(data_rate >> 32), (uint32_t)(data_rate));

 // Calculate and print data rate in bytes per second
 uint64_t data_rate_bps = (10 * CPU_FREQUENCY) / duration;
 neorv32_uart0_printf("Data rate: %u bytes per second (high part: %u, low part: %u).\n", (uint32_t)(data_rate_bps >> 32), (uint32_t)(data_rate_bps >> 32), (uint32_t)(data_rate_bps));

 return 0;

}


/**********************************************************************//**
 * SLINK interrupt handler.
 **************************************************************************/
void slink_firq_handler(void) {
  neorv32_slink_get();
  //neorv32_uart0_printf(" <<RX data: %i>> ", neorv32_slink_get());
}
