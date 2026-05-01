#ifndef __UI_H__
#define __UI_H__

#include "../../AppUtils/All.h"
#include "../../AppLog/SerialLog.h"

#define TEXT_LINE_0     24, 4
#define TEXT_LINE_1     48, 4

/*
    BTN_MENU:   Change menu, change mode
    BTN_ALT:    Increase value, pause
    
    CURRENT_MODE    BTN_MENU    BTN_ALT     NEXT_MODE   VALUE
    N               NEGEDGE     X           N+1         DON'T CHANGE
    N               HIGH        NEGEDGE     N           CycleAdd(+1)

*/

enum eUIFlagBit {
    eUI_FLUSH               = 0U,
    eUI_NEXT_MODE           = 1U,
    eUI_REQ_UPDATE          = 31U,
};

enum eUIMode {
    eUI_CONFIG_FREQ_MODE    = 0U,
    eUI_CONFIG_DUTY_MODE    = 1U,
    eUI_WORKING_MODE_PAUSED = 2U,
    eUI_WORKING_MODE_NORMAL = 3U,
    eUI_MODE_COUNT
};

extern volatile SafeFlag_t      UIFlag;
extern volatile enum eUIMode    UIMode;

#endif /// __UI_H__