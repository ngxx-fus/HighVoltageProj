#include "I2C1.h"
#include <stddef.h>
#include <stdarg.h>
#include "../../AppLog/SerialLog.h"

#define SysLog(...)     DelayUS(10)
#define SysEntry(...)   DelayUS(10)
#define SysExit(...)    DelayUS(10)

#define GPIOB_ODR       *(volatile uint32_t *)0x40010C0C

/* --- STM32F103C8T6 REGISTERS --- */
#define RCC_APB1ENR     *(volatile uint32_t *)0x4002101C
#define RCC_APB2ENR     *(volatile uint32_t *)0x40021018
#define GPIOB_CRL       *(volatile uint32_t *)0x40010C00

#define I2C1_BASE       0x40005400
#define I2C1_CR1        *(volatile uint32_t *)(I2C1_BASE + 0x00)
#define I2C1_CR2        *(volatile uint32_t *)(I2C1_BASE + 0x04)
#define I2C1_DR         *(volatile uint32_t *)(I2C1_BASE + 0x10)
#define I2C1_SR1        *(volatile uint32_t *)(I2C1_BASE + 0x14)
#define I2C1_SR2        *(volatile uint32_t *)(I2C1_BASE + 0x18)
#define I2C1_CCR        *(volatile uint32_t *)(I2C1_BASE + 0x1C)
#define I2C1_TRISE      *(volatile uint32_t *)(I2C1_BASE + 0x20)

#define APB1_FREQ       8000000U
#define APB1_FREQ_MHZ   8U

/* --- STATIC CONFIGURATIONS & STATE --- */
static enum I2C1_CONF_ADDR_BIT  g_AddrBitType = I2C1_ADDR_7BIT;
static enum I2C1_CONF_SPEED     g_SclFreqMode = I2C1_MODE_400KHZ;
static eI2C1_State_t            I2C1_State    = eI2C1_STATE_FREE;

static uint32_t I2C1_GetSafeAddrMode(void) {
    if ((g_AddrBitType == I2C1_ADDR_7BIT) || (g_AddrBitType == I2C1_ADDR_10BIT)) {
        return (uint32_t)g_AddrBitType;
    }
    return I2C1_ADDR_7BIT;
}

static uint32_t I2C1_GetSafeSclMode(void) {
    if ((g_SclFreqMode == I2C1_MODE_100KHZ) || (g_SclFreqMode == I2C1_MODE_400KHZ)) {
        return (uint32_t)g_SclFreqMode;
    }
    return I2C1_MODE_100KHZ;
}

/// @brief Configure I2C1 global parameters
/// @param AddrBitType 7-bit or 10-bit address mode
/// @param SclFreqMode Bus speed configuration
/// @return STAT_OKE on success
ReturnCode_t I2C1_Config(enum I2C1_CONF_ADDR_BIT AddrBitType, enum I2C1_CONF_SPEED SclFreqMode) {
    SysEntry("I2C1_Config(AddrBitType: %d, SclFreqMode: %d)", AddrBitType, SclFreqMode);


    /* Validate and store global settings */
    if ((AddrBitType == I2C1_ADDR_7BIT) || (AddrBitType == I2C1_ADDR_10BIT)) {
        g_AddrBitType = AddrBitType;
    } else {
        g_AddrBitType = I2C1_ADDR_7BIT;
        SysErr("I2C1_Config(...): Invalid AddrBitType=%d, force 7-bit", AddrBitType);
    }

    if ((SclFreqMode == I2C1_MODE_100KHZ) || (SclFreqMode == I2C1_MODE_400KHZ)) {
        g_SclFreqMode = SclFreqMode;
    } else {
        g_SclFreqMode = I2C1_MODE_100KHZ;
        SysErr("I2C1_Config(...): Invalid SclFreqMode=%d, force 100kHz", SclFreqMode);
    }
    
    SysLog("I2C1_Config(...): Config saved AddrMode=%u, SclMode=%u", 
           (unsigned)g_AddrBitType, (unsigned)g_SclFreqMode);
    SysExit("I2C1_Config(...): Returns: STAT_OKE");
    return STAT_OKE;
}

