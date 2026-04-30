/// @file   ReturnType.h
/// @brief  Defines standardized return types and status enums.

#ifndef __RETURN_TYPE_H__
#define __RETURN_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

/// Default return type definition
typedef int32_t         DefaultRet_t;
typedef DefaultRet_t    ReturnCode_t;

#define STR_STAT_OKE                    "STAT_OKE"
#define STR_STAT_ERR                    "STAT_ERR"
#define STR_STAT_ERR_NULL               "STAT_ERR_NULL"
#define STR_STAT_ERR_MALLOC_FAILED      "STAT_ERR_MALLOC_FAILED"
#define STR_STAT_ERR_TIMEOUT            "STAT_ERR_TIMEOUT"
#define STR_STAT_ERR_BUSY               "STAT_ERR_BUSY"
#define STR_STAT_ERR_INVALID_ARG        "STAT_ERR_INVALID_ARG"
#define STR_STAT_ERR_INVALID_STATE      "STAT_ERR_INVALID_STATE"
#define STR_STAT_ERR_INVALID_SIZE       "STAT_ERR_INVALID_SIZE"
#define STR_STAT_ERR_OVERFLOW           "STAT_ERR_OVERFLOW"
#define STR_STAT_ERR_UNDERFLOW          "STAT_ERR_UNDERFLOW"
#define STR_STAT_ERR_NOT_FOUND          "STAT_ERR_NOT_FOUND"
#define STR_STAT_ERR_ALREADY_EXISTS     "STAT_ERR_ALREADY_EXISTS"
#define STR_STAT_ERR_NOT_IMPLEMENTED    "STAT_ERR_NOT_IMPLEMENTED"
#define STR_STAT_ERR_UNSUPPORTED        "STAT_ERR_UNSUPPORTED"
#define STR_STAT_ERR_IO                 "STAT_ERR_IO"
#define STR_STAT_ERR_PERMISSION         "STAT_ERR_PERMISSION"
#define STR_STAT_ERR_CRC                "STAT_ERR_CRC"
#define STR_STAT_ERR_INIT_FAILED        "STAT_ERR_INIT_FAILED"
#define STR_STAT_ERR_PSRAM_FAILED       "STAT_ERR_PSRAM_FAILED"
#define STR_STAT_UNKNOWN                "STAT_UNKNOWN"

typedef enum {
    STAT_OKE                     = 0,   ///< Success
    STAT_ERR                     = -1,  ///< Generic error
    STAT_ERR_NULL                = -2,  ///< Null pointer passed
    STAT_ERR_MALLOC_FAILED       = -3,  ///< Memory allocation failed
    STAT_ERR_TIMEOUT             = -4,  ///< Timeout occurred
    STAT_ERR_BUSY                = -5,  ///< Resource/device is busy
    STAT_ERR_INVALID_ARG         = -6,  ///< Invalid argument
    STAT_ERR_INVALID_STATE       = -7,  ///< Invalid state
    STAT_ERR_INVALID_SIZE        = -8,  ///< Invalid size provided
    STAT_ERR_OVERFLOW            = -9,  ///< Buffer/variable overflow
    STAT_ERR_UNDERFLOW           = -10, ///< Buffer/variable underflow
    STAT_ERR_NOT_FOUND           = -11, ///< Resource not found
    STAT_ERR_ALREADY_EXISTS      = -12, ///< Resource already exists
    STAT_ERR_NOT_IMPLEMENTED     = -13, ///< Functionality not implemented
    STAT_ERR_UNSUPPORTED         = -14, ///< Unsupported operation
    STAT_ERR_IO                  = -15, ///< Input/Output error
    STAT_ERR_PERMISSION          = -16, ///< Permission denied
    STAT_ERR_CRC                 = -17, ///< CRC check failed
    STAT_ERR_INIT_FAILED         = -18, ///< Initialization failed
    STAT_ERR_PSRAM_FAILED        = -19, ///< External RAM failure
    STAT_ERR_NO_RESPONSE         = -20, ///< External IO not responding
    STAT_ERR_OUT_OF_RANGE        = -21  ///< Out of range
} DefaultRet_e;

/// @brief Get string representation of status
/// @param ret Status code
/// @return String description
const char* DefaultReturnType2Str(DefaultRet_t ret);

#ifdef __cplusplus
}
#endif

#endif /* __RETURN_TYPE_H__ */