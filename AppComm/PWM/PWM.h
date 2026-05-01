#ifndef PWM_H
#define PWM_H

#include "stdint.h"
#include "../../AppUtils/All.h"

typedef struct PWMFrequency_t {
    char        Label[16];
    uint32_t    Value;
} PWMFrequency_t;

typedef struct PWMDutyCycle_t {
    char        Label[10];
    uint8_t     Value;
} PWMDutyCycle_t;

/* Declare supported lists to be defined in .c file (const - stored in FLASH) */
extern const PWMFrequency_t PWMFreqSupportedList[];
extern const PWMDutyCycle_t PWMDutySupportedList[];

/* Store the index of the currently active preset */
extern volatile uint8_t CurrentFreq;
extern volatile uint8_t CurrentDuty;
extern const uint8_t PWMFreqSupportedListSize;
extern const uint8_t PWMDutySupportedListSize;

ReturnCode_t PWM_Init(void);
ReturnCode_t PWM_OutputStart(void);
ReturnCode_t PWM_OutputStop(void);
ReturnCode_t PWM_Config(PWMFrequency_t Freq, PWMDutyCycle_t Duty);

#endif /* PWM_H */