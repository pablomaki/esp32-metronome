#include "metronome.h"
#include "esp_log.h"
#include "esp_system.h"

void app_main(void)
{
    // Create tag
    const char *TAG = "app_main";
    ESP_LOGI(TAG, "App main started.");

    // Return value for handling errors from  the called functions
    esp_err_t ret;

    // Start metronome
    ret = start_metronome();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start the metronome: %s", esp_err_to_name(ret));
        esp_restart();
    }
    ESP_LOGI(TAG, "Metronome started successfully, exiting app_main...");
}