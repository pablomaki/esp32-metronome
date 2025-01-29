#include "metronome.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "settings.h"
#include "shared_variables.h"
#include "encoder.h"
#include "output_driver.h"
#include "screen.h"

static encoder_handle_t encoder_handle;
static output_driver_handle_t output_handle;
static screen_handle_t screen_handle;

void handle_sleep_state_task(void *arg)
{
    // Create tag
    static const char *TAG = "handle_sleep_state";
    ESP_LOGI(TAG, "Sleep mode requested, handling request");

    // Unpack the necessary parameters
    QueueHandle_t sleep_queue = (QueueHandle_t)arg; // Sleep request queue
    bool signal;

    while (true)
    {
        if (xQueueReceive(sleep_queue, &signal, pdMS_TO_TICKS(5000)))
        {
            // Enter sleep mode with SW pin as the wakeup
            // Disable peripherals, wait for it to take effect
            esp_err_t ret;
            ret = disable_peripherals();
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to disable the peripherals: %s", esp_err_to_name(ret));
            }
            vTaskDelay(pdMS_TO_TICKS(100));

            // Enable ENC_SW_PIN as sleep wakeup pin, wait for pin to go to untriggered state (high)
            gpio_wakeup_enable(ENC_SW_PIN, GPIO_INTR_LOW_LEVEL);
            esp_sleep_enable_gpio_wakeup();
            ESP_LOGI(TAG, "Waiting for GPIO%d to go high.", ENC_SW_PIN);
            while (gpio_get_level(ENC_SW_PIN) == 0)
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            // Enter sleep mode, wait for mesage to be sent
            ESP_LOGI(TAG, "Entring sleep mode... Wake up configured to GPIO%d", ENC_SW_PIN);
            vTaskDelay(pdMS_TO_TICKS(100));
            esp_light_sleep_start();

            // Wake up, wait for the pin to go to untriggered state (high)
            ESP_LOGI(TAG, "Waking up from sleep mode.");
            ESP_LOGI(TAG, "Waiting for GPIO%d to go high.", ENC_SW_PIN);
            while (gpio_get_level(ENC_SW_PIN) == 0)
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            // Turn the screen, output devices and encoder back on.
            while (enable_peripherals() != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to enable the peripherals: %s", esp_err_to_name(ret));
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
    }
}

esp_err_t enable_peripherals()
{
    // Create tag
    static const char *TAG = "enable_peripherals";
    ESP_LOGI(TAG, "Enabling the peripherals");

    // Start up the encoder handler
    esp_err_t ret;
    ret = start_encoder(encoder_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start the encoder handler: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Start up the output handler
    ret = start_output_devices(output_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start the output handler: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Start up the screen handler
    ret = start_screen_handler(screen_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start the screen handler: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Set system on parameter
    switch_system_on();
    return ESP_OK;
}

esp_err_t disable_peripherals()
{
    // Create tag
    static const char *TAG = "disable_peripherals";
    ESP_LOGI(TAG, "Disabling the peripherals");

    // Set system off parameter
    switch_system_off();

    // Stop the encoder handler
    switch_system_off();
    esp_err_t ret;
    ret = stop_encoder(encoder_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to stop the encoder handler: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Stop the output handler
    ret = stop_output_devices(output_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to stop the output handler: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Stop the screen handler
    ret = stop_screen_handler(screen_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to stop the screen handler: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t setup_peripherals()
{
    static const char *TAG = "setup_peripherals";
    ESP_LOGI(TAG, "Setting up the peripherals");
    esp_err_t ret;

    // Setup the encoder handler
    ret = setup_encoder(encoder_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to setup the encoder handler: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Setup the output handler
    ret = setup_output_devices(output_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to setup the output handler: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Setup the screen handler
    ret = setup_screen_handler(screen_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to setup the screen handler: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t start_metronome(void)
{
    // Create tag
    static const char *TAG = "start_metronome";
    ESP_LOGI(TAG, "Starting metronome");

    // Allocate memory for structs
    encoder_handle = malloc(sizeof(struct encoder));
    output_handle = malloc(sizeof(struct output_dirver));
    screen_handle = malloc(sizeof(struct screen));

    // Assign the sleep handler queue pointer to the encoder_handle, set all task handles to null
    encoder_handle->sleep_request_queue_handle = xQueueCreate(10, sizeof(bool));
    encoder_handle->encoder_task_handle = NULL;
    output_handle->output_task_handle = NULL;
    screen_handle->update_screen_task = NULL;

    // Initialize semaphores for handling the shared variables
    esp_err_t ret;
    ret = init_semaphores();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize semaphores: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Setup the peripherals, only called once
    ret = setup_peripherals();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to setup the peripherals: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Enable peripherals
    ret = enable_peripherals();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start the peripherals: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Create task for handling sleep mode requests
    BaseType_t x_returned;
    x_returned = xTaskCreate(handle_sleep_state_task, "handle_sleep_state_task", 8192, (void *)encoder_handle->sleep_request_queue_handle, 10, NULL);
    if (x_returned != pdPASS)
    {
        ESP_LOGE(TAG, "Sleep state handler task creation failed.");
        return ESP_FAIL;
    }

    return ESP_OK;
}