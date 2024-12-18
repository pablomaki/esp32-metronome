#include "encoder_handler.h"
#include "shared_variables.h"

action_t action_select = {0, 0, 0};
action_t action_up = {0, 0, 1};
action_t action_down = {0, 0, -1};

void handle_select(int8_t prev_direction, encoder_tick_t *tick)
{
    static const char *TAG = "encoder_handler_task";

    // Nullify up and down button counters
    action_up.consecutive_ticks = 0;
    action_down.consecutive_ticks = 0;

    // In case of doubleclick, change signature mode and reset double click counter
    if (tick->time - action_select.prev_tick_time < DOUBLE_CLICK_US && tick->direction == prev_direction && action_select.consecutive_ticks > 0)
    {
        ESP_LOGI(TAG, "Changing the signature mode");
        change_signature_mode();
        action_select.consecutive_ticks = 0;
    }
    // In case of single click, select the bpm
    else
    {
        // Select the current bpm
        ESP_LOGI(TAG, "Changing the bpm to %d", get_candidate_bpm());
        select_bpm();

        // Update select button info and set direction as previous direction
        action_select.consecutive_ticks++;
        action_select.prev_tick_time = tick->time;
    }
}

int8_t handle_up_down(int8_t prev_direction, encoder_tick_t *tick, action_t *action)
{
    // Nullify up and select button counters
    action_select.consecutive_ticks = 0;
    if (action->direction == 1)
    {
        action_down.consecutive_ticks = 0;
    }
    else
    {
        action_up.consecutive_ticks = 0;
    }

    // Count towards fast changes if time between last tick is below the set limit and direction is the same
    // In case direction is different or time since last change is over 1 second, zero the count
    uint64_t time_since_last_tick = tick->time - action->prev_tick_time;
    if (time_since_last_tick < FAST_CHANGE_US && tick->direction == prev_direction)
    {
        action->consecutive_ticks++;
    }
    // In case direction is different or time since last change is over the set limit, zero the count
    else if (time_since_last_tick > FAST_CHANGE_EXPIRE_US || tick->direction != prev_direction)
    {
        action->consecutive_ticks = 1;
    }
    // Update select button info
    action->prev_tick_time = tick->time;

    // With 3 or more fast changes, multiply change by the set value
    return action->consecutive_ticks > 3 ? tick->direction * FAST_CHANGE_MULTIPLIER : tick->direction;
}

void encoder_handler_task(void *arg)
{
    // Create tag
    static const char *TAG = "encoder_handler_task";
    ESP_LOGI(TAG, "Encoder handler task initiated.");

    // Unpack the necessary parameters
    QueueHandle_t encoder_tick_queue = (QueueHandle_t)arg; // Encoder tick queue

    // Define necessary parameters
    encoder_tick_t tick;
    int8_t prev_direction = 0;

    // Loop for receiving ticks from the queue
    while (true)
    {
        if (xQueueReceive(encoder_tick_queue, &tick, pdMS_TO_TICKS(5000)))
        {
            // Handle bpm set command
            if (tick.direction == 0)
            {
                handle_select(prev_direction, &tick);
                prev_direction = tick.direction;
                continue;
            }
            // Handle down click and get multiplier for changing the bpm value
            else if (tick.direction == -1)
            {
                int8_t bpm_delta = handle_up_down(prev_direction, &tick, &action_down);
                prev_direction = tick.direction;
                change_bpm(bpm_delta);
            }
            // Handle up click and get multiplier for changing the bpm value
            else if (tick.direction == 1)
            {
                int8_t bpm_delta = handle_up_down(prev_direction, &tick, &action_up);
                prev_direction = tick.direction;
                change_bpm(bpm_delta);
            }
            else
            {
                ESP_LOGW(TAG, "Unknown tick direction: %d", tick.direction);
                prev_direction = tick.direction;
            }
        }
        // Revert any change in BPM not selected with unconfirmed changes and no action for too long
        else if (!bpm_selcted())
        {
            reset_candidate_bpm();
            ESP_LOGI(TAG, "BPM change not confirmed in time, reverting.");
        }
    }
}

esp_err_t start_encoder_handler(void)
{
    // Create tag
    const char *TAG = "start_encoder_reader";
    ESP_LOGI(TAG, "Encoder setup started.");

    // Create encoder action queue
    static QueueHandle_t encoder_action_queue = NULL;
    encoder_action_queue = xQueueCreate(10, sizeof(encoder_tick_t));

    // Check that queue creation succeeded
    if (encoder_action_queue == NULL)
    {
        ESP_LOGE(TAG, "Encoder action queue creation failed.");
        return ESP_FAIL;
    }

    static encoder_reader_handle_t encoder;
    const encoreder_reader_settings_t encoder_reader_settings = {
        .pin_a = ENC_A_PIN,
        .pin_b = ENC_B_PIN,
        .pin_sw = ENC_SW_PIN,
        .a_debounce_us = ENC_A_DEBOUNCE,
        .b_debounce_us = ENC_B_DEBOUNCE,
        .sw_debounce_us = ENC_SW_DEBOUNCE,
        .tick_queue = encoder_action_queue,
    };

    // Setup encoder reader, return error if not succesful
    esp_err_t ret;
    ret = encoder_reader_setup(&encoder_reader_settings, &encoder);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Encoder reader setup failed.");
        return ret;
    }
    // Start encoder reader, return error if not succesful
    ret = encoder_reader_start(encoder);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Encoder reader startup failed.");
        return ret;
    }

    // Setup task parameters and start the task
    BaseType_t x_returned;
    x_returned = xTaskCreate(encoder_handler_task, "encoder_handler_task", 2048, (void *)encoder_action_queue, 10, NULL);
    if (x_returned != pdPASS)
    {
        ESP_LOGE(TAG, "Encoder handler task creation failed.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Encoder setup finished.");
    return ESP_OK;
}