/// @brief Initialize I2C1 hardware and GPIOs
/// @param None
/// @return STAT_OKE on success
ReturnCode_t I2C1_Init(void) {
    SysEntry("I2C1_Init(void)");


    /* Configure RCC and GPIO pins (PB6, PB7) for I2C1 */
    RCC_APB2ENR |= (1 << 0) | (1 << 3);
    RCC_APB1ENR |= (1 << 21);
    GPIOB_CRL &= ~(0xFFU << 24);
    GPIOB_CRL |=  (0xFFU << 24);
    GPIOB_ODR |= (1 << 6) | (1 << 7);


    /* Reset and configure I2C1 peripheral frequency */
    I2C1_CR1 |= (1 << 15);
    I2C1_CR1 &= ~(1 << 15);
    I2C1_CR2 = APB1_FREQ_MHZ;

    uint32_t safeSclMode = I2C1_GetSafeSclMode();

    if (safeSclMode == I2C1_MODE_100KHZ) {
        SysLog("I2C1_Init(...): 100KHz mode configured");
        I2C1_CCR = APB1_FREQ / (2 * safeSclMode);
        I2C1_TRISE = APB1_FREQ_MHZ + 1;
    }
    else {
        SysLog("I2C1_Init(...): 400KHz mode configured");
        I2C1_CCR = (1 << 15) | (APB1_FREQ / (3 * safeSclMode));
        I2C1_TRISE = (APB1_FREQ_MHZ * 300 / 1000) + 1;
    }

    if (safeSclMode != (uint32_t)g_SclFreqMode) {
        SysLog("I2C1_Init(...): INVALID mode configured");
        SysLog("I2C1_Init(...): DEF: 100KHz mode configured");
    }
                                                                                                                                                                                                                                                                                                                                                                                               

    /* Enable I2C peripheral */
    I2C1_CR1 |= (1 << 0);
    
    SysExit("I2C1_Init(...): Returns: STAT_OKE");
    return STAT_OKE;
}

/// @brief I2C Bus Recovery - toggle SCL 9 times to release stuck slave
/// @param None
/// @return void
static void I2C1_BusRecovery(void) {
    SysLog("I2C1_BusRecovery(...): Starting bus recovery sequence");
    
    /* Disable I2C peripheral to control SCL/SDA manually via GPIO */
    I2C1_CR1 &= ~(1 << 0);
    
    /* Toggle SCL 9 times to force any stuck slave to release data line */
    for (uint32_t i = 0; i < 9; i++) {
        GPIOB_ODR &= ~(1 << 6);  /* SCL = 0 (drive low) */
        for (volatile uint32_t delay = 0; delay < 50000; delay++);
        
        GPIOB_ODR |= (1 << 6);   /* SCL = 1 (release, pull-up will drive high) */
        for (volatile uint32_t delay = 0; delay < 50000; delay++);
    }
    
    /* Generate STOP condition: SDA=0, SCL=1, then SDA=1 */
    GPIOB_ODR &= ~(1 << 7);  /* SDA = 0 (drive low) */
    for (volatile uint32_t delay = 0; delay < 50000; delay++);
    GPIOB_ODR |= (1 << 6) | (1 << 7);  /* SCL=1, SDA=1 (release both) */
    for (volatile uint32_t delay = 0; delay < 50000; delay++);
    
    /* Soft reset I2C peripheral to clear hung state */
    I2C1_CR1 |= (1 << 15);  /* Set SWRST bit */
    for (volatile uint32_t delay = 0; delay < 10000; delay++) {
    }
    I2C1_CR1 &= ~(1 << 15); /* Clear SWRST bit */
    for (volatile uint32_t delay = 0; delay < 10000; delay++) {
    }
    
    /* Re-configure and enable I2C peripheral */
    {
        uint32_t safeSclMode = I2C1_GetSafeSclMode();
        I2C1_CR2 = APB1_FREQ_MHZ;
        if (safeSclMode == I2C1_MODE_400KHZ) {
            I2C1_CCR = (1 << 15) | (APB1_FREQ / (3 * safeSclMode));
            I2C1_TRISE = (APB1_FREQ_MHZ * 300 / 1000) + 1;
        } else {
            I2C1_CCR = APB1_FREQ / (2 * safeSclMode);
            I2C1_TRISE = APB1_FREQ_MHZ + 1;
        }
    }
    I2C1_CR1 |= (1 << 0);  /* Enable PE bit */
    
    SysLog("I2C1_BusRecovery(...): Bus recovery completed - SCL=%d, SDA=%d", 
           (GPIOB_ODR >> 6) & 1, (GPIOB_ODR >> 7) & 1);
}

