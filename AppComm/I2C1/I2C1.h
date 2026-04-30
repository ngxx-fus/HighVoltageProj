/// @file   I2C1.h
/// @brief  Bare-metal I2C1 driver for STM32F103C8T6 (PB6=SCL, PB7=SDA).

#ifndef __I2C1_H__
#define __I2C1_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../../AppUtils/All.h"
#include "../../AppLog/SerialLog.h"

/* --- I2C CONFIGURATION ENUMS --- */
enum I2C1_CONF_ADDR_BIT {
    I2C1_ADDR_7BIT   = 7U,
    I2C1_ADDR_10BIT  = 10U
};

enum I2C1_CONF_SPEED {
    I2C1_MODE_100KHZ = 100000U,
    I2C1_MODE_400KHZ = 400000U
};

/* --- I2C STATE ENUM --- */
/// @brief I2C1 operation state tracker
typedef enum {
    eI2C1_STATE_FREE = 0,
    eI2C1_STATE_READ,
    eI2C1_STATE_WRITE
} eI2C1_State_t;

/// @brief Configure I2C1 global parameters
/// @param AddrBitType 7-bit (I2C1_ADDR_7BIT) or 10-bit (I2C1_ADDR_10BIT)
/// @param SclFreqMode Frequency mode (100kHz or 400kHz)
/// @return STAT_OKE on success
ReturnCode_t I2C1_Config(enum I2C1_CONF_ADDR_BIT AddrBitType, enum I2C1_CONF_SPEED SclFreqMode);

/// @brief Initialize I2C1 hardware (Clocks, GPIO PB6/PB7, I2C Registers)
/// @param None
/// @return STAT_OKE on success
ReturnCode_t I2C1_Init(void);

/// @brief Start an I2C transaction and send device address
/// @param DevAddr Target device address
/// @param ReadWrite 0 for Write, 1 for Read
/// @return STAT_OKE on success, or STAT_ERR_NO_RESPONSE / STAT_ERR_TIMEOUT
ReturnCode_t I2C1_StartTransaction(uint16_t DevAddr, uint8_t ReadWrite);

/// @brief Generate STOP condition to end transaction
/// @param None
/// @return STAT_OKE on success
ReturnCode_t I2C1_StopTransaction(void);

/// @brief Write a single byte to the target
/// @param ByteValue Data byte to send
/// @return Number of bytes written (1) or error code
ReturnCode_t I2C1_WriteByte(Byte_t ByteValue);

/// @brief Write a Word (2 bytes) to the target
/// @param WordValue Data word to send (MSB sent first)
/// @return Number of bytes written (2) or error code
ReturnCode_t I2C1_WriteWord(Word_t WordValue);

/// @brief Write a DWord (4 bytes) to the target
/// @param DwordValue Data dword to send (MSB sent first)
/// @return Number of bytes written (4) or error code
ReturnCode_t I2C1_WriteDword(Dword_t DwordValue);

/// @brief Write a QWord (8 bytes) to the target
/// @param QwordValue Data qword to send (MSB sent first)
/// @return Number of bytes written (8) or error code
ReturnCode_t I2C1_WriteQword(Qword_t QwordValue);

/// @brief Write an arbitrary number of bytes
/// @param pData Pointer to the transmit buffer
/// @param Length Number of bytes to send
/// @return Number of bytes written or error code
ReturnCode_t I2C1_WriteNByte(const Byte_t *pData, uint32_t Length);

/// @brief Read a single byte from the target
/// @param pByteValue Pointer to store the read byte
/// @return Number of bytes read (1) or error code
ReturnCode_t I2C1_ReadByte(Byte_t *pByteValue);

/// @brief Read a Word (2 bytes) from the target
/// @param pWordValue Pointer to store the read word (MSB received first)
/// @return Number of bytes read (2) or error code
ReturnCode_t I2C1_ReadWord(Word_t *pWordValue);

/// @brief Read a DWord (4 bytes) from the target
/// @param pDwordValue Pointer to store the read dword (MSB received first)
/// @return Number of bytes read (4) or error code
ReturnCode_t I2C1_ReadDword(Dword_t *pDwordValue);

/// @brief Read a QWord (8 bytes) from the target
/// @param pQwordValue Pointer to store the read qword (MSB received first)
/// @return Number of bytes read (8) or error code
ReturnCode_t I2C1_ReadQword(Qword_t *pQwordValue);

/// @brief Read an arbitrary number of bytes
/// @param pData Pointer to the receive buffer
/// @param Length Number of bytes to read
/// @return Number of bytes read or error code
ReturnCode_t I2C1_ReadNByte(Byte_t *pData, uint32_t Length);

#ifdef __cplusplus
}
#endif

#endif /* __I2C1_H__ */