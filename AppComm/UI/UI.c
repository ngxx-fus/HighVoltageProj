#include "UI.h"

volatile SafeFlag_t     UIFlag = eUI_REQ_UPDATE;
volatile enum eUIMode   UIMode = eUI_WORKING_MODE_PAUSED;

