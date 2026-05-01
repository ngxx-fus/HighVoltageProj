#include <stdint.h>
#include "./AppComm/All.h"
#include "./AppUtils/All.h"
#include "./AppLog/SerialLog.h"

/* Register Definitions */
#define RCC_APB2ENR    *(volatile uint32_t *)0x40021018
#define GPIOC_CRH      *(volatile uint32_t *)0x40011004
#define GPIOC_ODR      *(volatile uint32_t *)0x4001100C

void GPIO_Init(void) {
    /* Enable GPIOC clock */
    RCC_APB2ENR |= (1 << 4);

    /* Configure PC13 as Output Push-Pull, 2MHz */
    GPIOC_CRH &= ~(0xF << 20); // Clear bits for PC13
    GPIOC_CRH |= (0x2 << 20);  // Mode: Output, CNF: Push-Pull
}

void OLEDSayHello(void) {
    SysEntry("OLEDSayHello(...)");

    OLED_ShowRAMContent();
    OLED_FillScreen(0);
    OLED_DrawEmptyRect(0, 0, 63, 127, 2, 1);
    OLED_DrawText(24, 6, "PWM TOOL", 1, true, 0);
    OLED_DrawText(48, 6, "F: 40K D: 50%", 1, true, 0);
    OLED_Flush();

    SysExit("OLEDSayHello: CODE:NULL");
}

void IsrHandler0(void *){
    SysLog("IsrHandler0(...): Was called!");
}

void IsrHandler1(void *){
    SysLog("IsrHandler1(...): Was called!");
}


void main(void) {
    int32_t RetVal;

    GPIO_Init();
    PWM_Init();
    Log_Init();

    IntHandler[0] = IsrHandler0;
    IntHandler[1] = IsrHandler1;
    INT_Init();

    SysLog("main(...): Init OLED");
    RetVal = OLED_Init();
    if (RetVal != STAT_OKE) {
        SysErr("main(...): OLED_Init failed, CODE=%d", RetVal);
        SysLog("main(...): Put to toggle LED mode to raise ERROR!");
        I2C1_StopTransaction();
        while (1) {
            GPIOC_ODR ^= (1 << 13);
            BusyWaitMs(500);
        }
    }

    SysLog("main(...): OLED_Init success");

    /* Hardware self-test: force all pixels ON briefly */
    if (OLED_TurnOnAllPixels() == STAT_OKE) {
        SysLog("main(...): OLED all-pixels-ON test");
        BusyWaitMs(500);
    }

    OLEDSayHello();

    SysLog("main(...): Toggle to indicate sleeping");
    while (1) {
        /* No continuous toggle here so ISR-driven toggles are visible */
        BusyWaitMs(500);
    }
}
