#include "BuiltinLED.h"

/* Register Definitions */
#define RCC_APB2ENR    *(volatile uint32_t *)0x40021018
#define GPIOC_CRH      *(volatile uint32_t *)0x40011004
#define GPIOC_ODR      *(volatile uint32_t *)0x4001100C

/// @brief Initialize PC13 as Output Push-Pull for Built-in LED
/// @param void
/// @return None
void BuiltinLED_Init(void) {
    /* Enable GPIOC clock and configure PC13 to Output Push-Pull */
    RCC_APB2ENR |= (1 << 4);
    GPIOC_CRH = (GPIOC_CRH & ~(0xF << 20)) | (0x2 << 20); 
}

/// @brief Turn on the built-in LED (Active Low)
/// @param void
/// @return None
void BuiltinLED_TurnOn(void) {
    /* Pull PC13 low to turn ON the LED */
    GPIOC_ODR &= ~(1 << 13);
}

/// @brief Turn off the built-in LED (Active Low)
/// @param void
/// @return None
void BuiltinLED_TurnOff(void) {
    /* Pull PC13 high to turn OFF the LED */
    GPIOC_ODR |= (1 << 13);
}

/// @brief Toggle the current state of the built-in LED
/// @param void
/// @return None
void BuiltinLED_ToggleState(void) {
    /* Invert the current logic level of PC13 */
    GPIOC_ODR ^= (1 << 13);
}

/// @brief Blink the built-in LED a specific number of times
/// @param Count Number of blink cycles
/// @param usec Delay between toggles in microseconds
/// @return None
void BuiltinLED_Blink(Dword_t Count, Dword_t usec) {
    /* Toggle LED state and wait repeatedly */
    for (Dword_t i = 0; i < Count; i++) {
        GPIOC_ODR ^= (1 << 13);
        DelayUS(usec);
        GPIOC_ODR ^= (1 << 13);
        DelayUS(usec);
    }
}

/// @brief Blink the built-in LED forever (blocking)
/// @param usec Delay between toggles in microseconds
/// @return None
void BuiltinLED_BlinkForever(Dword_t usec) {
    /* Continuously toggle the LED in an infinite loop */
    while (1) {
        GPIOC_ODR ^= (1 << 13);
        DelayUS(usec);
    }
}