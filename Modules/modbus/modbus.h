/* Copyright © 2023 Georgy E. All rights reserved. */

#ifndef _MODBUS_H_
#define _MODBUS_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


void modbus_init();
void modbus_tick();
void modbus_recieve(uint8_t byte);



#ifdef __cplusplus
}
#endif


#endif
