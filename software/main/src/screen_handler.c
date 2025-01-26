#include "freertos/FreeRTOS.h"

#include "ssd1306.h"
#include "settings.h"
#include "resources.h"
#include "shared_variables.h"
#include "screen_handler.h"

#include "esp_log.h"
#include <string.h>

static uint8_t *segment_image_numbers = NULL;
static uint8_t *segment_image_signatures = NULL;
static SSD1306_t dev;

void get_indexes(uint16_t *arr)
{
    uint16_t bpm = get_candidate_bpm();
    arr[0] = get_signature_mode() * 256; // Signature mode * 256
    arr[1] = bpm / 100 * 256;            // Hundreds in bpm * 256
    arr[2] = (bpm / 10) % 10 * 256;      // Tens in bpm * 256
    arr[3] = bpm % 10 * 256;             // Ones in bpm * 256
}

bool is_screen_dim()
{
    // Initialize frame counter for blinking
    static uint8_t frame_counter = 0;
    static bool screen_bright = true;

    // If BPM value and selected BPM value are the same, screen bright
    if (bpm_selcted())
    {
        frame_counter = 0;
        screen_bright = true;
        return false;
    }

    // If blinking is on, count the frames for blinking purposes
    frame_counter++;

    // Determine if screen should be on or off based on frames (4 in each mode)
    if (frame_counter > 4)
    {
        screen_bright = !screen_bright;
        frame_counter = 0;
    }
    return screen_bright;
}

void screen_update_handler_task(void *arg)
{
    // Create tag
    static const char *TAG = "screen_update_handler_task";
    ESP_LOGI(TAG, "Screen update handler task initiated.");

    // Initialize necesary parameters
    TickType_t x_last_wake_time;                      // Time when screen was updated
    const TickType_t x_frequency = pdMS_TO_TICKS(42); // 1000ms = 1 second
    // bool screen_on = true;                           // Screen on/off during blinking

    // Initialize the last wake time
    x_last_wake_time = xTaskGetTickCount();
    while (true)
    {
        // System off state, clear the screen, otherwise display BPM and the signature
        if (get_system_state() == SYSTEM_OFF)
        {
            // Clear the screen
            ssd1306_clear_screen(&dev, false);
        }
        else
        {
            // Set contrast accordingly
            ssd1306_contrast(&dev, is_screen_dim() ? 0x00 : 0xFF);

            // Parse necessary informatiion from bpm variable and determine if blinking should be on
            uint16_t index_array[4];
            get_indexes(index_array);
            for (int page = 0; page < 8; page++)
            {
                if (INVERT_SCREEN)
                {
                    ssd1306_display_image(&dev, page, 96, &segment_image_signatures[index_array[0] + page * 32], 32);
                    ssd1306_display_image(&dev, page, 70, &segment_image_numbers[index_array[1] + page * 32], 32);
                    ssd1306_display_image(&dev, page, 35, &segment_image_numbers[index_array[2] + page * 32], 32);
                    ssd1306_display_image(&dev, page, 0, &segment_image_numbers[index_array[3] + page * 32], 32);
                }
                else
                {
                    ssd1306_display_image(&dev, page, 0, &segment_image_signatures[index_array[0] + page * 32], 32);
                    ssd1306_display_image(&dev, page, 26, &segment_image_numbers[index_array[1] + page * 32], 32);
                    ssd1306_display_image(&dev, page, 61, &segment_image_numbers[index_array[2] + page * 32], 32);
                    ssd1306_display_image(&dev, page, 96, &segment_image_numbers[index_array[3] + page * 32], 32);
                }
            }
        }
        vTaskDelayUntil(&x_last_wake_time, x_frequency);
    }
}

esp_err_t conver_bitmap_to_image(uint8_t segment_display[][192], uint8_t *segment_image, size_t image_count)
{
    uint8_t *buffer = (uint8_t *)malloc(8 * 128); // 8 page 128 pixel
    if (buffer == NULL)
    {
        return ESP_FAIL;
    }

    // Convert from segmentDisplay to segmentImage
    for (int image_index = 0; image_index < image_count; image_index++)
    {
        // ssd1306_clear_screen(&dev, false);
        ssd1306_bitmaps(&dev, 0, 0, segment_display[image_index], 32, 32, false);
        // vTaskDelay(200 / portTICK_PERIOD_MS);

        // Get from internal buffer to local buffer
        // buffer is [8][128] 8 page 128 pixel
        ssd1306_get_buffer(&dev, buffer);

        // Save from buffer to segmentImage
        // segmentImage is [10][8][32] 10 image 8 page 32 pixel
        int segment_image_index = image_index * 256;
        for (int page = 0; page < 8; page++)
        {
            memcpy(&segment_image[segment_image_index + page * 32], &buffer[page * 128], 32);
        }
    }
    free(buffer);
    return ESP_OK;
}

esp_err_t start_screen_handler(void)
{
    // Create tag
    static const char *TAG = "setup_screen";
    ESP_LOGI(TAG, "Screen setup started.");

    // Allocate memory for temporary buffer, Number image buffer and signature image buffer
    segment_image_numbers = (uint8_t *)malloc(NUMBER_IMAGES * 8 * 32);
    segment_image_signatures = (uint8_t *)malloc(SIGNATURE_IMAGES * 8 * 32);

    // If memory allocation fails, handle error.
    if (segment_image_numbers == NULL || segment_image_signatures == NULL)
    {
        ESP_LOGE(TAG, "Segment image memory allocation failed.");
        return ESP_FAIL;
    }

    // Initial screen setup
    i2c_master_init(&dev, SSD1306_SDA_PIN, SSD1306_SCL_PIN, SSD1306_RST_PIN);
    ssd1306_init(&dev, 128, 32);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_clear_screen(&dev, false);

    esp_err_t ret;
    if (INVERT_SCREEN)
    {
        ret = conver_bitmap_to_image(segment_display_numbers_inverse, segment_image_numbers, NUMBER_IMAGES);
    }
    else
    {
        ret = conver_bitmap_to_image(segment_display_numbers, segment_image_numbers, NUMBER_IMAGES);
    }
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Segment image conversion failed.");
        return ESP_FAIL;
    }
    if (INVERT_SCREEN)
    {
        ret = conver_bitmap_to_image(segment_display_signatures_inverse, segment_image_signatures, SIGNATURE_IMAGES);
    }
    else
    {
        ret = conver_bitmap_to_image(segment_display_signatures, segment_image_signatures, SIGNATURE_IMAGES);
    }
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Segment image conversion failed.");
        return ESP_FAIL;
    }

    // Free the buffer and clear the screen
    ssd1306_clear_screen(&dev, false);

    // Setup task parameters and start the task
    BaseType_t x_returned;
    x_returned = xTaskCreate(screen_update_handler_task, "screen_update_handler", 2048, NULL, 10, NULL);
    if (x_returned != pdPASS)
    {
        ESP_LOGE(TAG, "Screen update handler task creation failed.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Screen setup finished.");
    return ESP_OK;
}