#include "SerialLog.h"

/* --- STM32F103C8T6 REGISTERS FOR USART2 & GPIOA --- */
#define RCC_APB2ENR     *(volatile uint32_t *)0x40021018
#define RCC_APB1ENR     *(volatile uint32_t *)0x4002101C
#define GPIOA_CRL       *(volatile uint32_t *)0x40010800
#define USART2_SR       *(volatile uint32_t *)0x40004400
#define USART2_DR       *(volatile uint32_t *)0x40004404
#define USART2_BRR      *(volatile uint32_t *)0x40004408
#define USART2_CR1      *(volatile uint32_t *)0x4000440C

/// @brief Write byte stream directly to USART2 Data Register.
/// @param file File descriptor (ignored).
/// @param ptr Pointer to data buffer.
/// @param len Length of data.
/// @return Number of bytes written.
int _write(int file, char *ptr, int len) {
    /* Wait for TX buffer to be empty and send characters sequentially */
    for (int i = 0; i < len; i++) {
        while (!(USART2_SR & (1 << 7))) { }
        USART2_DR = ptr[i];
    }
    return len;
}

/// @brief Internal generic logging function to reduce memory footprint.
/// @param tag String prefix (e.g., "[LOG] ").
/// @param format Format string (printf style).
/// @param args Variable arguments list.
/// @return None
static void SysLogInternal(const char *tag, const char *format, va_list args) {
    /* Buffer for formatted string */
    char buffer[128];
    int len = 0;

    /* Prepend prefix tag */
    len = snprintf(buffer, sizeof(buffer), "%s", tag);

    /* Format the user string with arguments */
    len += vsnprintf(buffer + len, sizeof(buffer) - len, format, args);

    /* Automatically append \r\n if buffer has space */
    if (len < sizeof(buffer) - 2) {
        buffer[len++] = '\r';
        buffer[len++] = '\n';
    }

    /* Transmit the final string via bare-metal _write */
    _write(1, buffer, len);
}

void SysLog(const char *format, ...) {
    /* Wrap arguments and call internal log function */
    va_list args;
    va_start(args, format);
    SysLogInternal("[LOG] ", format, args);
    va_end(args);
}

void SysErr(const char *format, ...) {
    /* Wrap arguments and call internal log function */
    va_list args;
    va_start(args, format);
    SysLogInternal("[ERR] ", format, args);
    va_end(args);
}

void SysEntry(const char *format, ...) {
    /* Wrap arguments and call internal log function */
    va_list args;
    va_start(args, format);
    SysLogInternal("[>>>] ", format, args);
    va_end(args);
}

void SysExit(const char *format, ...) {
    /* Wrap arguments and call internal log function */
    va_list args;
    va_start(args, format);
    SysLogInternal("[<<<] ", format, args);
    va_end(args);
}

void Log_Init(void) {
    /* Enable clocks for GPIOA/USART2, configure PA2 as TX, and set baudrate */
    RCC_APB2ENR |= (1 << 2);
    RCC_APB1ENR |= (1 << 17);

    GPIOA_CRL &= ~(0xF << 8);
    GPIOA_CRL |=  (0xB << 8);

    USART2_BRR = 0x45;
    USART2_CR1 = (1 << 13) | (1 << 3);

    /* Print early boot banner */
    char Hello[] =  "\r\n\n>>>>>>>>>>>>>>>>>>>>>>>>" 
                    "HELLO!" 
                    ">>>>>>>>>>>>>>>>>>>>>>>>\n\r\n";
    _write(1, Hello, sizeof(Hello) - 1);
}