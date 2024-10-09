/* Copyright © 2023 Georgy E. All rights reserved. */

#ifndef _APP_H_
#define _APP_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

#include "STM32_adapter_BA_BLE.h"


typedef struct _app_info_t {
    uint8_t  start;
    int32_t  target_ml;
    uint32_t level_adc;
    int32_t  result_ml;
    unsigned status;
    uint8_t  sens_addr;
    uint8_t  last_addr;
} app_info_t;


extern app_info_t app_info;

extern Adapter_BA_BLE ba_ble;


void app_init();
void app_proccess();


#ifdef __cplusplus
}
#endif


#endif
