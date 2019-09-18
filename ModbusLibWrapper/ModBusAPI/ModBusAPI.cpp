// ModBusAPI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "modbus.h"

LIBRARY_API int GetRegisterValue(int Register)
{
	modbus_t *mb;
	uint16_t tab_reg[32];

	memset(tab_reg, -1, sizeof(tab_reg));

	mb = modbus_new_tcp("127.0.0.1", 502);

	modbus_connect(mb);

//	modbus_read_registers(mb, Register, 1, tab_reg);
	modbus_read_input_registers(mb, Register, 1, tab_reg);

	modbus_close(mb);
	modbus_free(mb);

	return tab_reg[0];
}