/* Copyright © 2023 Georgy E. All rights reserved. */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>


#define DEVICE_MAJOR (0)
#define DEVICE_MINOR (1)
#define DEVICE_PATCH (0)


/*
 * Device types:
 * 0x0001 - Dispenser
 * 0x0002 - Gas station
 * 0x0003 - Logger
 * 0x0004 - B.O.B.A.
 * 0x0005 - Calibrate station
 * 0x0006 - Dispenser-mini
 */
#define DEVICE_TYPE ((uint16_t)0x0005)
#define SW_VERSION  ((uint8_t)0x01)
#define FW_VERSION  ((uint8_t)0x01)
#define CF_VERSION  ((uint8_t)0x01)


typedef enum _SettingsStatus {
    SETTINGS_OK = 0,
    SETTINGS_ERROR
} SettingsStatus;


typedef struct __attribute__((packed)) _settings_t  {
	// Device type
	uint16_t dv_type;
	// Software version
    uint8_t  sw_id;
    // Firmware version
    uint8_t  fw_id;
} settings_t;


extern settings_t settings;


/* copy settings to the target */
settings_t* settings_get();
/* copy settings from the target */
void settings_set(settings_t* other);
/* reset current settings to default */
void settings_reset(settings_t* other);

uint32_t settings_size();

bool settings_check(settings_t* other);
void settings_repair(settings_t* other);

void settings_show();


#ifdef __cplusplus
}
#endif


#endif
