#include "INT.h"
#include "SerialLog.h"

/* --- REGISTERS FOR RCC, GPIOB, AFIO, EXTI, NVIC --- */
#define RCC_APB2ENR     *(volatile uint32_t *)0x40021018
#define GPIOB_CRL       *(volatile uint32_t *)0x40010C00
#define GPIOB_ODR       *(volatile uint32_t *)0x40010C0C /* Added ODR register */
#define AFIO_EXTICR1    *(volatile uint32_t *)0x40010008
#define EXTI_IMR        *(volatile uint32_t *)0x40010400
#define EXTI_FTSR       *(volatile uint32_t *)0x4001040C
#define EXTI_RTSR       *(volatile uint32_t *)0x40010408
#define EXTI_PR         *(volatile uint32_t *)0x40010414
#define NVIC_ISER0      *(volatile uint32_t *)0xE000E100
/* NVIC ICER (Interrupt Clear-Enable) to disable interrupts */
#define NVIC_ICER0      *(volatile uint32_t *)0xE000E180
/* GPIOC ODR for LED toggle (PC13) */
#define GPIOC_ODR       *(volatile uint32_t *)0x4001100C

const uint8_t IntPinList[2] = {PB0, PB1};
const uint8_t IntHandlerCount = sizeof(IntPinList) / sizeof(uint8_t); 
void (*IntHandler[2])(void* Arg) = {NULL, NULL};

/// @brief Initialize EXTI for defined pins if their handlers are not NULL
/// @param void
/// @return STAT_OKE if success
ReturnCode_t INT_Init(void) {
    SysEntry("INT_Init(...)");

    /* Enable clocks for GPIOB and AFIO */
    RCC_APB2ENR |= (1 << 3) | (1 << 0); 

    for (uint8_t i = 0; i < IntHandlerCount; i++) {
        if (IntHandler[i] != NULL) {
            uint8_t pin = IntPinList[i];
            SysLog("INT_Init(...): Enabling EXTI for PB%d", pin);

            /* Configure PB0 / PB1 as Input with Pull-up/Pull-down */
            GPIOB_CRL = (GPIOB_CRL & ~(0xF << (pin * 4))) | (0x8 << (pin * 4)); 
            
            /* FORCE PULL-UP: Set the corresponding bit in ODR to 1 */
            GPIOB_ODR |= (1 << pin);

            /* Map EXTIx line to Port B */
            AFIO_EXTICR1 = (AFIO_EXTICR1 & ~(0xF << (pin * 4))) | (0x1 << (pin * 4));
            
            /* Configure EXTI to trigger on Falling Edge */
            EXTI_FTSR |= (1 << pin);

            EXTI_PR = (1 << pin);
            
            /* Unmask (Enable) EXTI line */
            EXTI_IMR |= (1 << pin);

            /* Ensure only Falling trigger is enabled (clear Rising) */
            EXTI_RTSR &= ~(1 << pin);

            /* Enable EXTI0 (IRQ 6) or EXTI1 (IRQ 7) in NVIC (write-1 to ISER) */
            if (pin == 0) NVIC_ISER0 = (1 << 6);
            if (pin == 1) NVIC_ISER0 = (1 << 7);
        } 
        else {
            uint8_t pin = IntPinList[i];
            SysLog("INT_Init(...): Disabling EXTI for PB%d", pin);

            /* Mask (Disable) EXTI line if handler is NULL */
            EXTI_IMR &= ~(1 << pin);
            
            /* Disable NVIC Interrupts using ICER */
            if (pin == 0) NVIC_ICER0 = (1 << 6);
            if (pin == 1) NVIC_ICER0 = (1 << 7);
        }
    }
    
    SysExit("INT_Init(...): CODE: STAT_OKE");
    return STAT_OKE;
}

/// @brief Set a new handler for a specific interrupt index and re-initialize
/// @param Index Index in the IntPinList array
/// @param NewHandler Pointer to the handler function
/// @return STAT_OKE if success, STAT_ERR if index out of bounds
ReturnCode_t INT_SetIntHandler(uint8_t Index, void (*NewHandler)(void*)) {
    SysEntry("INT_SetIntHandler(...): Index=%d", Index);

    /* Validate bounds before updating handler array */
    if (Index >= IntHandlerCount) {
        SysErr("INT_SetIntHandler(...): Index out of bounds!");
        SysExit("INT_SetIntHandler(...): CODE: STAT_ERR");
        return STAT_ERR;
    }
    
    IntHandler[Index] = NewHandler;
    ReturnCode_t ret = INT_Init();
    
    /* Log the final result based on INT_Init execution */
    if (ret == STAT_OKE) {
        SysExit("INT_SetIntHandler(...): CODE: STAT_OKE");
    }
    else {
        SysErr("INT_SetIntHandler(...): INT_Init failed!");
        SysExit("INT_SetIntHandler(...): CODE: STAT_ERR");
    }
    return ret;
}

/// @brief Clear the handler for a specific interrupt index and re-initialize
/// @param Index Index in the IntPinList array
/// @return STAT_OKE if success, STAT_ERR if index out of bounds
ReturnCode_t INT_ClearIntHandler(uint8_t Index) {
    SysEntry("INT_ClearIntHandler(...): Index=%d", Index);

    /* Validate bounds before clearing handler */
    if (Index >= IntHandlerCount) {
        SysErr("INT_ClearIntHandler(...): Index out of bounds!");
        SysExit("INT_ClearIntHandler(...): CODE: STAT_ERR");
        return STAT_ERR;
    }
    
    IntHandler[Index] = NULL;
    ReturnCode_t ret = INT_Init();
    
    /* Log the final result based on INT_Init execution */
    if (ret == STAT_OKE) {
        SysExit("INT_ClearIntHandler(...): CODE: STAT_OKE");
    }
    else {
        SysErr("INT_ClearIntHandler(...): INT_Init failed!");
        SysExit("INT_ClearIntHandler(...): CODE: STAT_ERR");
    }
    return ret;
}

/* --- EXTI Interrupt Request Handlers --- */

/// @brief Hardware interrupt handler for EXTI Line 0 (PB0)
/// @return None
void EXTI0_IRQHandler(void) {
    /* Check if EXTI0 triggered */
    if (EXTI_PR & (1 << 0)) {
        /* Clear pending bit by writing 1 */
        EXTI_PR = (1 << 0);
        
        SysLog("EXTI0_IRQHandler(...): Triggered");
        /* Quick visible action: toggle PC13 so user can observe ISR */
        GPIOC_ODR ^= (1 << 13);
        
        /* Execute callback if valid */
        if (IntHandler[0] != NULL) {
            IntHandler[0](NULL);
        }
    }
}

/// @brief Hardware interrupt handler for EXTI Line 1 (PB1)
/// @return None
void EXTI1_IRQHandler(void) {
    /* Check if EXTI1 triggered */
    if (EXTI_PR & (1 << 1)) {
        /* Clear pending bit by writing 1 */
        EXTI_PR = (1 << 1);
        
        SysLog("EXTI1_IRQHandler(...): Triggered");
        /* Quick visible action: toggle PC13 twice for EXTI1 */
        GPIOC_ODR ^= (1 << 13);
        GPIOC_ODR ^= (1 << 13);
        
        /* Execute callback if valid */
        if (IntHandler[1] != NULL) {
            IntHandler[1](NULL);
        }
    }
}