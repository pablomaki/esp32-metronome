#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "settings.h"
#include "esp_err.h"
#include "shared_variables.h"
#include "esp_log.h"
#include "output_handler.h"
#include "driver/gptimer.h"
#include "resources.h"

/**
 * Handle output timer alarms. Mark output to be activated and set new timer based on the current bpm
 *
 * @param timer Timer related to the event.
 * @param edata Event data.
 * @param user_data Arguments passed to the event.
 * @return void.
 */
static bool IRAM_ATTR output_timer_alarm(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    // Create bool for high_task_awoken
    BaseType_t high_task_awoken = pdFALSE;

    // Unpack the necessary parameters
    QueueHandle_t queue = (QueueHandle_t)user_data;
    uint16_t bpm = get_selected_bpm();

    // Send a "true" signal to the queue from the ISR
    bool signal = true; // Just a simple boolean signal
    xQueueSendFromISR(queue, &signal, &high_task_awoken);

    // Set new alarm based on the current bpm
    uint64_t alarm_count;
    if (get_beat() == 0)
    {
        alarm_count = edata->alarm_value + 60 * 1000000 / (bpm * 4.0);
    }
    else if (get_beat() == 1)
    {
        alarm_count = edata->alarm_value + 60 * 1000000 / (bpm * 4.0 / 3.0);
    }
    else
    {
        alarm_count = edata->alarm_value + 60 * 1000000 / bpm;
    }
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = alarm_count}; // bpm
    gptimer_set_alarm_action(timer, &alarm_config);
    return (high_task_awoken == pdTRUE);
}

void click(uint8_t x, bool led_on)
{
    gpio_set_level(LED_PIN, led_on); // Set pin high
    for (int n = 0; n < x; n++)
    {
        gpio_set_level(OUTPUT_PIN, true);                            // Set pin high
        vTaskDelay(OUTPUT_ACTIVATION_DURATION / portTICK_PERIOD_MS); // Delay for y milliseconds
        gpio_set_level(OUTPUT_PIN, false);
        vTaskDelay(OUTPUT_ACTIVATION_DURATION / 4 / portTICK_PERIOD_MS); // Delay for y milliseconds
    }
    gpio_set_level(LED_PIN, false); // Set pin high
}

void output_handler_task(void *arg)
{
    // Create tag
    static const char *TAG = "output_handler_task";
    ESP_LOGI(TAG, "Output handler task initiated.");

    // Unpack the necessary parameters
    QueueHandle_t output_activation_queue = (QueueHandle_t)arg; // Encoder tick queue

    // Track the current beat count
    // uint16_t beat = 0;
    bool signal;

    while (1)
    {
        // Wait for output activation flag to be activated
        if (xQueueReceive(output_activation_queue, &signal, portMAX_DELAY))
        {
            // Activate and deactivate the output after predermined duration
            if (get_beat() == 0 || get_beat() == 1)
            {
                click(1, true);
            }
            else
            {
                click(1, false);
            }
            increment_beat();
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Adjust the delay as needed
    }
}

esp_err_t start_output_handler(void)
{
    // Create tag
    static const char *TAG = "start_output_handler";
    ESP_LOGI(TAG, "Output handler setup started.");

    // Create encoder action queue
    static QueueHandle_t output_activation_queue = NULL;
    output_activation_queue = xQueueCreate(10, sizeof(bool));

    // Check that queue creation succeeded
    if (output_activation_queue == NULL)
    {
        ESP_LOGE(TAG, "Encoder action queue creation failed.");
        return ESP_FAIL;
    }

    /* Set the GPIO as a push/pull output */
    gpio_reset_pin(OUTPUT_PIN);
    gpio_set_direction(OUTPUT_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // Create timer handle
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
        .intr_priority = 1,
    };

    // Create timer, return error if not succesful
    esp_err_t ret;
    ret = gptimer_new_timer(&timer_config, &gptimer);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Output timer creation failed.");
        return ret;
    }

    // Set callback for the timer, return error if not succesful
    gptimer_event_callbacks_t cbs = {
        .on_alarm = output_timer_alarm,
    };
    ret = gptimer_register_event_callbacks(gptimer, &cbs, (void *)output_activation_queue);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Output timer callback registration failed.");
        return ret;
    }

    // Enable, set first alarm and start the timer.
    ret = gptimer_enable(gptimer);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Output timer enable failed.");
        return ret;
    }
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 1000000, // period = 1s
    };
    ret = gptimer_set_alarm_action(gptimer, &alarm_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Output timer set alarm failed.");
        return ret;
    }
    ret = gptimer_start(gptimer);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Output timer start failed.");
        return ret;
    }

    // Setup task parameters and start the task
    BaseType_t x_returned;
    x_returned = xTaskCreate(output_handler_task, "output_handler_task", 2048, (void *)output_activation_queue, 10, NULL);
    if (x_returned != pdPASS)
    {
        ESP_LOGE(TAG, "Output handler task creation failed.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Output driver setup finished.");
    return ESP_OK;
}