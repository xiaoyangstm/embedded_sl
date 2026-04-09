#include <Arduino.h>
#include <Wire.h>

// D4 = PA08 (SDA), D5 = PA09 (SCL) — default I2C pins on XIAO SAMD21

// BME280 I2C Address
#define BME280_ADDR 0x76

// BME280 Register addresses
#define BME280_REG_ID        0xD0
#define BME280_REG_RESET     0xE0
#define BME280_REG_CTRL_HUM  0xF2
#define BME280_REG_STATUS    0xF3
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG    0xF5

// BME280 Chip ID
#define BME280_CHIP_ID 0x60

// Calibration data
struct bme280_calib_data {
    uint16_t dig_t1;
    int16_t  dig_t2;
    int16_t  dig_t3;
    uint16_t dig_p1;
    int16_t  dig_p2;
    int16_t  dig_p3;
    int16_t  dig_p4;
    int16_t  dig_p5;
    int16_t  dig_p6;
    int16_t  dig_p7;
    int16_t  dig_p8;
    int16_t  dig_p9;
    uint8_t  dig_h1;
    int16_t  dig_h2;
    uint8_t  dig_h3;
    int16_t  dig_h4;
    int16_t  dig_h5;
    int8_t   dig_h6;
};

static struct bme280_calib_data calib;

// I2C register read
static uint8_t i2c_read_reg(uint8_t reg) {
    Wire.beginTransmission(BME280_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)BME280_ADDR, (uint8_t)1);
    return Wire.read();
}

// I2C register write
static void i2c_write_reg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(BME280_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

// I2C burst read
static void i2c_read_regs(uint8_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(BME280_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)BME280_ADDR, len);
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = Wire.read();
    }
}

// Check BME280 presence
static bool bme280_check(void) {
    uint8_t id = i2c_read_reg(BME280_REG_ID);
    if (id == BME280_CHIP_ID) {
        Serial1.print("BME280 found, Chip ID: 0x");
        Serial1.println(id, HEX);
        return true;
    }
    Serial1.print("BME280 NOT found, got ID: 0x");
    Serial1.println(id, HEX);
    return false;
}

// Read calibration data
static void bme280_read_calibration(void) {
    uint8_t calib_tp[24];
    uint8_t calib_h[7];

    i2c_read_regs(0x88, calib_tp, 24);

    calib.dig_t1 = (uint16_t)((calib_tp[1] << 8) | calib_tp[0]);
    calib.dig_t2 = (int16_t)((calib_tp[3] << 8) | calib_tp[2]);
    calib.dig_t3 = (int16_t)((calib_tp[5] << 8) | calib_tp[4]);

    calib.dig_p1 = (uint16_t)((calib_tp[7] << 8) | calib_tp[6]);
    calib.dig_p2 = (int16_t)((calib_tp[9] << 8) | calib_tp[8]);
    calib.dig_p3 = (int16_t)((calib_tp[11] << 8) | calib_tp[10]);
    calib.dig_p4 = (int16_t)((calib_tp[13] << 8) | calib_tp[12]);
    calib.dig_p5 = (int16_t)((calib_tp[15] << 8) | calib_tp[14]);
    calib.dig_p6 = (int16_t)((calib_tp[17] << 8) | calib_tp[16]);
    calib.dig_p7 = (int16_t)((calib_tp[19] << 8) | calib_tp[18]);
    calib.dig_p8 = (int16_t)((calib_tp[21] << 8) | calib_tp[20]);
    calib.dig_p9 = (int16_t)((calib_tp[23] << 8) | calib_tp[22]);

    uint8_t h1 = i2c_read_reg(0xA1);
    calib.dig_h1 = h1;

    i2c_read_regs(0xE1, calib_h, 7);

    calib.dig_h2 = (int16_t)((calib_h[1] << 8) | calib_h[0]);
    calib.dig_h3 = calib_h[2];
    calib.dig_h4 = (int16_t)((int16_t)((int8_t)calib_h[3]) * 16 + (calib_h[4] & 0x0F));
    calib.dig_h5 = (int16_t)(((int16_t)((int8_t)calib_h[5]) * 16) | ((calib_h[4] >> 4) & 0x0F));
    calib.dig_h6 = (int8_t)calib_h[6];

    Serial1.println("Calibration data loaded");
}

// Initialize BME280
static bool bme280_init(void) {
    Serial1.println("Initializing BME280...");

    // Soft reset
    i2c_write_reg(BME280_REG_RESET, 0xB6);
    delay(10);

    // Wait for NVM copy
    uint8_t status;
    do {
        status = i2c_read_reg(BME280_REG_STATUS);
    } while (status & 0x01);

    // Read calibration
    bme280_read_calibration();

    // Humidity oversampling x1
    i2c_write_reg(BME280_REG_CTRL_HUM, 0x01);
    // Normal mode, temp x1, pressure x1
    i2c_write_reg(BME280_REG_CTRL_MEAS, 0x27);
    // Standby 0.5ms, filter off
    i2c_write_reg(BME280_REG_CONFIG, 0x00);

    Serial1.println("BME280 initialized OK");
    return true;
}