/// @brief Start an I2C transaction and send device address
/// @param DevAddr Target device address
/// @param ReadWrite 1 for Read, 0 for Write
/// @return STAT_OKE on success, STAT_ERR on failure
ReturnCode_t I2C1_StartTransaction(uint16_t DevAddr, uint8_t ReadWrite) {
    SysEntry("I2C1_StartTransaction(DevAddr: 0x%02X, RW: %d)", DevAddr, ReadWrite);

    /* Recover only when bus is stuck busy to avoid unnecessary clock toggling */
    if (I2C1_SR2 & (1 << 1)) {
        SysLog("I2C1_StartTransaction(...): BUSY flag detected, run recovery");
        I2C1_BusRecovery();
        SysLog("I2C1_StartTransaction(...): After recovery - SCL=%d, SDA=%d", 
               (GPIOB_ODR >> 6) & 1, (GPIOB_ODR >> 7) & 1);
    }

    /* Generate START condition and wait for completion */
    uint32_t timeout = 0x000FFFFF;
    I2C1_CR1 |= (1 << 8);  /* Set START bit only */

    while (!(I2C1_SR1 & (1 << 0))) {
        if (--timeout == 0) {
            SysErr("I2C1_StartTransaction(...): Timeout waiting for START condition");
            SysExit("I2C1_StartTransaction(...): Returns: STAT_ERR");
            return STAT_ERR;
        }
    }
    
    SysLog("I2C1_StartTransaction(...): START condition generated");


    /* Transmit address combined with normalized Read/Write bit */
    if (I2C1_GetSafeAddrMode() == I2C1_ADDR_7BIT) {
        I2C1_DR = (DevAddr & 0xFE) | (ReadWrite & 1); 
    }
    else {
        I2C1_DR = 0xF0 | ((DevAddr >> 7) & 0x06) | (ReadWrite & 1);
    }
    SysLog("I2C1_StartTransaction(...): Sent DR=0x%02X (AddrMode=%u)", 
           (unsigned)I2C1_DR, (unsigned)I2C1_GetSafeAddrMode());


    /* Wait for ADDR flag, handle NACK and Timeout */
    timeout = 0x000FFFFF;
    while (!(I2C1_SR1 & (1 << 1))) {
        if (I2C1_SR1 & (1 << 10)) {
            I2C1_SR1 &= ~(1 << 10);
            I2C1_StopTransaction();
            SysErr("I2C1_StartTransaction(...): NACK received (AF flag set)");
            SysLog("I2C1_StartTransaction(...): SR1=0x%04X on NACK", I2C1_SR1);
            SysExit("I2C1_StartTransaction(...): Returns: STAT_ERR");
            return STAT_ERR;
        }
        if (--timeout == 0) {
            I2C1_StopTransaction(); 
            SysLog("I2C1_StartTransaction(...): SR1=0x%04X before timeout", I2C1_SR1);
            SysErr("I2C1_StartTransaction(...): Timeout waiting for ADDR flag");
            SysExit("I2C1_StartTransaction(...): Returns: STAT_ERR");
            return STAT_ERR;
        }
    }
    
    SysLog("I2C1_StartTransaction(...): Address matched and ACKed");

    /* Clear ADDR flag by reading SR1 and SR2, then set state */
    (void)I2C1_SR1;
    (void)I2C1_SR2;
    
    /* Set ACK bit based on read/write mode */
    if (ReadWrite == 0) {
        I2C1_CR1 &= ~(1 << 10);  /* WriteMode: ACK LOW */
        SysLog("I2C1_StartTransaction(...): WriteMode - ACK set LOW");
    } else {
        I2C1_CR1 |= (1 << 10);   /* ReadMode: ACK HIGH */
        SysLog("I2C1_StartTransaction(...): ReadMode - ACK set HIGH");
    }
    
    I2C1_State = (ReadWrite == 1) ? eI2C1_STATE_READ : eI2C1_STATE_WRITE;
    
    SysExit("I2C1_StartTransaction(...): Returns: STAT_OKE");
    return STAT_OKE;
}

