/* Copyright © 2023 Georgy E. All rights reserved. */

#ifndef _LEVEL_H_
#define _LEVEL_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


void     level_tick();
uint32_t get_liquid_adc();


#ifdef __cplusplus
}
#endif


#endif
