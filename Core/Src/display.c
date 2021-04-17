#include <stdio.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "display.h"
#include "pwm.h"
#include "ssd1306.h"

void DisplayTaskMain(void) {
    ssd1306_Init();

    while (true) {
        struct PwmLevels *levels = GetPwmLevels();

        ssd1306_Fill(Black);

        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("PWM Output:", Font_7x10, White);

        char buffer[16];

        sprintf(buffer, "R: %hu%%", levels->R);
        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString(buffer, Font_7x10, White);

        sprintf(buffer, "G: %hu%%", levels->G);
        ssd1306_SetCursor(0, 30);
        ssd1306_WriteString(buffer, Font_7x10, White);

        sprintf(buffer, "B: %hu%%", levels->B);
        ssd1306_SetCursor(0, 40);
        ssd1306_WriteString(buffer, Font_7x10, White);

        ssd1306_UpdateScreen();

        osDelay(100);
    }
}
