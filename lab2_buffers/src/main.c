#include <esp_timer.h>
#include "esp_task_wdt.h"
#include "driver/dac.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "soc/dac_channel.h"
#include "esp32/rom/ets_sys.h"

//The sampling freq to be used
int freq = 10000;

static uint16_t buffer[10000] = {0};
static uint32_t k = 0;

// Callback for how often you sample.
static void periodic_timer_callback(void *arg)
{
    gpio_set_level(GPIO_NUM_14, 1);
    float invalue = adc1_get_raw(ADC1_CHANNEL_4);
    invalue = invalue / 16;

    //float outvalue;

    buffer[k] = invalue;
    k++;
    if (k == 10000)
        k = 0;
    float outvalue = buffer[k] + invalue;
    
    dac_output_voltage(DAC_CHANNEL_1, (int)outvalue);
    gpio_set_level(GPIO_NUM_14, 0);
}

void app_main()
{
    // init adc and dac
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
    gpio_pullup_en(GPIO_NUM_32);
    dac_output_enable(DAC_CHANNEL_1);

    gpio_config_t config;
    config.pin_bit_mask = 1 << GPIO_NUM_14;
    config.mode = GPIO_MODE_DEF_OUTPUT;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&config));

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        // name is optional, but may help identify the timer when debugging
        .name = "periodic"};

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000 / freq));
}