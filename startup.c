#include <stdint.h>

extern uint32_t _etext, _sdata, _edata, _sbss, _ebss;
void main(void);

/// @brief Reset Handler called on CPU startup
/// @param None
/// @return None
void Reset_Handler(void) {
    /* Place the actual comment here */
    /* Copy .data section from FLASH to SRAM */
    uint32_t size = (uint8_t *)&_edata - (uint8_t *)&_sdata;
    uint8_t *pDst = (uint8_t *)&_sdata;
    uint8_t *pSrc = (uint8_t *)&_etext;
    for (uint32_t i = 0; i < size; i++) *pDst++ = *pSrc++;

    /* Initialize .bss section to zero */
    size = (uint8_t *)&_ebss - (uint8_t *)&_sbss;
    pDst = (uint8_t *)&_sbss;
    for (uint32_t i = 0; i < size; i++) *pDst++ = 0;

    main();
    while (1);
}

/* Default handler used for unused interrupts */
void Default_Handler(void) { while (0); }

/* Core exception handlers (weak aliases to Default_Handler) */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/* Forward declarations for external IRQ handlers implemented elsewhere */
extern void EXTI0_IRQHandler(void);
extern void EXTI1_IRQHandler(void);

/* Vector Table for STM32F103 */
/* Stack Pointer initial value at 0x20005000 (End of 20KB RAM) */
__attribute__((section(".vector_table")))
uint32_t vectors[] = {
    0x20005000,               // Initial Stack Pointer
    (uint32_t)Reset_Handler,  // Reset Handler
    (uint32_t)NMI_Handler,
    (uint32_t)HardFault_Handler,
    (uint32_t)MemManage_Handler,
    (uint32_t)BusFault_Handler,
    (uint32_t)UsageFault_Handler,
    0,
    0,
    0,
    0,
    (uint32_t)SVC_Handler,
    (uint32_t)DebugMon_Handler,
    0,
    (uint32_t)PendSV_Handler,
    (uint32_t)SysTick_Handler,
    /* External IRQs (IRQ0 .. ) */
    (uint32_t)Default_Handler, /* IRQ0 */
    (uint32_t)Default_Handler, /* IRQ1 */
    (uint32_t)Default_Handler, /* IRQ2 */
    (uint32_t)Default_Handler, /* IRQ3 */
    (uint32_t)Default_Handler, /* IRQ4 */
    (uint32_t)Default_Handler, /* IRQ5 */
    (uint32_t)EXTI0_IRQHandler,/* IRQ6 - EXTI0 */
    (uint32_t)EXTI1_IRQHandler /* IRQ7 - EXTI1 */
};
