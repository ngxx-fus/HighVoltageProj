/// @brief Thư viện macro Delay đơn giản dựa trên vòng lặp for.
/// @note Chỉ chính xác tương đối, phụ thuộc vào System Clock (đang giả định 8MHz) và mức tối ưu -O0.

/// @brief Busy wait blocking delay theo đơn vị micro giây (us).
/// @param us Số micro giây cần chờ.
#define BusyWait(us) do { \
    /* Place the actual comment here */ \
    /* Với 8MHz và -O0, mỗi vòng lặp volatile tốn khoảng ~1us */ \
    uint32_t _count = (us); \
    for (volatile uint32_t _i = 0; _i < _count; _i++) { } \
} while (0)

/// @brief Busy wait blocking delay theo đơn vị mili giây (ms).
/// @param ms Số mili giây cần chờ.
#define BusyWaitMs(ms) BusyWait((ms) * 1000)