#include <stdio.h>
#include "modbus.h"
#include <conio.h>

int main(void) {
  modbus_t *mb;
  uint16_t tab_reg[32];

  memset(tab_reg, 0, sizeof(tab_reg));

  mb = modbus_new_tcp("127.0.0.1", 502);
  modbus_set_debug(mb, 3);

  modbus_connect(mb);

  /* Read 5 registers from the address 0 */
//  modbus_read_registers(mb, 0, 5, tab_reg);
  modbus_read_input_registers(mb, 0, 5, tab_reg);
  printf("Register values : %d %d %d %d %d", tab_reg[0], tab_reg[1], tab_reg[2], tab_reg[3], tab_reg[4]);

  modbus_close(mb);
  modbus_free(mb);
  _getch();
}