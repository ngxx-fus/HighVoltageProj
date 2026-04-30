/// @file   SerialLog.h
/// @brief  System logging module using USART2 via bare-metal _write.

#ifndef __SERIAL_LOG_H__
#define __SERIAL_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

/// @brief Initialize the logging module using USART2 on pin PA2 at 115200 baudrate.
/// @param None
/// @return None
void Log_Init(void);

/// @brief Print formatted standard log over UART with [LOG] prefix.
/// @param format Format string (printf style).
/// @param ... Variable arguments.
/// @return None
void SysLog(const char *format, ...);

/// @brief Print formatted error log over UART with [ERR] prefix.
/// @param format Format string (printf style).
/// @param ... Variable arguments.
/// @return None
void SysErr(const char *format, ...);

/// @brief Print formatted function entry log over UART with [>>>] prefix.
/// @param format Format string (printf style).
/// @param ... Variable arguments.
/// @return None
void SysEntry(const char *format, ...);

/// @brief Print formatted function exit log over UART with [<<<] prefix.
/// @param format Format string (printf style).
/// @param ... Variable arguments.
/// @return None
void SysExit(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __SERIAL_LOG_H__ */