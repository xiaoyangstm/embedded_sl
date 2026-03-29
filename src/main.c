/*
 * XIAO nRF54L15 双串口测试
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

int main(void)
{
	const struct device *uart21_dev;
	int counter = 0;

	printk("=== UART Test ===\n");

	/* 获取 UART21 设备 */
	uart21_dev = DEVICE_DT_GET(DT_CHOSEN(uart_passthrough));
	printk("UART21 device pointer: %p\n", uart21_dev);

	if (uart21_dev == NULL) {
		printk("ERROR: UART21 device is NULL!\n");
		printk("Check device tree overlay.\n");
	} else if (!device_is_ready(uart21_dev)) {
		printk("ERROR: UART21 device not ready!\n");
	} else {
		printk("UART21 is ready!\n");
	}

	printk("=================\n\n");
	k_sleep(K_MSEC(1000));

	/* 测试循环 - 持续向两个串口发送数据 */
	for (;;) {
		/* 向 UART20 (USB 串口) 发送数据 */
		printk("[%d] USB test\n", counter);

		/* 向 UART21 (GPIO 串口) 发送数据 */
		if (uart21_dev != NULL && device_is_ready(uart21_dev)) {
			uart_poll_out(uart21_dev, 'A');
			uart_poll_out(uart21_dev, 'B');
			uart_poll_out(uart21_dev, 'C');
			uart_poll_out(uart21_dev, '\r');
			uart_poll_out(uart21_dev, '\n');
		}

		counter++;
		k_sleep(K_MSEC(1000));
	}

	return 0;
}