/// @brief Generate STOP condition to end transaction
/// @param None
/// @return STAT_OKE on success
ReturnCode_t I2C1_StopTransaction(void) {
    SysEntry("I2C1_StopTransaction(void)");


    /* Assert STOP flag and free the state */
    I2C1_CR1 |= (1 << 9);
    I2C1_State = eI2C1_STATE_FREE;
    
    SysLog("I2C1_StopTransaction(...): STOP condition generated");
    SysExit("I2C1_StopTransaction(...): Returns: STAT_OKE");
    return STAT_OKE;
}

ReturnCode_t I2C1_WriteNByte(const Byte_t *pData, uint32_t Length) {
    SysEntry("I2C1_WriteNByte(Length: %lu)", Length);

    /* Verify if we are in the correct state and the buffer is valid */
    if (I2C1_State != eI2C1_STATE_WRITE) {
        SysErr("I2C1_WriteNByte(...): Invalid state for WRITE");
        SysExit("I2C1_WriteNByte(...): Returns: STAT_ERR");
        return STAT_ERR;
    }

    /* Iterate through the buffer and send each byte */
    for (uint32_t i = 0; i < Length; i++) {
        uint32_t timeout = 0x000FFFFF;

        /* Wait for TXE (Transmit data register empty) before writing */
        while (!(I2C1_SR1 & (1 << 7))) {
            if (I2C1_SR1 & (1 << 10)) { /* Check for NACK */
                I2C1_StopTransaction();
                SysErr("I2C1_WriteNByte(...): NACK at byte %lu", i);
                return STAT_ERR;
            }
            if (--timeout == 0) {
                I2C1_StopTransaction();
                SysErr("I2C1_WriteNByte(...): Timeout TXE at byte %lu. SR1=0x%04X", i, I2C1_SR1);
                return STAT_ERR;
            }
        }

        I2C1_DR = pData[i];
    
        /* Optional: Add a very small delay for testboard stability */
        for (volatile uint32_t d = 0; d < 100; d++); 
    }

    /* Final wait for BTF (Byte Transfer Finished) to ensure last byte is on the wire */
    uint32_t timeout = 0x000FFFFF;
    while (!(I2C1_SR1 & (1 << 2))) {
        if (--timeout == 0) {
            I2C1_StopTransaction();
            SysErr("I2C1_WriteNByte(...): Timeout BTF");
            return STAT_ERR;
        }
    }
    
    SysLog("I2C1_WriteNByte(...): Success");
    SysExit("I2C1_WriteNByte(...): Returns: STAT_OKE");
    return STAT_OKE;
}

