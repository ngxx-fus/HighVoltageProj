/// @brief Simple for-loop based delay macro library.
/// @note Relative accuracy, depends on System Clock (assumed 8MHz) and -O0 optimization.

/// @brief Busy wait blocking delay in microseconds (us).
/// @param us Number of microseconds to wait.
#define BusyWait(us) do { \
    /* At 8MHz and -O0, each volatile loop takes ~1us */ \
    uint32_t _count = (us); \
    for (volatile uint32_t _i = 0; _i < _count; _i++) { } \
} while (0)

/// @brief Busy wait blocking delay in milliseconds (ms).
/// @param ms Number of milliseconds to wait.
#define BusyWaitMs(ms) BusyWait((ms) * 1000)

/// @brief Busy wait blocking delay in microseconds (us).
/// @param us Number of microseconds to wait.
#define DelayUS(us) do { \
    /* At 8MHz and -O0, each volatile loop takes ~1us */ \
    uint32_t _count = (us); \
    for (volatile uint32_t _i = 0; _i < _count; _i++) { } \
} while (0)

/// @brief Busy wait blocking delay in milliseconds (ms).
/// @param ms Number of milliseconds to wait.
#define DelayMS(ms) BusyWait((ms) * 1000)