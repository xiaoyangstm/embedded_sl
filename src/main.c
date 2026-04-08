#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(adc_4ch, CONFIG_LOG_DEFAULT_LEVEL);

// ADC通道配置
#define ADC_CHANNEL_COUNT 4

// ADC通道名称映射
// nRF54L15的SAADC AIN引脚在P1端口：
// AIN0 = P1.04 (D0), AIN1 = P1.05 (D1), AIN2 = P1.06 (D2), AIN3 = P1.07 (D3)
static const char *const adc_channel_names[ADC_CHANNEL_COUNT] = {
    "A0 (D0/P1.04/AIN0)",
    "A1 (D1/P1.05/AIN1)",
    "A2 (D2/P1.06/AIN2)",
    "A3 (D3/P1.07/AIN3)"
};

// 从设备树获取ADC通道配置
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified for ADC channels"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)
};

int main(void)
{
    int ret;
    int16_t adc_raw;
    int32_t adc_mv;

    LOG_INF("========================================");
    LOG_INF("4-Channel ADC Voltage Monitor");
    LOG_INF("nRF54L15 SAADC - P1 Port Analog Inputs");
    LOG_INF("========================================");

    // 初始化所有ADC通道
    for (int i = 0; i < ADC_CHANNEL_COUNT; i++) {
        if (!adc_is_ready_dt(&adc_channels[i])) {
            LOG_ERR("ADC channel %d not ready", i);
            return 0;
        }

        ret = adc_channel_setup_dt(&adc_channels[i]);
        if (ret < 0) {
            LOG_ERR("Failed to setup ADC channel %d: %d", i, ret);
            return 0;
        }

        LOG_INF("Channel %d (%s) - Resolution: %d bits",
                i, adc_channel_names[i], adc_channels[i].resolution);
    }

    LOG_INF("Starting measurements...\n");

    // 主测量循环
    while (1) {
        printk("\n========== ADC Reading ==========\n");

        for (int i = 0; i < ADC_CHANNEL_COUNT; i++) {
            // 配置ADC序列
            struct adc_sequence sequence = {
                .buffer = &adc_raw,
                .buffer_size = sizeof(adc_raw),
            };

            (void)adc_sequence_init_dt(&adc_channels[i], &sequence);

            // 读取ADC值
            ret = adc_read(adc_channels[i].dev, &sequence);
            if (ret < 0) {
                LOG_ERR("ADC read failed on channel %d: %d", i, ret);
                printk("[%s] ERROR: Read failed\n", adc_channel_names[i]);
                continue;
            }

            // 转换为毫伏
            adc_mv = adc_raw;
            ret = adc_raw_to_millivolts_dt(&adc_channels[i], &adc_mv);
            if (ret < 0) {
                printk("[%s] Raw: %d (conversion failed)\n",
                       adc_channel_names[i], adc_raw);
                continue;
            }

            // 打印结果
            int voltage_v_int = adc_mv / 1000;
            int voltage_v_frac = ((adc_mv % 1000) + 50) / 100;

            printk("[%s] %4d mV  |  %d.%d V  |  Raw: %d\n",
                   adc_channel_names[i],
                   adc_mv,
                   voltage_v_int, voltage_v_frac,
                   adc_raw);
        }

        k_msleep(500);
    }

    return 0;
}
