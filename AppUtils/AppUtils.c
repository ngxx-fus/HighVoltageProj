#include "FlagControl.h"
#include "Arithmetic.h"
#include "ReturnType.h"

/// @brief Convert return status to string
/// @param ret The return code
/// @return String representation
const char* DefaultReturnType2Str(DefaultRet_t ret) {

    /* Map error codes to string equivalents */
    switch (ret) {
        case STAT_OKE:                  return STR_STAT_OKE;
        case STAT_ERR:                  return STR_STAT_ERR;
        case STAT_ERR_NULL:             return STR_STAT_ERR_NULL;
        case STAT_ERR_MALLOC_FAILED:    return STR_STAT_ERR_MALLOC_FAILED;
        case STAT_ERR_TIMEOUT:          return STR_STAT_ERR_TIMEOUT;
        case STAT_ERR_BUSY:             return STR_STAT_ERR_BUSY;
        case STAT_ERR_INVALID_ARG:      return STR_STAT_ERR_INVALID_ARG;
        case STAT_ERR_INVALID_STATE:    return STR_STAT_ERR_INVALID_STATE;
        case STAT_ERR_INVALID_SIZE:     return STR_STAT_ERR_INVALID_SIZE;
        case STAT_ERR_OVERFLOW:         return STR_STAT_ERR_OVERFLOW;
        case STAT_ERR_UNDERFLOW:        return STR_STAT_ERR_UNDERFLOW;
        case STAT_ERR_NOT_FOUND:        return STR_STAT_ERR_NOT_FOUND;
        case STAT_ERR_ALREADY_EXISTS:   return STR_STAT_ERR_ALREADY_EXISTS;
        case STAT_ERR_NOT_IMPLEMENTED:  return STR_STAT_ERR_NOT_IMPLEMENTED;
        case STAT_ERR_UNSUPPORTED:      return STR_STAT_ERR_UNSUPPORTED;
        case STAT_ERR_IO:               return STR_STAT_ERR_IO;
        case STAT_ERR_PERMISSION:       return STR_STAT_ERR_PERMISSION;
        case STAT_ERR_CRC:              return STR_STAT_ERR_CRC;
        case STAT_ERR_INIT_FAILED:      return STR_STAT_ERR_INIT_FAILED;
        case STAT_ERR_PSRAM_FAILED:     return STR_STAT_ERR_PSRAM_FAILED;
        default:                        return STR_STAT_UNKNOWN;
    }
}

/// @brief Generate pseudo-random 32-bit number
/// @param SeedInput Initialization seed or negative to continue sequence
/// @return 32-bit random integer
uint32_t GenerateRandomNumber(uint32_t SeedInput) {

    /* Generate chaotic noise based on previous state and primes */
    static uint32_t last_known_random_state = 104729u;
    static int last_known_seed_input = 0;
    uint32_t x;

    if (last_known_seed_input == SeedInput) {
        x = last_known_random_state;
    }
    else {
        x = (uint32_t)SeedInput ^ 0xA5A5A5A5u;
    }
    
    last_known_seed_input = SeedInput;

    const uint32_t P1 = 7919u, P2 = 104729u, P3 = 65537u, P4 = 439u, P5 = 1223u;

    x = (x * P1 + P3) ^ (x >> 11);
    x = (x << 7) - (x >> 5) + P4;
    x ^= (x * P2);
    x = (x ^ (x >> 17)) + (x << 13);

    uint32_t dx = x - ((x >> 1) | (x << 31));
    uint32_t ddx = dx ^ (dx >> 3) ^ (dx << 5);

    x ^= (ddx * P5);
    x ^= (dx << 9) | (dx >> 23);

    last_known_random_state = x ^ (x >> 16) ^ (x << 7) ^ P2;
    return last_known_random_state;
}

/// @brief Atomically set a flag bit
/// @param f Pointer to flag
/// @param i Bit index
/// @return None
void SafeFlagSet(SafeFlag_t *f, uint8_t i) {

    /* Fetch and logical OR */
    atomic_fetch_or(f, Mask32(i));
}

/// @brief Atomically clear a flag bit
/// @param f Pointer to flag
/// @param i Bit index
/// @return None
void SafeFlagClear(SafeFlag_t *f, uint8_t i) {

    /* Fetch and logical AND */
    atomic_fetch_and(f, InvMask32(i));
}

/// @brief Atomically toggle a flag bit
/// @param f Pointer to flag
/// @param i Bit index
/// @return None
void SafeFlagToggle(SafeFlag_t *f, uint8_t i) {

    /* Fetch and logical XOR */
    atomic_fetch_xor(f, Mask32(i));
}

/// @brief Check if flag bit is set
/// @param f Pointer to flag
/// @param i Bit index
/// @return 1 if set, 0 otherwise
ReturnCode_t SafeFlagHas(SafeFlag_t *f, uint8_t i) {

    /* Atomic load and bit mask check */
    uint32_t val = atomic_load(f);
    return (val & Mask32(i)) ? 1 : 0;
}