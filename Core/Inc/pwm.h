#include <stdint.h>

/// ISR

void pwm_isr_data_recv(uint8_t *buffer, uint32_t length);


/// Non ISR

struct pwm_levels {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} __attribute__((packed));

void pwm_task_main(void);

struct pwm_levels *pwm_get_levels(void);
