#ifndef PWM_H
#define PWM_H

#include "stdint.h"
#include "../../AppUtils/All.h"

typedef struct PWMFrequency_t {
    char        Label[32];
    uint32_t    Value;
} PWMFrequency_t;

typedef struct PWMDutyCycle_t {
    char        Label[32];
    uint8_t     Value;
} PWMDutyCycle_t;

/* Declare supported lists to be defined in .c file */
extern PWMFrequency_t PWMFreqSupportedList[16];
extern PWMDutyCycle_t PWMDutySupportedList[9];

extern volatile uint32_t PWMFreq;
extern volatile uint32_t PWMDuti;

ReturnCode_t PWM_Init(void);
ReturnCode_t PWM_OutputStart(void);
ReturnCode_t PWM_OutputStop(void);
ReturnCode_t PWM_Config(PWMFrequency_t Freq, PWMDutyCycle_t Duty);

#endif /* PWM_H */