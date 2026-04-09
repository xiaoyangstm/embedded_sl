/*
 * W25Q128JV SPI Flash Test for XIAO nRF54L15 (Zephyr RTOS)
 *
 * Wiring:
 *   W25Q128JV SCK  -> D8  (P2.01)
 *   W25Q128JV MISO -> D9  (P2.04)
 *   W25Q128JV MOSI -> D10 (P2.02)
 *   W25Q128JV CS   -> D1  (P1.05)
 *
 * WP# and HOLD# are assumed to be held high by external hardware.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(spi_flash_test, CONFIG_LOG_DEFAULT_LEVEL);

/* SPI device: spi00 (xiao_spi), already configured in board DTS */
#define SPI_DEV_NODE DT_NODELABEL(spi00)

/* CS pin: D1 = P1.05 (gpio1, pin 5) */
#define CS_GPIO_NODE DT_NODELABEL(gpio1)
#define CS_PIN       5

/* SPI clock frequency */
#define SPI_FREQ_HZ 8000000

/* W25Q128JV command set */
#define CMD_JEDEC_ID     0x9F
#define CMD_WRITE_ENABLE 0x06
#define CMD_READ_STATUS1 0x05
#define CMD_SECTOR_ERASE 0x20
#define CMD_PAGE_PROGRAM 0x02
#define CMD_READ_DATA    0x03

/* Supported JEDEC IDs: Winbond W25Q128JV, Gigadevice GD25Q128 */
static const uint8_t supported_ids[][3] = {
    { 0xEF, 0x40, 0x18 },
    { 0xC8, 0x40, 0x18 },
};

static const uint8_t tx_pattern[] = {
    0x57, 0x32, 0x35, 0x51, 0x31, 0x32, 0x38, 0x4A,
    0x56, 0x20, 0x38, 0x4D, 0x48, 0x7A, 0x20, 0x64,
    0x65, 0x6D, 0x6F, 0x20, 0x40, 0x20, 0x30, 0x78,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
};

static const struct device *spi_dev;
static const struct device *cs_gpio;
static struct spi_config spi_cfg;

/* ---- CS helpers ---- */

static inline void cs_low(void)  { gpio_pin_set(cs_gpio, CS_PIN, 0); }
static inline void cs_high(void) { gpio_pin_set(cs_gpio, CS_PIN, 1); }

/* ---- Flash operations ---- */

static void spi_read_jedec_id(uint8_t id[3])
{
    uint8_t tx[4] = { CMD_JEDEC_ID, 0xFF, 0xFF, 0xFF };
    uint8_t rx[4];

    struct spi_buf tx_b = { .buf = tx, .len = sizeof(tx) };
    struct spi_buf rx_b = { .buf = rx, .len = sizeof(rx) };
    struct spi_buf_set tx_s = { .buffers = &tx_b, .count = 1 };
    struct spi_buf_set rx_s = { .buffers = &rx_b, .count = 1 };

    cs_low();
    spi_transceive(spi_dev, &spi_cfg, &tx_s, &rx_s);
    cs_high();

    id[0] = rx[1];
    id[1] = rx[2];
    id[2] = rx[3];
}

static bool jedec_id_supported(const uint8_t id[3])
{
    for (size_t i = 0; i < sizeof(supported_ids) / sizeof(supported_ids[0]); i++) {
        if (memcmp(id, supported_ids[i], 3) == 0)
            return true;
    }
    return false;
}

static uint8_t read_status1(void)
{
    uint8_t tx[2] = { CMD_READ_STATUS1, 0xFF };
    uint8_t rx[2];

    struct spi_buf tx_b = { .buf = tx, .len = sizeof(tx) };
    struct spi_buf rx_b = { .buf = rx, .len = sizeof(rx) };
    struct spi_buf_set tx_s = { .buffers = &tx_b, .count = 1 };
    struct spi_buf_set rx_s = { .buffers = &rx_b, .count = 1 };

    cs_low();
    spi_transceive(spi_dev, &spi_cfg, &tx_s, &rx_s);
    cs_high();

    return rx[1];
}

static void wait_busy(void)
{
    while (read_status1() & 0x01) {
        k_msleep(1);
    }
}

static void write_enable(void)
{
    uint8_t cmd = CMD_WRITE_ENABLE;
    struct spi_buf tx_b = { .buf = &cmd, .len = 1 };
    struct spi_buf_set tx_s = { .buffers = &tx_b, .count = 1 };

    cs_low();
    spi_write(spi_dev, &spi_cfg, &tx_s);
    cs_high();
}

static void sector_erase(uint32_t addr)
{
    write_enable();

    uint8_t tx[4] = {
        CMD_SECTOR_ERASE,
        (uint8_t)(addr >> 16),
        (uint8_t)(addr >> 8),
        (uint8_t)(addr),
    };
    struct spi_buf tx_b = { .buf = tx, .len = sizeof(tx) };
    struct spi_buf_set tx_s = { .buffers = &tx_b, .count = 1 };

    cs_low();
    spi_write(spi_dev, &spi_cfg, &tx_s);
    cs_high();

    wait_busy();
}

