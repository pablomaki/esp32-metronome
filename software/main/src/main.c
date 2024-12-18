#include "encoder_handler.h"
#include "screen_handler.h"
#include "output_handler.h"
#include "shared_variables.h"

void app_main(void)
{
    // Create tag
    const char *TAG = "app_main";
    ESP_LOGI(TAG, "App main started.");

    // Return value for handling errors from  the called functions
    esp_err_t ret;

    // Initialize semaphores for handling the shared variables
    ret = init_semaphores();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize semaphores: %s", esp_err_to_name(ret));
        esp_restart();
    }

    // Setup and start the encoder handler
    start_encoder_handler();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start the encoder handler: %s", esp_err_to_name(ret));
        esp_restart();
    }

    // Setup and start the screen handler
    start_screen_handler();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start the screen handler: %s", esp_err_to_name(ret));
        esp_restart();
    }

    // Setup and start the output handler
    start_output_handler();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start the output handler: %s", esp_err_to_name(ret));
        esp_restart();
    }
}