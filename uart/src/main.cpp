#include <Arduino.h>

// D0 = PA02 (DAC output for PWM)
// D6 = PB08 (Serial1 TX)
// D7 = PB09 (Serial1 RX)

#define PWM_PIN 0  // D0 pin

void setup()
{
    Serial1.begin(230400);  // 使用 D6(TX) D7(RX) 硬件串口
    delay(100);             // 等待串口就绪
    pinMode(PWM_PIN, OUTPUT);
    Serial1.println("SAMD21 PWM (DAC) Example - D0 Pin");
}

void loop()
{
    // Fade in: 0 -> 255
    for (int duty = 0; duty <= 255; duty++) {
        analogWrite(PWM_PIN, duty);
        delay(10);
    }

    Serial1.println("Fade in done");
    delay(500);

    // Fade out: 255 -> 0
    for (int duty = 255; duty >= 0; duty--) {
        analogWrite(PWM_PIN, duty);
        delay(10);
    }

    Serial1.println("Fade out done");
    delay(500);
}