static void page_program(uint32_t addr, const uint8_t *data, size_t len)
{
    write_enable();

    uint8_t header[4] = {
        CMD_PAGE_PROGRAM,
        (uint8_t)(addr >> 16),
        (uint8_t)(addr >> 8),
        (uint8_t)(addr),
    };
    struct spi_buf tx_bufs[2] = {
        { .buf = header, .len = sizeof(header) },
        { .buf = (void *)data, .len = len },
    };
    struct spi_buf_set tx_s = { .buffers = tx_bufs, .count = 2 };

    cs_low();
    spi_write(spi_dev, &spi_cfg, &tx_s);
    cs_high();

    wait_busy();
}

static void read_flash(uint32_t addr, uint8_t *buf, size_t len)
{
    static uint8_t tx[4 + 256];
    static uint8_t rx[4 + 256];
    size_t total = 4 + len;

    tx[0] = CMD_READ_DATA;
    tx[1] = (uint8_t)(addr >> 16);
    tx[2] = (uint8_t)(addr >> 8);
    tx[3] = (uint8_t)(addr);
    memset(&tx[4], 0xFF, len);

    struct spi_buf tx_b = { .buf = tx, .len = total };
    struct spi_buf rx_b = { .buf = rx, .len = total };
    struct spi_buf_set tx_s = { .buffers = &tx_b, .count = 1 };
    struct spi_buf_set rx_s = { .buffers = &rx_b, .count = 1 };

    cs_low();
    spi_transceive(spi_dev, &spi_cfg, &tx_s, &rx_s);
    cs_high();

    memcpy(buf, &rx[4], len);
}

static void dump_hex(const char *tag, const uint8_t *data, size_t len)
{
    for (size_t off = 0; off < len; off += 16) {
        char line[80];
        int pos = 0;

        pos += snprintf(line + pos, sizeof(line) - pos,
                        "%s %03X: ", tag, (unsigned int)off);
        for (size_t i = 0; i < 16 && (off + i) < len; i++)
            pos += snprintf(line + pos, sizeof(line) - pos,
                            "%02X ", data[off + i]);
        LOG_INF("%s", line);
    }
}

/* ---- Main ---- */

int main(void)
{
    LOG_INF("================ W25Q128JV SPI Test ================");
    LOG_INF("Board: XIAO nRF54L15");
    LOG_INF("SPI clock: 8 MHz");
    LOG_INF("Wiring: SCK=D8/P2.01 MISO=D9/P2.04 MOSI=D10/P2.02 CS=D1/P1.05");

    /* Initialize CS GPIO */
    cs_gpio = DEVICE_DT_GET(CS_GPIO_NODE);
    if (!device_is_ready(cs_gpio)) {
        LOG_ERR("GPIO device not ready");
        return 0;
    }
    gpio_pin_configure(cs_gpio, CS_PIN, GPIO_OUTPUT_ACTIVE);
    cs_high();

    /* Initialize SPI */
    spi_dev = DEVICE_DT_GET(SPI_DEV_NODE);
    if (!device_is_ready(spi_dev)) {
        LOG_ERR("SPI device not ready");
        return 0;
    }

    spi_cfg.frequency = SPI_FREQ_HZ;
    spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8);
    /* Step 1: Read JEDEC ID */
    uint8_t jedec_id[3] = { 0 };
    spi_read_jedec_id(jedec_id);

    LOG_INF("JEDEC ID: %02X %02X %02X", jedec_id[0], jedec_id[1], jedec_id[2]);

    if (!jedec_id_supported(jedec_id)) {
        LOG_ERR("Unexpected JEDEC ID, supported: EF 40 18 / C8 40 18");
        return 0;
    }
    LOG_INF("Flash detected!");

    /* Step 2: Erase sector at 0x000000 */
    LOG_INF("Erasing sector 0x000000 ...");
    sector_erase(0x000000);
    LOG_INF("Erase OK");

    /* Step 3: Write test pattern */
    LOG_INF("Writing %d bytes ...", (int)sizeof(tx_pattern));
    page_program(0x000000, tx_pattern, sizeof(tx_pattern));
    LOG_INF("Write OK");

    /* Step 4: Read back */
    uint8_t rx_buf[sizeof(tx_pattern)] = { 0 };
    read_flash(0x000000, rx_buf, sizeof(rx_buf));

    dump_hex("TX", tx_pattern, sizeof(tx_pattern));
    dump_hex("RX", rx_buf, sizeof(rx_buf));

    /* Step 5: Verify */
    if (memcmp(tx_pattern, rx_buf, sizeof(tx_pattern)) == 0) {
        LOG_INF("Verify OK");
    } else {
        LOG_ERR("Verify FAILED: data mismatch");
    }

    LOG_INF("Test finished.");

    while (1) {
        k_msleep(5000);
    }

    return 0;
}
