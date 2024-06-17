/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Watchdogs.h"

#include "soul.h"
#include "main.h"
#include "settings.h"

#include "CodeStopwatch.h"



SettingsWatchdog::SettingsWatchdog()
{
	settings_reset(&settings);
	set_status(WAIT_LOAD);
}

void SettingsWatchdog::check()
{
	utl::CodeStopwatch stopwatch("STNG", GENERAL_TIMEOUT_MS);

	if (!settings_check(&settings)) {
		settings_repair(&settings);
	}
}
