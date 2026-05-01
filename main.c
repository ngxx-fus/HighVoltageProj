#include "main.h"

int main(void) {
    AppInit();

    OLEDSayHello();

    UIMode = eUI_WORKING_MODE_PAUSED;
    SafeFlagSet(&UIFlag, eUI_REQ_UPDATE);

    SysLog("main(...): Toggle to indicate sleeping");
    while (1) {
        if(SafeFlagHas(&UIFlag, eUI_REQ_UPDATE)){
            SafeFlagClear(&UIFlag, eUI_REQ_UPDATE);
            SysLog("main(...): Updating UI...");
            switch (UIMode) {
                case eUI_CONFIG_FREQ_MODE:
                case eUI_CONFIG_DUTY_MODE:
                    UI_ConfigState();
                    PWM_Config(
                        PWMFreqSupportedList[CurrentFreq],
                        PWMDutySupportedList[CurrentDuty]
                    );
                    break;
                case eUI_WORKING_MODE_NORMAL:
                    UI_WorkingState();
                    PWM_OutputStart();
                    break;
                case eUI_WORKING_MODE_PAUSED:
                    PWM_OutputStop();
                    UI_WorkingState();
                    break;
                
                default:
                    break;
            }
        }
        BusyWaitMs(50);
    }
}
