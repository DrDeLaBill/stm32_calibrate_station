/* Copyright © 2024 Georgy E. All rights reserved. */

#pragma once


#include "Timer.h"
#include "FiniteStateMachine.h"


#define SETTINGS_WATCHDOG_BEDUG (true)
#define POWER_WATCHDOG_BEDUG    (true)


#define WATCHDOG_TIMEOUT_MS     ((uint32_t)100)


/*
 * Filling an empty area of RAM with the STACK_CANARY_WORD value
 * For calculating the RAM fill factor
 */
extern "C" void STACK_WATCHDOG_FILL_RAM(void);


struct StackWatchdog
{
	void check();

private:
	static constexpr char TAG[] = "STCK";
	static unsigned lastFree;
};


struct RestartWatchdog
{
public:
	// TODO: check IWDG or another reboot
	void check();

	static void reset_i2c_errata();

private:
	static constexpr char TAG[] = "RSTw";
	static bool flagsCleared;

};


struct SettingsWatchdog
{
	SettingsWatchdog();

	void check();
};


struct PowerWatchdog
{
private:
	static constexpr unsigned TRIG_LEVEL = 1500;
	static constexpr uint32_t POWER_ADC_CHANNEL = 1;
	static constexpr char TAG[] = "PWRw";

	uint32_t getPower();

public:
	void check();
};
