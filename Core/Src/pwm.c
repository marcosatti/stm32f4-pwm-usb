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

static bool task_ready = false;
static bool slip_packet_continuation;
static uint8_t *slip_packet_buffer;
static uint32_t slip_packet_buffer_length;
static uint32_t slip_packet_buffer_index;
static char payload_buffer[MAX_BUFFER_SIZE];
static size_t payload_buffer_index;

static struct PwmLevels pwmLevels;

static bool PwmIsrRecvChar(char *c, void *user_data) {
    if (slip_packet_buffer_index >= slip_packet_buffer_length)
        return false;

    *c = (char)(slip_packet_buffer[slip_packet_buffer_index++]);
    return true;
}

static bool PwmIsrWriteChar(char c, void *user_data) {
    if (payload_buffer_index >= MAX_BUFFER_SIZE)
        return false;
    
    payload_buffer[payload_buffer_index++] = c;
    return true;
}

static slips_context_t slips_context = {
    .decode_recv_char_fn = PwmIsrRecvChar,
    .decode_write_char_fn = PwmIsrWriteChar,
    .check_start = true
};

void PwmIsrDataRecv(uint8_t *this_buffer, uint32_t this_length) {
    if (!task_ready)
        return;

    // Setup state.
    slip_packet_buffer = this_buffer;
    slip_packet_buffer_length = this_length;
    slip_packet_buffer_index = 0;
    
    // Decode SLIP packet.
    while (true) {
        if (slip_packet_buffer_index >= slip_packet_buffer_length) {
            // No more data to process.
            return;
        }

        if (payload_buffer_index >= MAX_BUFFER_SIZE) {
            // Payload buffer overrun, discard packet.
            payload_buffer_index = 0;
            slip_packet_continuation = false;
            return;
        }

        slips_context.check_start = !slip_packet_continuation;
        if (slips_recv_packet(&slips_context)) {
            // Now have a fully decoded packet.
            slip_packet_continuation = false;
            break;
        }
        
        // Payload packet not decoded (fully?) due to invalid SLIP data.
        slip_packet_continuation = (payload_buffer_index > 0);
    }

    // Check length of payload.
    uint16_t payload_length = *(uint16_t *)(&payload_buffer[0]);
    if (payload_length != payload_buffer_index) {
        payload_buffer_index = 0;
        return;
    }

    // Application specific: always expecting a length of 3 (+2 overhead).
    if (payload_length == 5) {
        // Queue message.
        osMessageQueuePut(pwmQueueHandle, (void *)&payload_buffer, 0, 0);
    }

    // Reset state.
    payload_buffer_index = 0;
}


/// Non ISR

static void PwmGetMessage(void) {
    while (osMessageQueueGet(pwmQueueHandle, &pwmLevels, NULL, osWaitForever) == osOK) {
        return;
    }
}

void PwmTaskMain(void) {
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
    task_ready = true;

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
