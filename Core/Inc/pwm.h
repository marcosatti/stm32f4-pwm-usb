#include <stdint.h>

/// ISR

void PwmIsrDataRecv(uint8_t *buffer, uint32_t length);


/// Non ISR

struct PwmLevels {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} __attribute__((packed));

void PwmTaskMain(void);

struct PwmLevels *GetPwmLevels(void);
