#include <stdint.h>
#include "./AppComm/All.h"
#include "./AppUtils/All.h"
#include "./AppLog/SerialLog.h"

void OLEDSayHello(void) {
    SysEntry("OLEDSayHello(...)");

    OLED_ShowRAMContent();
    OLED_FillScreen(0);
    OLED_DrawEmptyRect(0, 0, 63, 127, 2, 1);
    OLED_DrawText(TEXT_LINE_0, "PWM TOOL", 1, true, 0);
    OLED_DrawText(TEXT_LINE_1, "40KHZ | 50%", 1, true, 0);
    OLED_Flush();

    SysExit("OLEDSayHello: CODE:NULL");
}

void IsrHandler0(void *){
    /*INCR/PAUSE/PLAY*/
    SysLog("IsrHandler0(...): Was called!");
    switch (UIMode){
        case eUI_WORKING_MODE_PAUSED:
            UIMode = eUI_WORKING_MODE_NORMAL;
            SysLog("IsrHandler0(...): Changed to MODE%d", UIMode);
            break;
        case eUI_WORKING_MODE_NORMAL:
            UIMode = eUI_WORKING_MODE_PAUSED;
            SysLog("IsrHandler0(...): Changed to MODE%d", UIMode);
            break;
        case eUI_CONFIG_FREQ_MODE:
        CurrentFreq = (CurrentFreq+1)%PWMFreqSupportedListSize;
            SysLog("IsrHandler0(...): Freq=%s", PWMFreqSupportedList[CurrentFreq].Label);
            break;
        case eUI_CONFIG_DUTY_MODE:
            CurrentDuty = (CurrentDuty+1)%PWMDutySupportedListSize;
            SysLog("IsrHandler0(...): Duty=%s", PWMDutySupportedList[CurrentFreq].Label);
            break;
        default:
            SysErr("IsrHandler0(...): Invalid MODE!");
            break;
    }
    SafeFlagSet(&UIFlag, eUI_REQ_UPDATE);
}

void IsrHandler1(void *){
    /*MENU*/
    switch (UIMode){
        case eUI_WORKING_MODE_PAUSED:
        case eUI_WORKING_MODE_NORMAL:
            UIMode = eUI_CONFIG_FREQ_MODE;
            break;
        case eUI_CONFIG_FREQ_MODE:
            UIMode = eUI_CONFIG_DUTY_MODE;
            break;
        case eUI_CONFIG_DUTY_MODE:
            UIMode = eUI_WORKING_MODE_PAUSED;
            break;
        default:
            SysErr("IsrHandler1(...): Invalid MODE!");
            break;
    }
    SysLog("IsrHandler1(...): Changed to MODE%d", UIMode);
    SafeFlagSet(&UIFlag, eUI_REQ_UPDATE);
}


void UI_ConfigState(){
    char TempStr[50];
    int retval;
    SysEntry("UI_ConfigState");

    OLED_FillScreen(0);
    OLED_DrawEmptyRect(0, 0, 63, 127, 2, 1);
    switch (UIMode) {
        case eUI_CONFIG_FREQ_MODE:
                OLED_DrawText(TEXT_LINE_0, "CONF [FREQ]", 1, true, 0);
            break;
        case eUI_CONFIG_DUTY_MODE:
                OLED_DrawText(TEXT_LINE_0, "CONF [DUTY]", 1, true, 0);
            break;
        default:
                OLED_DrawText(TEXT_LINE_0, "CONF [DUTY]", 1, true, 0);
                goto EXIT_SECTION;
            break;
    }

    /* Safely format PWM status string */
    memset(TempStr, 0, sizeof(TempStr));
    retval = snprintf(
        TempStr, sizeof(TempStr), 
        "> %s/%s",
        PWMFreqSupportedList[CurrentFreq].Label,
        PWMDutySupportedList[CurrentDuty].Label
    );
    SysLog("UI_WorkingState: snprintf returned %d, TempStr='%s'", retval, TempStr);

    OLED_DrawText(TEXT_LINE_1, TempStr, 1, true, 0);
    
    EXIT_SECTION:
    OLED_Flush();
    SysExit("UI_ConfigState");
}

void UI_WorkingState(){
    char TempStr[50];
    int retval;
    SysEntry("UI_WorkingState");
    
    OLED_FillScreen(0);
    OLED_DrawEmptyRect(0, 0, 63, 127, 2, 1);

    if(UIMode == eUI_WORKING_MODE_PAUSED){
        OLED_DrawText(TEXT_LINE_0, "PWM [STOP]", 1, true, 0);
    }else 
    if(UIMode == eUI_WORKING_MODE_NORMAL){
        OLED_DrawText(TEXT_LINE_0, "PWM [START]", 1, true, 0);
    } else {
        OLED_DrawText(TEXT_LINE_0, "PWM ERROR!", 1, true, 0);
        goto EXIT_SECTION;
    }

    /* Safely format PWM status string */
    memset(TempStr, 0, sizeof(TempStr));
    retval = snprintf(
        TempStr, sizeof(TempStr), 
        "> %s/%s",
        PWMFreqSupportedList[CurrentFreq].Label,
        PWMDutySupportedList[CurrentDuty].Label
    );

    OLED_DrawText(TEXT_LINE_1, TempStr, 1, true, 0);
    EXIT_SECTION:
    OLED_Flush();
    SysExit("UI_WorkingState");
}

void AppInit(){
    int32_t RetVal;

    Log_Init();
    SysEntry("AppInit");

    PWM_Init();
    BuiltinLED_Init();

    IntHandler[0] = IsrHandler0;
    IntHandler[1] = IsrHandler1;
    INT_Init();

    SysLog("main(...): Init OLED");
    RetVal = OLED_Init();
    if (RetVal != STAT_OKE) {
        SysErr("main(...): OLED_Init failed, CODE=%d", RetVal);
        SysLog("main(...): Put to toggle LED mode to raise ERROR!");
        I2C1_StopTransaction();
        BuiltinLED_BlinkForever(120);
    }

    SysExit("AppInit");
}