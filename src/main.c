#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <string.h>

LOG_MODULE_REGISTER(main_app, CONFIG_LOG_DEFAULT_LEVEL);

/* UART device structures */
static const struct device *const uart_console_dev = DEVICE_DT_GET(DT_NODELABEL(uart20));
static const struct device *const uart_hw_dev = DEVICE_DT_GET(DT_NODELABEL(uart21));

/* Send data to both UARTs (using polling mode) */
static int send_to_both_uarts(const char *data, size_t len)
{
    /* Send to console UART (USB-Serial) - polling mode */
    for (size_t i = 0; i < len; i++) {
        uart_poll_out(uart_console_dev, data[i]);
    }

    /* Send to hardware UART - polling mode */
    for (size_t i = 0; i < len; i++) {
        uart_poll_out(uart_hw_dev, data[i]);
    }

    return 0;
}

/* Send formatted string to both UARTs */
static void printf_both(const char *fmt, ...)
{
    va_list args;
    char buffer[256];

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    send_to_both_uarts(buffer, strlen(buffer));
}

int main(void)
{
    uint32_t counter = 0;

    LOG_INF("Starting Dual UART Example...");
    LOG_INF("Console UART: USB-Serial (uart20)");
    LOG_INF("HW UART: P2.08 (TX/D6), P2.07 (RX/D7)");

    /* Check if console UART device is ready */
    if (!device_is_ready(uart_console_dev)) {
        LOG_ERR("Console UART device not ready");
        return -1;
    }

    /* Check if hardware UART device is ready */
    if (!device_is_ready(uart_hw_dev)) {
        LOG_ERR("Hardware UART device not ready");
        return -1;
    }

    /* Wait a moment for everything to stabilize */
    k_msleep(500);

    LOG_INF("Dual UART system initialized!");
    LOG_INF("Sending data to both Console and Hardware UART...");

    /* Send startup message to both ports */
    printf_both("\r\n");
    printf_both("========================================\r\n");
    printf_both("Dual UART Test Started\r\n");
    printf_both("========================================\r\n");
    printf_both("Console UART: USB-Serial Output\r\n");
    printf_both("HW UART: P2.08(TX/D6), P2.07(RX/D7)\r\n");
    printf_both("Baud Rate: 115200\r\n");
    printf_both("========================================\r\n\r\n");

    /* Main loop - send data to both UARTs periodically */
    while (1) {
        counter++;

        /* Send counter message */
        printf_both("[%u] Hello from Dual UART! Counter: %u\r\n",
                    k_uptime_get_32() / 1000, counter);

        /* Send a longer message every 5 iterations */
        if (counter % 5 == 0) {
            printf_both("[%u] This message appears on BOTH UARTs!\r\n",
                        k_uptime_get_32() / 1000);
        }

        /* Print status every 10 iterations */
        if (counter % 10 == 0) {
            printf_both("[%u] Status: Both UARTs working normally\r\n",
                        k_uptime_get_32() / 1000);
        }

        k_msleep(1000);
    }

    return 0;
}
