/* Copyright © 2023 Georgy E. All rights reserved. */

#ifndef _LEVEL_H_
#define _LEVEL_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#include "hal_defs.h"

void     level_tick();
int32_t  get_liquid_level();
uint16_t get_liquid_adc();
bool     is_liquid_tank_empty();


#ifdef __cplusplus
}
#endif


#endif
