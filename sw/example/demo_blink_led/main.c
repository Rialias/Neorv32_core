// ================================================================================ //
// The NEORV32 RISC-V Processor - https://github.com/stnolting/neorv32              //
// Copyright (c) NEORV32 contributors.                                              //
// Copyright (c) 2020 - 2024 Stephan Nolting. All rights reserved.                  //
// Licensed under the BSD-3-Clause license, see LICENSE for details.                //
// SPDX-License-Identifier: BSD-3-Clause                                            //
// ================================================================================ //

/**********************************************************************//**
 * @file demo_blink_led/main.c
 * @author Riya Alias
 * @brief Minimal blinking LED demo program along with classic hello world program
 *
 **************************************************************************/
#include <neorv32.h>

/**********************************************************************//**
 * @name User configuration
 **************************************************************************/
/**@{*/
/** UART BAUD rate */
#define BAUD_RATE 19200
/**@}*/


/**********************************************************************//**
 * Main function; shows an incrementing 8-bit counter on GPIO.output(7:0).
 *
 * @note This program requires the GPIO controller to be synthesized.
 *
 * @return Will never return.
 **************************************************************************/
int main() {

  // clear GPIO output (set all bits to 0)
  neorv32_gpio_port_set(0);

  int cnt = 0;

  // capture all exceptions and give debug info via UART
  // this is not required, but keeps us safe
  neorv32_rte_setup();

  // setup UART at default baud rate, no interrupts
  neorv32_uart0_setup(BAUD_RATE, 0);

  // print project logo via UART
  neorv32_rte_print_logo();


  while (1) {
    neorv32_gpio_port_set(cnt++ & 0xFF); // increment counter and mask for lowest 8 bit
    neorv32_cpu_delay_ms(250); // wait 250ms using busy wait
    // say hello
    neorv32_uart0_puts("Hello world! :)\n");
    if(cnt == 16)
    {
      cnt = 0;
    }
  }
  return 0;
}