// Temperature compensation, returns t_fine
static int32_t compensate_temp(int32_t adc_temp) {
    int32_t var1, var2;
    var1 = ((((adc_temp >> 3) - ((int32_t)calib.dig_t1 << 1))) *
            ((int32_t)calib.dig_t2)) >> 11;
    var2 = (((((adc_temp >> 4) - ((int32_t)calib.dig_t1)) *
              ((adc_temp >> 4) - ((int32_t)calib.dig_t1))) >> 12) *
            ((int32_t)calib.dig_t3)) >> 14;
    return var1 + var2;
}

// Pressure compensation
static uint32_t compensate_press(int32_t adc_press, int32_t t_fine) {
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_p6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_p5) << 17);
    var2 = var2 + (((int64_t)calib.dig_p4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_p3) >> 8) +
           ((var1 * (int64_t)calib.dig_p2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_p1) >> 33;

    if (var1 == 0) return 0;

    p = 1048576 - (int64_t)adc_press;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_p9) * ((int64_t)(((p >> 13) * (p >> 13)) >> 13))) >> 25;
    var2 = (((int64_t)calib.dig_p8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_p7) << 4);
    return (uint32_t)p;
}

// Humidity compensation
static uint32_t compensate_humidity(int32_t adc_hum, int32_t t_fine) {
    int32_t v_x1 = t_fine - ((int32_t)76800);
    v_x1 = ((((adc_hum << 14) - ((int32_t)calib.dig_h4 << 20) -
              ((int32_t)calib.dig_h5 * v_x1)) + ((int32_t)16384)) >> 15) *
             (((((((v_x1 * ((int32_t)calib.dig_h6)) >> 10) *
                  (((v_x1 * ((int32_t)calib.dig_h3)) >> 11) +
                   ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
               ((int32_t)calib.dig_h2) + 8192) >> 14);
    v_x1 = v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) *
                     ((int32_t)calib.dig_h1)) >> 4);
    v_x1 = (v_x1 < 0) ? 0 : v_x1;
    v_x1 = (v_x1 > 419430400) ? 419430400 : v_x1;
    return (uint32_t)(v_x1 >> 12);
}

// Read and print sensor data
static void read_sensor(void) {
    uint8_t data[8];
    i2c_read_regs(0xF7, data, 8);

    int32_t adc_press = (int32_t)(((uint32_t)data[0] << 12) |
                                  ((uint32_t)data[1] << 4) |
                                  ((uint32_t)data[2] >> 4));
    int32_t adc_temp = (int32_t)(((uint32_t)data[3] << 12) |
                                 ((uint32_t)data[4] << 4) |
                                 ((uint32_t)data[5] >> 4));
    int32_t adc_hum = (int32_t)(((uint16_t)data[6] << 8) | (uint16_t)data[7]);

    int32_t t_fine = compensate_temp(adc_temp);
    int32_t temp = (t_fine * 5 + 128) >> 8;
    uint32_t press = compensate_press(adc_press, t_fine);
    uint32_t hum = compensate_humidity(adc_hum, t_fine);

    Serial1.print("Temp: ");
    Serial1.print(temp / 100.0f, 2);
    Serial1.print(" C, Press: ");
    Serial1.print(press / 256.0f / 100.0f, 2);
    Serial1.print(" hPa, Hum: ");
    Serial1.print(hum / 1000.0f, 2);
    Serial1.println(" %");
}

// ============================================================
//  I2C Speed Test
// ============================================================

// Test a single clock frequency: perform N reads and report stats.
// Returns true if all reads succeeded (got expected chip ID).
static bool speed_test_at(uint32_t clock_hz, uint16_t iterations) {
    Wire.setClock(clock_hz);

    // Verify sensor is still responding
    uint8_t id = i2c_read_reg(BME280_REG_ID);
    if (id != BME280_CHIP_ID) {
        Serial1.print("  FAIL: no response (got 0x");
        Serial1.print(id, HEX);
        Serial1.println("), abort this freq");
        return false;
    }

    // --- Write test: write ctrl_meas register repeatedly ---
    uint32_t t0 = micros();
    for (uint16_t i = 0; i < iterations; i++) {
        i2c_write_reg(BME280_REG_CTRL_MEAS, 0x27);
    }
    uint32_t write_us = micros() - t0;

    // --- Read test: burst-read 8 bytes (sensor data) repeatedly ---
    uint8_t buf[8];
    t0 = micros();
    for (uint16_t i = 0; i < iterations; i++) {
        i2c_read_regs(0xF7, buf, 8);
    }
    uint32_t read8_us = micros() - t0;

    // --- Single-byte read test ---
    t0 = micros();
    for (uint16_t i = 0; i < iterations; i++) {
        i2c_read_reg(BME280_REG_ID);
    }
    uint32_t read1_us = micros() - t0;

    // Calculate effective throughput
    // Write: 3 bytes per transaction (addr+reg+val) + overhead
    // Read8: write addr+reg (2 bytes), then read 8 bytes = 10 bytes total
    // Read1: write addr+reg (2 bytes), then read 1 byte = 3 bytes total
    float write_bytes = (float)iterations * 3.0f;
    float read8_bytes = (float)iterations * 10.0f;
    float read1_bytes = (float)iterations * 3.0f;

    float write_kbps = write_bytes / ((float)write_us / 1000000.0f) / 1000.0f;
    float read8_kbps = read8_bytes / ((float)read8_us / 1000000.0f) / 1000.0f;
    float read1_kbps = read1_bytes / ((float)read1_us / 1000000.0f) / 1000.0f;

    // Per-transaction latency
    float write_lat_us = (float)write_us / (float)iterations;
    float read8_lat_us = (float)read8_us / (float)iterations;
    float read1_lat_us = (float)read1_us / (float)iterations;

    Serial1.print("  Write 3B x");
    Serial1.print(iterations);
    Serial1.print(": ");
    Serial1.print(write_us);
    Serial1.print(" us  (");
    Serial1.print(write_lat_us, 1);
    Serial1.print(" us/txn, ");
    Serial1.print(write_kbps, 1);
    Serial1.println(" KB/s)");

    Serial1.print("  Read  8B x");
    Serial1.print(iterations);
    Serial1.print(": ");
    Serial1.print(read8_us);
    Serial1.print(" us  (");
    Serial1.print(read8_lat_us, 1);
    Serial1.print(" us/txn, ");
    Serial1.print(read8_kbps, 1);
    Serial1.println(" KB/s)");

    Serial1.print("  Read  1B x");
    Serial1.print(iterations);
    Serial1.print(": ");
    Serial1.print(read1_us);
    Serial1.print(" us  (");
    Serial1.print(read1_lat_us, 1);
    Serial1.print(" us/txn, ");
    Serial1.print(read1_kbps, 1);
    Serial1.println(" KB/s)");

    // Verify sensor still OK after stress test
    id = i2c_read_reg(BME280_REG_ID);
    return (id == BME280_CHIP_ID);
}

static void run_speed_test(void) {
    // Clock frequencies to test (Hz)
    const uint32_t freqs[] = {
        100000,   // 100 kHz  Standard
        200000,   // 200 kHz
        400000,   // 400 kHz  Fast mode
        600000,   // 600 kHz
        800000,   // 800 kHz
        1000000,  // 1 MHz    Fast-mode+
        1200000,  // 1.2 MHz
        1500000,  // 1.5 MHz
        1700000,  // 1.7 MHz
        2000000,  // 2.0 MHz  (beyond HS mode, likely unstable)
    };
    const uint8_t num_freqs = sizeof(freqs) / sizeof(freqs[0]);
    const uint16_t iterations = 200;

    Serial1.println();
    Serial1.println("============================================");
    Serial1.println("  I2C Speed Test Start");
    Serial1.print  ("  Iterations per test: ");
    Serial1.println(iterations);
    Serial1.println("============================================");

    for (uint8_t f = 0; f < num_freqs; f++) {
        Serial1.println();
        Serial1.print("[ ");
        Serial1.print(freqs[f] / 1000);
        Serial1.print(" kHz ]  (");
        Serial1.print(freqs[f]);
        Serial1.println(" Hz)");

        bool ok = speed_test_at(freqs[f], iterations);

        if (!ok) {
            Serial1.println("  >>> Communication FAILED at this frequency <<<");
            Serial1.println("  Remaining frequencies skipped.");
            break;
        }
        Serial1.println("  OK");
    }

    // Restore safe clock for normal operation
    Wire.setClock(100000);

    Serial1.println();
    Serial1.println("============================================");
    Serial1.println("  Speed Test Complete");
    Serial1.println("============================================");
    Serial1.println();
}

// ============================================================

void setup() {
    Serial1.begin(230400);  // D6(TX) D7(RX)
    delay(100);

    Serial1.println("============================================");
    Serial1.println("  I2C BME280 Reader - XIAO SAMD21");
    Serial1.println("  SDA: D4 (PA08), SCL: D5 (PA09)");
    Serial1.println("============================================");

    Wire.begin();  // D4=SDA, D5=SCL (default I2C pins)
    Wire.setClock(100000);  // 100 kHz Standard mode

    if (!bme280_check()) {
        Serial1.println("ERROR: BME280 not found!");
        Serial1.println("Check wiring:");
        Serial1.println("  BME280 SDA -> D4");
        Serial1.println("  BME280 SCL -> D5");
        Serial1.println("  BME280 VCC -> 3.3V");
        Serial1.println("  BME280 GND -> GND");
        while (1) delay(1000);
    }

    bme280_init();

    // Run speed test once at boot, then switch to normal readings
    run_speed_test();

    Serial1.println("Switching to normal sensor readings (1 s interval)...");
}

void loop() {
    read_sensor();
    delay(1000);
}
