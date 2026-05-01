#ifndef __BUILT_IN_LED_H__
#define __BUILT_IN_LED_H__

#include "../../AppUtils/All.h"

/// @brief Initialize PC13 as Output Push-Pull for Built-in LED
/// @param void
/// @return None
void BuiltinLED_Init(void);

/// @brief Turn on the built-in LED (Active Low)
/// @param void
/// @return None
void BuiltinLED_TurnOn(void);

/// @brief Turn off the built-in LED (Active Low)
/// @param void
/// @return None
void BuiltinLED_TurnOff(void);

/// @brief Toggle the current state of the built-in LED
/// @param void
/// @return None
void BuiltinLED_ToggleState(void);

/// @brief Blink the built-in LED a specific number of times
/// @param Count Number of blink cycles
/// @param usec Delay between toggles in microseconds
/// @return None
void BuiltinLED_Blink(Dword_t Count, Dword_t usec);

/// @brief Blink the built-in LED forever (blocking)
/// @param usec Delay between toggles in microseconds
/// @return None
void BuiltinLED_BlinkForever(Dword_t usec);

#endif /// __BUILT_IN_LED_H__