#include <Arduino.h>

// PB02: Control pin - pull HIGH to enable voltage reading
// PB03: ADC input (AIN11)
// Reference: INTVCC1 (VDDANA/2), Gain: DIV2 -> Full scale = 3.3V

#define ADC_FULL_SCALE 4095.0f

static void adc_init(void) {
    // Enable ADC bus clock
    PM->APBCMASK.reg |= PM_APBCMASK_ADC;

    // Feed GCLK0 to ADC
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC |
                         GCLK_CLKCTRL_GEN_GCLK0 |
                         GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY);

    // Reset ADC
    ADC->CTRLA.bit.SWRST = 1;
    while (ADC->CTRLA.bit.SWRST);

    // Reference: INTVCC1 = VDDANA / 2 (~1.65V)
    ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INTVCC1;

    // No averaging
    ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1;

    // 12-bit resolution, prescaler DIV32 (48MHz/32 = 1.5MHz)
    ADC->CTRLB.reg = ADC_CTRLB_RESSEL_12BIT |
                     ADC_CTRLB_PRESCALER_DIV32;
    while (ADC->STATUS.bit.SYNCBUSY);

    // Positive input: AIN11 (PB03), Negative: GND, Gain: DIV2
    // DIV2 gain + 1.65V ref = 3.3V full scale
    ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_PIN11 |
                         ADC_INPUTCTRL_MUXNEG_GND |
                         ADC_INPUTCTRL_GAIN_DIV2;
    while (ADC->STATUS.bit.SYNCBUSY);

    // Enable ADC
    ADC->CTRLA.bit.ENABLE = 1;
    while (ADC->STATUS.bit.SYNCBUSY);
}

static uint16_t adc_read(void) {
    ADC->SWTRIG.bit.START = 1;
    while (ADC->STATUS.bit.SYNCBUSY);
    while (!ADC->INTFLAG.bit.RESRDY);
    return ADC->RESULT.reg;
}

void setup() {
    Serial1.begin(230400);
    delay(100);

    Serial1.println("============================================");
    Serial1.println("  ADC Voltage Reader - XIAO SAMD21");
    Serial1.println("  ADC Pin: PB03 (AIN11)");
    Serial1.println("  Control: PB02 (HIGH to enable reading)");
    Serial1.println("============================================");

    // PB02: GPIO output, pull HIGH and keep
    PORT->Group[1].DIRSET.reg = PORT_PB02;
    PORT->Group[1].OUTSET.reg = PORT_PB02;
    delay(1);  // Wait for voltage to settle

    // PB03: analog input via peripheral mux
    PORT->Group[1].PINCFG[3].bit.PMUXEN = 1;   // Enable PMUX
    PORT->Group[1].PINCFG[3].bit.INEN   = 0;   // Disable digital input buffer
    PORT->Group[1].PINCFG[3].bit.PULLEN = 0;   // No pull resistor
    // PB03 is odd pin -> uses PMUX[1].PMUXO, function B = Analog
    PORT->Group[1].PMUX[1].bit.PMUXO = PORT_PMUX_PMUXO_B_Val;

    adc_init();
    Serial1.println("ADC initialized, starting readings...\n");
}

void loop() {
    uint16_t raw = adc_read();

    // Voltage = raw * (1.65V * 2) / 4095 = raw * 3.3 / 4095
    float voltage = (raw * 3.3f) / ADC_FULL_SCALE;

    Serial1.print("Raw: ");
    Serial1.print(raw);
    Serial1.print("  Voltage: ");
    Serial1.print(voltage, 3);
    Serial1.println(" V");

    delay(1000);
}
