/* Copyright © 2023 Georgy E. All rights reserved. */

#include "settings.h"

#include <string.h>
#include <stdbool.h>

#include "glog.h"
#include "main.h"
#include "soul.h"
#include "gutils.h"
#include "hal_defs.h"


static const char SETTINGS_TAG[] = "STNG";

settings_t settings = { 0 };


settings_t* settings_get()
{
	return &settings;
}

void settings_set(settings_t* other)
{
	memcpy((uint8_t*)&settings, (uint8_t*)other, sizeof(settings));
	if (!settings_check(&settings)) {
		settings_repair(&settings);
	}
}

void settings_reset(settings_t* other)
{
	printTagLog(SETTINGS_TAG, "Reset settings");
	other->dv_type = DEVICE_TYPE;
	other->sw_id   = SW_VERSION;
	other->fw_id   = FW_VERSION;
}

uint32_t settings_size()
{
	return sizeof(settings_t);
}

bool settings_check(settings_t* other)
{
	if (other->dv_type != DEVICE_TYPE) {
		return false;
	}
	if (other->sw_id != SW_VERSION) {
		return false;
	}
	if (other->fw_id != FW_VERSION) {
		return false;
	}
	return true;
}

void settings_repair(settings_t* other)
{
	printTagLog(SETTINGS_TAG, "Repair settings");

	set_status(NEED_SAVE_SETTINGS);

	if (other->fw_id != FW_VERSION) {
		other->fw_id = FW_VERSION;
	}

	if (!settings_check(other)) {
		settings_reset(other);
	}
}

void settings_show()
{
	printPretty("################SETTINGS################\n");
    printPretty("Device type: %u\n", settings.dv_type);
	printPretty("Software v%u\n", settings.sw_id);
	printPretty("Firmware v%u\n", settings.fw_id);
	printPretty("################SETTINGS################\n");
}
