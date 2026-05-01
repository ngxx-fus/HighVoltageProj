#include "PWM.h"

/* --- REGISTERS FOR TIM2 & GPIOA (PA0 = TIM2_CH1) --- */
#define RCC_APB2ENR     *(volatile uint32_t *)0x40021018
#define RCC_APB1ENR     *(volatile uint32_t *)0x4002101C
#define GPIOA_CRL       *(volatile uint32_t *)0x40010800
#define TIM2_CR1        *(volatile uint32_t *)0x40000000
#define TIM2_CCMR1      *(volatile uint32_t *)0x40000018
#define TIM2_CCER       *(volatile uint32_t *)0x40000020
#define TIM2_PSC        *(volatile uint32_t *)0x40000028
#define TIM2_ARR        *(volatile uint32_t *)0x4000002C
#define TIM2_CCR1       *(volatile uint32_t *)0x40000034
#define TIM2_EGR        *(volatile uint32_t *)0x40000014

/// @brief List of supported PWM frequencies mapping labels to actual values in Hz
const PWMFrequency_t PWMFreqSupportedList[] = {
    {"10Hz",      10U},
    {"100Hz",     100U},
    {"500Hz",     500U},
    {"1KHz",      1000U},
    {"2.5KHz",    2500U},
    {"5KHz",      5000U},
    {"7.5KHz",    7500U},
    {"10KHz",     10000U},
    {"15KHz",     15000U},
    {"20KHz",     20000U},
    {"30KHz",     30000U},
    {"50KHz",     50000U},
    {"100KHz",    100000U},
    {"200KHz",    200000U},
    {"300KHz",    300000U},
    {"400KHz",    400000U},
    {"500KHz",    500000U},
    {"700KHz",    700000U},
    {"1MHz",      1000000U}
};

const uint8_t PWMFreqSupportedListSize = sizeof(PWMFreqSupportedList)/sizeof(PWMFrequency_t);

/// @brief List of supported PWM duty cycles mapping labels to actual values
const PWMDutyCycle_t PWMDutySupportedList[] = {
    {"10%",      10U},
    {"20%",      20U},
    {"30%",      30U},
    {"40%",      40U},
    {"50%",      50U},
    {"60%",      60U},
    {"70%",      70U},
    {"80%",      80U},
    {"90%",      90U}
};

const uint8_t PWMDutySupportedListSize = sizeof(PWMDutySupportedList)/sizeof(PWMDutyCycle_t);

volatile uint8_t CurrentFreq = 0U;
volatile uint8_t CurrentDuty = 4U;

/// @brief Initialize TIM2 Channel 1 (PA0) for PWM output
/// @return STAT_OKE if success
ReturnCode_t PWM_Init(void) {
    /* Enable APB clocks for GPIOA and TIM2, configure PA0 as AF Push-Pull */
    RCC_APB2ENR |= (1 << 2);
    RCC_APB1ENR |= (1 << 0);
    GPIOA_CRL = (GPIOA_CRL & ~0x0000000F) | 0x0000000B;

    /* Set TIM2 Channel 1 to PWM Mode 1 and enable preload */
    TIM2_CCMR1 = (TIM2_CCMR1 & ~0x00000070) | (0x06 << 4) | (1 << 3);
    TIM2_CR1 |= (1 << 7);

    /* Set default state to 1KHz, 50% and stopped */
    PWM_Config(PWMFreqSupportedList[0], PWMDutySupportedList[4]);

    return STAT_OKE;
}

/// @brief Start the PWM output generation
/// @return STAT_OKE if success
ReturnCode_t PWM_OutputStart(void) {
    /* Enable TIM2 Channel 1 output and start counter */
    TIM2_CCER |= (1 << 0);
    TIM2_CR1 |= (1 << 0);
    return STAT_OKE;
}

/// @brief Stop the PWM output generation
/// @return STAT_OKE if success
ReturnCode_t PWM_OutputStop(void) {
    /* Disable TIM2 Channel 1 output and stop counter */
    TIM2_CCER &= ~(1 << 0);
    TIM2_CR1 &= ~(1 << 0);
    return STAT_OKE;
}

/// @brief Configure PWM frequency and duty cycle, strictly accepting only preset values
/// @param Freq Selected frequency from supported list
/// @param Duty Selected duty cycle from supported list
/// @return STAT_OKE if success, STAT_ERR if invalid preset
ReturnCode_t PWM_Config(PWMFrequency_t Freq, PWMDutyCycle_t Duty) {
    uint8_t validFreqIdx = 0xFF;
    uint8_t validDutyIdx = 0xFF;

    /* Validate Freq against the supported preset list */
    for (uint8_t i = 0; i < 16; i++) {
        if (Freq.Value == PWMFreqSupportedList[i].Value) {
            validFreqIdx = i;
            break;
        }
    }

    /* Validate Duty against the supported preset list */
    for (uint8_t i = 0; i < 9; i++) {
        if (Duty.Value == PWMDutySupportedList[i].Value) {
            validDutyIdx = i;
            break;
        }
    }

    /* Reject the configuration if parameters do not match any preset */
    if (validFreqIdx == 0xFF || validDutyIdx == 0xFF) {
        return STAT_ERR;
    }

    /* Stop PWM output immediately per requirement */
    PWM_OutputStop();

    /* Update tracking indices with the validated preset locations */
    CurrentFreq = validFreqIdx;
    CurrentDuty = validDutyIdx;

    /* Calculate hardware timer parameters */
    uint32_t sys_clk = 72000000U;
    uint32_t psc = 0;
    uint32_t arr = (sys_clk / Freq.Value) - 1;

    /* Adjust prescaler if auto-reload value exceeds 16-bit hardware limit */
    if (arr > 65535U) {
        psc = (arr / 65535U) + 1;
        arr = (sys_clk / (psc + 1) / Freq.Value) - 1;
    }

    /* Apply configurations to TIM2 registers and force update */
    TIM2_PSC = psc;
    TIM2_ARR = arr;
    TIM2_CCR1 = ((arr + 1) * Duty.Value) / 100;
    TIM2_EGR |= (1 << 0);

    return STAT_OKE;
}