/// @brief Write a single byte to the target
/// @param ByteValue Data byte to send
/// @return STAT_OKE on success, STAT_ERR on failure
ReturnCode_t I2C1_WriteByte(Byte_t ByteValue) {
    SysEntry("I2C1_WriteByte(ByteValue: 0x%02X)", ByteValue);


    /* Wrapper for single byte write */
    ReturnCode_t stat = I2C1_WriteNByte(&ByteValue, 1);
    
    SysExit("I2C1_WriteByte(...): Returns status");
    return stat;
}

/// @brief Write a Word (2 bytes) to the target
/// @param WordValue Data word to send (MSB sent first)
/// @return STAT_OKE on success, STAT_ERR on failure
ReturnCode_t I2C1_WriteWord(Word_t WordValue) {
    SysEntry("I2C1_WriteWord(WordValue: 0x%04X)", WordValue);


    /* Format Word to MSB-first array and send */
    Byte_t data[2];
    data[0] = (Byte_t)(WordValue >> 8);
    data[1] = (Byte_t)(WordValue & 0xFF);
    ReturnCode_t stat = I2C1_WriteNByte(data, 2);
    
    SysExit("I2C1_WriteWord(...): Returns status");
    return stat;
}

/// @brief Write a DWord (4 bytes) to the target
/// @param DwordValue Data dword to send (MSB sent first)
/// @return STAT_OKE on success, STAT_ERR on failure
ReturnCode_t I2C1_WriteDword(Dword_t DwordValue) {
    SysEntry("I2C1_WriteDword(DwordValue: 0x%08lX)", DwordValue);


    /* Format DWord to MSB-first array and send */
    Byte_t data[4];
    data[0] = (Byte_t)(DwordValue >> 24);
    data[1] = (Byte_t)(DwordValue >> 16);
    data[2] = (Byte_t)(DwordValue >> 8);
    data[3] = (Byte_t)(DwordValue & 0xFF);
    ReturnCode_t stat = I2C1_WriteNByte(data, 4);
    
    SysExit("I2C1_WriteDword(...): Returns status");
    return stat;
}

/// @brief Write a QWord (8 bytes) to the target
/// @param QwordValue Data qword to send (MSB sent first)
/// @return STAT_OKE on success, STAT_ERR on failure
ReturnCode_t I2C1_WriteQword(Qword_t QwordValue) {
    SysEntry("I2C1_WriteQword(QwordValue: 0x%016llX)", QwordValue);


    /* Format QWord to MSB-first array and send */
    Byte_t data[8];
    data[0] = (Byte_t)(QwordValue >> 56);
    data[1] = (Byte_t)(QwordValue >> 48);
    data[2] = (Byte_t)(QwordValue >> 40);
    data[3] = (Byte_t)(QwordValue >> 32);
    data[4] = (Byte_t)(QwordValue >> 24);
    data[5] = (Byte_t)(QwordValue >> 16);
    data[6] = (Byte_t)(QwordValue >> 8);
    data[7] = (Byte_t)(QwordValue & 0xFF);
    ReturnCode_t stat = I2C1_WriteNByte(data, 8);
    
    SysExit("I2C1_WriteQword(...): Returns status");
    return stat;
}

/// @brief Read an arbitrary number of bytes
/// @param pData Pointer to the receive buffer
/// @param Length Number of bytes to read
/// @return STAT_OKE on success, STAT_ERR on failure
ReturnCode_t I2C1_ReadNByte(Byte_t *pData, uint32_t Length) {
    SysEntry("I2C1_ReadNByte(Length: %lu)", Length);


    /* Verify read state and parameter validity */
    if (I2C1_State != eI2C1_STATE_READ) {
        SysErr("I2C1_ReadNByte(...): Invalid state for READ");
        SysExit("I2C1_ReadNByte(...): Returns: STAT_ERR");
        return STAT_ERR;
    }
    if (pData == NULL || Length == 0) {
        SysErr("I2C1_ReadNByte(...): Null pointer or zero length");
        SysExit("I2C1_ReadNByte(...): Returns: STAT_ERR");
        return STAT_ERR;
    }


    /* Sequentially receive all bytes and handle ACK mapping */
    for (uint32_t i = 0; i < Length; i++) {
        if (i == Length - 1) {
            I2C1_CR1 &= ~(1 << 10);
        }
        while (!(I2C1_SR1 & (1 << 6))) { }
        pData[i] = I2C1_DR;
    }

    SysLog("I2C1_ReadNByte(...): %lu bytes read successfully", Length);
    SysExit("I2C1_ReadNByte(...): Returns: STAT_OKE");
    return STAT_OKE;
}

