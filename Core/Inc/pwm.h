#include <stdint.h>

struct PwmLevels {
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

void PwmTaskMain(void);
struct PwmLevels *GetPwmLevels(void);

void PwmIsrDataRecv(uint8_t *buf, uint32_t len);
