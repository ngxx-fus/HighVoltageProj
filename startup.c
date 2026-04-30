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

/* Vector Table for STM32F103 */
/* Stack Pointer initial value at 0x20005000 (End of 20KB RAM) */
__attribute__((section(".vector_table")))
uint32_t vectors[] = {
    0x20005000,           // Initial Stack Pointer
    (uint32_t)Reset_Handler // Reset Handler
};