/// @brief Read a single byte from the target
/// @param pByteValue Pointer to store the read byte
/// @return STAT_OKE on success, STAT_ERR on failure
ReturnCode_t I2C1_ReadByte(Byte_t *pByteValue) {
    SysEntry("I2C1_ReadByte(pByteValue: %p)", (void*)pByteValue);


    /* Wrapper for single byte read */
    ReturnCode_t stat = I2C1_ReadNByte(pByteValue, 1);
    
    SysExit("I2C1_ReadByte(...): Returns status");
    return stat;
}

/// @brief Read a Word (2 bytes) from the target
/// @param pWordValue Pointer to store the read word (MSB received first)
/// @return STAT_OKE on success, STAT_ERR on failure
ReturnCode_t I2C1_ReadWord(Word_t *pWordValue) {
    SysEntry("I2C1_ReadWord(pWordValue: %p)", (void*)pWordValue);


    /* Receive 2 bytes and assemble Word */
    Byte_t data[2] = {0};
    ReturnCode_t stat = I2C1_ReadNByte(data, 2);
    
    if (stat == STAT_OKE && pWordValue != NULL) {
        *pWordValue = (Word_t)((data[0] << 8) | data[1]);
    }
    
    SysExit("I2C1_ReadWord(...): Returns status");
    return stat;
}

/// @brief Read a DWord (4 bytes) from the target
/// @param pDwordValue Pointer to store the read dword (MSB received first)
/// @return STAT_OKE on success, STAT_ERR on failure
ReturnCode_t I2C1_ReadDword(Dword_t *pDwordValue) {
    SysEntry("I2C1_ReadDword(pDwordValue: %p)", (void*)pDwordValue);


    /* Receive 4 bytes and assemble DWord */
    Byte_t data[4] = {0};
    ReturnCode_t stat = I2C1_ReadNByte(data, 4);
    
    if (stat == STAT_OKE && pDwordValue != NULL) {
        *pDwordValue = ((Dword_t)data[0] << 24) | 
                       ((Dword_t)data[1] << 16) | 
                       ((Dword_t)data[2] << 8)  | 
                       ((Dword_t)data[3]);
    }
    
    SysExit("I2C1_ReadDword(...): Returns status");
    return stat;
}

/// @brief Read a QWord (8 bytes) from the target
/// @param pQwordValue Pointer to store the read qword (MSB received first)
/// @return STAT_OKE on success, STAT_ERR on failure
ReturnCode_t I2C1_ReadQword(Qword_t *pQwordValue) {
    SysEntry("I2C1_ReadQword(pQwordValue: %p)", (void*)pQwordValue);


    /* Receive 8 bytes and assemble QWord */
    Byte_t data[8] = {0};
    ReturnCode_t stat = I2C1_ReadNByte(data, 8);
    
    if (stat == STAT_OKE && pQwordValue != NULL) {
        *pQwordValue = ((Qword_t)data[0] << 56) | 
                       ((Qword_t)data[1] << 48) | 
                       ((Qword_t)data[2] << 40) | 
                       ((Qword_t)data[3] << 32) | 
                       ((Qword_t)data[4] << 24) | 
                       ((Qword_t)data[5] << 16) | 
                       ((Qword_t)data[6] << 8)  | 
                       ((Qword_t)data[7]);
    }
    
    SysExit("I2C1_ReadQword(...): Returns status");
    return stat;
}