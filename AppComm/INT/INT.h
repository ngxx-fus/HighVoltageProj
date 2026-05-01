#ifndef INT_H
#define INT_H

#include "stdint.h"
#include <stddef.h>
#include "../../AppUtils/All.h"

/* Define GPIO Pins for Interrupts */
#define PB0 0
#define PB1 1

/* Declare global arrays for pin list and handlers */
extern const uint8_t IntPinList[2];
extern const uint8_t IntHandlerCount;
extern void (*IntHandler[2])(void* Arg);

ReturnCode_t INT_Init(void);
ReturnCode_t INT_SetIntHandler(uint8_t Index, void (*NewHandler)(void*));
ReturnCode_t INT_ClearIntHandler(uint8_t Index);

#endif /* INT_H */