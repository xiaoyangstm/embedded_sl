/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/drivers/gpio.h>

#include "transport/dtm_transport.h"
#include "dtm.h"

/* Auto TX configuration */
#define AUTO_TX_CHANNEL    0   /* RF Channel 0 = 2402 MHz */
#define AUTO_TX_POWER      8   /* TX Power in dBm */
#define AUTO_TX_LENGTH     37  /* Packet length in bytes */
#define AUTO_TX_PACKET     DTM_PACKET_55  /* 0x55 pattern */

/* Antenna selection for XIAO nRF54L15 */
#define ANTENNA_ON_BOARD   0   /* Use on-board antenna */
#define ANTENNA_EXTERNAL   1   /* Use external antenna (U.FL) */

/* Select antenna: Change this value to switch antennas */
#define ANTENNA_SELECTION  ANTENNA_EXTERNAL

/* RF Switch control pins for XIAO nRF54L15 */
#define RFSW_CTL_NODE  DT_NODELABEL(rfsw_ctl)
#define RFSW_PWR_NODE  DT_NODELABEL(rfsw_pwr)

#if DT_NODE_HAS_PROP(RFSW_CTL_NODE, enable_gpios)
#define RFSW_CTL_PIN  DT_GPIO_PIN(RFSW_CTL_NODE, enable_gpios)
#define RFSW_CTL_FLAGS  DT_GPIO_FLAGS(RFSW_CTL_NODE, enable_gpios)
#else
#define RFSW_CTL_PIN  255  /* Not defined */
#endif

#if DT_NODE_HAS_PROP(RFSW_PWR_NODE, enable_gpios)
#define RFSW_PWR_PIN  DT_GPIO_PIN(RFSW_PWR_NODE, enable_gpios)
#define RFSW_PWR_FLAGS  DT_GPIO_FLAGS(RFSW_PWR_NODE, enable_gpios)
#else
#define RFSW_PWR_PIN  255  /* Not defined */
#endif

/**
 * @brief Configure antenna selection for XIAO nRF54L15
 *
 * Controls the RF switch to select between on-board and external antenna.
 *
 * @param antenna  ANTENNA_ON_BOARD or ANTENNA_EXTERNAL
 * @return 0 on success, negative error code on failure
 */
static int antenna_select(uint8_t antenna)
{
#if RFSW_CTL_PIN != 255 && RFSW_PWR_PIN != 255
	const struct device *gpio_dev;
	int ret;

	/* Get GPIO device */
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio2));
	if (!device_is_ready(gpio_dev)) {
		printk("Error: GPIO device not ready\n");
		return -ENODEV;
	}

	/* Configure RF Switch Power pin (P2.03) - always enable */
	ret = gpio_pin_configure(gpio_dev, RFSW_PWR_PIN,
				 GPIO_OUTPUT_ACTIVE | RFSW_PWR_FLAGS);
	if (ret) {
		printk("Error configuring RFSW_PWR pin: %d\n", ret);
		return ret;
	}

	/* Configure RF Switch Control pin (P2.05) */
	ret = gpio_pin_configure(gpio_dev, RFSW_CTL_PIN,
				 GPIO_OUTPUT | RFSW_CTL_FLAGS);
	if (ret) {
		printk("Error configuring RFSW_CTL pin: %d\n", ret);
		return ret;
	}

	/* Set antenna selection */
	if (antenna == ANTENNA_EXTERNAL) {
		gpio_port_set_masked_raw(gpio_dev, BIT(RFSW_CTL_PIN),
					 BIT(RFSW_CTL_PIN));  /* HIGH = External */
		printk("Antenna: EXTERNAL (U.FL connector)\n");
	} else {
		gpio_port_set_masked_raw(gpio_dev, BIT(RFSW_CTL_PIN), 0);  /* LOW = On-board */
		printk("Antenna: ON-BOARD\n");
	}

	printk("RF Switch enabled (P2.03 HIGH, P2.05 %s)\n",
	       antenna == ANTENNA_EXTERNAL ? "HIGH" : "LOW");

	return 0;
#else
	printk("Antenna control not available on this board\n");
	return -ENOTSUP;
#endif
}

int main(void)
{
	int err;
	union dtm_tr_packet cmd;
	struct dtm_tx_power tx_power;

	printk("Starting Direct Test Mode sample\n");

#if defined(CONFIG_SOC_SERIES_NRF54HX) || defined(CONFIG_SOC_SERIES_NRF54LX)
	const struct device *dtm_uart = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(ncs_dtm_uart));

	if (dtm_uart != NULL) {
		int ret = pm_device_runtime_get(dtm_uart);

		if (ret < 0) {
			printk("Failed to get DTM UART runtime PM: %d\n", ret);
		}
	}
#endif /* defined(CONFIG_SOC_SERIES_NRF54HX) || defined(CONFIG_SOC_SERIES_NRF54LX) */

	err = dtm_tr_init();
	if (err) {
		printk("Error initializing DTM transport: %d\n", err);
		return err;
	}

	/* Configure antenna selection */
	err = antenna_select(ANTENNA_SELECTION);
	if (err) {
		printk("Warning: Antenna selection failed: %d\n", err);
		/* Continue anyway - use default antenna */
	}

	/* Auto-start TX at 8dBm on channel 0 */
	printk("Setting TX power to %d dBm\n", AUTO_TX_POWER);
	tx_power = dtm_setup_set_transmit_power(DTM_TX_POWER_REQUEST_VAL,
					       AUTO_TX_POWER,
					       AUTO_TX_CHANNEL);
	printk("Actual TX power: %d dBm\n", tx_power.power);

	printk("Starting TX test on channel %d (2402 MHz)\n", AUTO_TX_CHANNEL);
	err = dtm_test_transmit(AUTO_TX_CHANNEL, AUTO_TX_LENGTH, AUTO_TX_PACKET);
	if (err) {
		printk("Error starting TX test: %d\n", err);
		return err;
	}
	printk("TX test started! Packet: 0x55, Length: %d bytes\n", AUTO_TX_LENGTH);

	for (;;) {
		cmd = dtm_tr_get();
		err = dtm_tr_process(cmd);
		if (err) {
			printk("Error processing command: %d\n", err);
			return err;
		}
	}
}
