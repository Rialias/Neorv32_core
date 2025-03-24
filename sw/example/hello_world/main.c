// ================================================================================ //
// The NEORV32 RISC-V Processor - https://github.com/stnolting/neorv32              //
// Copyright (c) NEORV32 contributors.                                              //
// Copyright (c) 2020 - 2025 Stephan Nolting. All rights reserved.                  //
// Licensed under the BSD-3-Clause license, see LICENSE for details.                //
// SPDX-License-Identifier: BSD-3-Clause                                            //
// ================================================================================ //

#include <neorv32.h>
#include <string.h>

/**@{*/
/** UART BAUD rate */
#define BAUD_RATE 19200
/**@}*/

int main()
{

  // capture all exceptions and give debug info via UART
  // this is not required, but keeps us safe
  neorv32_rte_setup();

  // setup UART at default baud rate, no interrupts
  neorv32_uart0_setup(BAUD_RATE, 0);

  neorv32_cpu_delay_ms(5000);
  // print project logo via UART
  char buffer[512];
  size_t length = 0;
  for (;;)
  {
    neorv32_uart0_printf("waiting for message");
    length = neorv32_uart0_scan(buffer, sizeof(buffer), 1);
    neorv32_uart0_printf("\n");

    if (!length) // nothing to be done
      continue;

    if (!strcmp(buffer, "helloWorld"))
    {
      for (int i = 0; i < 200; i++)
      {
        neorv32_uart0_printf("%d\n", i);
      }
    }
  }
  return 0;
}