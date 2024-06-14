/* Copyright © 2023 Georgy E. All rights reserved. */

#ifndef _APP_H_
#define _APP_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


typedef struct _app_info_t {
	uint8_t  start;
	uint32_t target_ml;
	uint32_t level;
	uint32_t result_ml;
} app_info_t;


extern app_info_t app_info;


void app_init();
void app_proccess();


#ifdef __cplusplus
}
#endif


#endif
