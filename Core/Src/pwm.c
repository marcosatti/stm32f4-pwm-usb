#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <slip.h>
#include "stm32f4xx_hal.h"
#include "pwm.h"
#include "main.h"
#include "cmsis_os.h"

#define MAX_CCR_VALUE 480000
#define PACKET_BUFFER_SIZE 1024

extern TIM_HandleTypeDef htim2;
extern osMessageQueueId_t pwmQueueHandle;
static bool isr_ready = false;
static struct PwmLevels pwmLevels;
static char buffer[PACKET_BUFFER_SIZE];
static size_t buffer_read_idx = PACKET_BUFFER_SIZE - 1;
static size_t buffer_write_idx;

static void PwmGetMessage(void) {
    while (osMessageQueueGet(pwmQueueHandle, &pwmLevels, NULL, osWaitForever) == osOK) {
        return;
    }
}

static char PwmRecvChar(void) {
    return 1;
}

const static slip_config slipConfig = {
    .send_char = NULL, // Not supported.
    .recv_char = PwmRecvChar,
    .check_start = true
};

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

    // Signal ready for data.
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

void PwmIsrDataRecv(uint8_t *this_buffer, uint32_t this_length) {
    if (!isr_ready)
        return;

    // Write to buffer as much as we can.

    // Work out the length of data in the buffer still to read.

    // 

    // Construct message and queue it.
    struct PwmLevels message = { 0 };
    osMessageQueuePut(pwmQueueHandle, (void *)&message, 0, 0);
}
