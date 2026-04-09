/*
 * SPI Speed Test for XIAO nRF54L15 (Zephyr RTOS)
 *
 * Auto-tests multiple SPI clock frequencies, finds the maximum working speed.
 *
 * Wiring:
 *   W25Q128JV SCK  -> D8  (P2.01)
 *   W25Q128JV MISO -> D9  (P2.04)
 *   W25Q128JV MOSI -> D10 (P2.02)
 *   W25Q128JV CS   -> D1  (P1.05)
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(spi_speed_test, CONFIG_LOG_DEFAULT_LEVEL);

#define SPI_DEV_NODE DT_NODELABEL(spi00)
#define CS_GPIO_NODE DT_NODELABEL(gpio1)
#define CS_PIN       5

/* W25Q128JV commands */
#define CMD_JEDEC_ID     0x9F
#define CMD_WRITE_ENABLE 0x06
#define CMD_READ_STATUS1 0x05
#define CMD_SECTOR_ERASE 0x20
#define CMD_PAGE_PROGRAM 0x02
#define CMD_READ_DATA    0x03

/* Frequencies to test (high -> low) */
static const uint32_t test_freqs[] = {
	32000000,
	24000000,
	16000000,
	12000000,
	8000000,
	4000000,
	2000000,
	1000000,
};
#define NUM_FREQS (sizeof(test_freqs) / sizeof(test_freqs[0]))

static const uint8_t tx_pattern[] = {
	0x57, 0x32, 0x35, 0x51, 0x31, 0x32, 0x38, 0x4A,
	0x56, 0x20, 0x38, 0x4D, 0x48, 0x7A, 0x20, 0x64,
	0x65, 0x6D, 0x6F, 0x20, 0x40, 0x20, 0x30, 0x78,
	0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
};

static const struct device *spi_dev;
static const struct device *cs_gpio;
static struct spi_config spi_cfg;

static inline void cs_low(void)  { gpio_pin_set(cs_gpio, CS_PIN, 0); }
static inline void cs_high(void) { gpio_pin_set(cs_gpio, CS_PIN, 1); }

/* ---- Flash helpers ---- */

static bool spi_read_jedec_id(uint8_t id[3])
{
	uint8_t tx[4] = { CMD_JEDEC_ID, 0xFF, 0xFF, 0xFF };
	uint8_t rx[4];

	struct spi_buf tx_b = { .buf = tx, .len = sizeof(tx) };
	struct spi_buf rx_b = { .buf = rx, .len = sizeof(rx) };
	struct spi_buf_set tx_s = { .buffers = &tx_b, .count = 1 };
	struct spi_buf_set rx_s = { .buffers = &rx_b, .count = 1 };

	cs_low();
	int ret = spi_transceive(spi_dev, &spi_cfg, &tx_s, &rx_s);
	cs_high();

	if (ret) {
		return false;
	}

	id[0] = rx[1];
	id[1] = rx[2];
	id[2] = rx[3];
	return true;
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

static bool read_flash(uint32_t addr, uint8_t *buf, size_t len)
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
	int ret = spi_transceive(spi_dev, &spi_cfg, &tx_s, &rx_s);
	cs_high();

	if (ret) {
		return false;
	}

	memcpy(buf, &rx[4], len);
	return true;
}

/* ---- Speed test ---- */

static bool test_at_freq(uint32_t freq_hz)
{
	uint8_t jedec_id[3] = { 0 };
	uint8_t rx_buf[sizeof(tx_pattern)] = { 0 };
	bool ok = true;

	spi_cfg.frequency = freq_hz;

	/* Test 1: JEDEC ID */
	if (!spi_read_jedec_id(jedec_id)) {
		LOG_INF("  %2u MHz: JEDEC ID read FAILED (SPI error)", freq_hz / 1000000);
		return false;
	}
	if (jedec_id[0] == 0xFF && jedec_id[1] == 0xFF && jedec_id[2] == 0xFF) {
		LOG_INF("  %2u MHz: JEDEC ID = FF FF FF (no response)", freq_hz / 1000000);
		return false;
	}

	/* Test 2: erase + write + readback */
	sector_erase(0x000000);
	page_program(0x000000, tx_pattern, sizeof(tx_pattern));

	if (!read_flash(0x000000, rx_buf, sizeof(rx_buf))) {
		LOG_INF("  %2u MHz: read FAILED (SPI error)", freq_hz / 1000000);
		return false;
	}

	if (memcmp(tx_pattern, rx_buf, sizeof(tx_pattern)) != 0) {
		LOG_INF("  %2u MHz: JEDEC OK (%02X%02X%02X) but data mismatch!",
			freq_hz / 1000000, jedec_id[0], jedec_id[1], jedec_id[2]);
		return false;
	}

	LOG_INF("  %2u MHz: JEDEC=%02X%02X%02X  read/write OK",
		freq_hz / 1000000, jedec_id[0], jedec_id[1], jedec_id[2]);
	return true;
}

/* ---- Main ---- */

int main(void)
{
	LOG_INF("======== SPI Speed Test ========");
	LOG_INF("Board: XIAO nRF54L15");
	LOG_INF("SCK=D8/P2.01  MISO=D9/P2.04  MOSI=D10/P2.02  CS=D1/P1.05");

	cs_gpio = DEVICE_DT_GET(CS_GPIO_NODE);
	if (!device_is_ready(cs_gpio)) {
		LOG_ERR("GPIO not ready");
		return 0;
	}
	gpio_pin_configure(cs_gpio, CS_PIN, GPIO_OUTPUT_ACTIVE);
	cs_high();

	spi_dev = DEVICE_DT_GET(SPI_DEV_NODE);
	if (!device_is_ready(spi_dev)) {
		LOG_ERR("SPI not ready");
		return 0;
	}

	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8);

	LOG_INF("Testing frequencies (high -> low):");

	uint32_t max_ok_freq = 0;

	for (int i = 0; i < NUM_FREQS; i++) {
		bool ok = test_at_freq(test_freqs[i]);
		if (ok && max_ok_freq == 0) {
			max_ok_freq = test_freqs[i];
		}
	}

	LOG_INF("================================");
	if (max_ok_freq > 0) {
		LOG_INF(">> Max working SPI speed: %u MHz", max_ok_freq / 1000000);
	} else {
		LOG_INF(">> No speed worked!");
	}

	while (1) {
		k_msleep(5000);
	}
	return 0;
}
