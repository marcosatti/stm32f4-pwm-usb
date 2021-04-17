#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <slips.h>
#include "stm32f4xx_hal.h"
#include "pwm.h"
#include "main.h"
#include "cmsis_os.h"

#define MAX_CCR_VALUE 480000
#define MAX_BUFFER_SIZE 1024

extern TIM_HandleTypeDef htim2;
extern osMessageQueueId_t pwmQueueHandle;
static bool isr_ready = false;
static char slip_buffer[MAX_BUFFER_SIZE];
static char raw_buffer[MAX_BUFFER_SIZE];
static size_t raw_buffer_idx;
static struct PwmLevels pwmLevels;

static bool PwmIsrRecvChar(char *c, void *user_data) {
    cirbuf_t *buf = (cirbuf_t *)user_data;
    return buf_read(buf, c);
}

static bool PwmIsrWriteChar(char c, void *user_data) {
    if (raw_buffer_idx >= MAX_BUFFER_SIZE)
        return false;
    
    raw_buffer[raw_buffer_idx++] = c;
    return true;
}

const static slips_config slipConfig = {
    .encode_send_char = NULL, // Not used.
    .encode_read_char = NULL, // Not used.
    .decode_recv_char = PwmIsrRecvChar,
    .decode_write_char = PwmIsrWriteChar,
    .send_start = true,
    .check_start = true,
    .user_data = NULL
};

void PwmIsrDataRecv(uint8_t *this_buffer, uint32_t this_length) {
    if (!isr_ready)
        return;

    // Write to buffer as much as we can.
    for (size_t i = 0; i < this_length; i++) {
        if (!buf_write(buf, this_buffer[i]))
            return;
    }

    if (!slips_recv_packet()) {
        // Failed to receive a packet, so clear the raw buffer.
        raw_buffer_idx = 0;
    }

    // Work out the length of data in the buffer still to read.

    // 

    // Construct message and queue it.
    struct PwmLevels message = { 0 };
    osMessageQueuePut(pwmQueueHandle, (void *)&message, 0, 0);
}



/// Non-ISR

static void PwmGetMessage(void) {
    while (osMessageQueueGet(pwmQueueHandle, &pwmLevels, NULL, osWaitForever) == osOK) {
        return;
    }
}

void PwmTaskMain(void) {
    // Start the SLIP library.
    slip_init(&slipConfig);

    // Start the base counter.
    HAL_TIM_Base_Start(&htim2);

    // Start the channels.
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0); 
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0); 
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0); 

    // Signal to ISR ready for processing.
    isr_ready = true;

    // Set the PWM levels.
    while (true) {
        PwmGetMessage();
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ((uint32_t)pwmLevels.R * MAX_CCR_VALUE) / 100); 
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, ((uint32_t)pwmLevels.G * MAX_CCR_VALUE) / 100); 
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ((uint32_t)pwmLevels.B * MAX_CCR_VALUE) / 100); 
    }
}

struct PwmLevels *GetPwmLevels(void) {
    return &pwmLevels;
}
