#ifndef ENCODER_H
#define ENCODER_H

#include "encoder_reader.h"
#include "freertos/FreeRTOS.h"
#include "settings.h"
#include "esp_log.h"

// Struct to simplify encoder actions
typedef struct
{
    uint64_t prev_tick_time;
    uint8_t consecutive_ticks;
    int8_t direction;
} action_t;

// Function type for handling sleep state
typedef esp_err_t (*sleep_handler_func_t)(void);

// Encoder struct
struct encoder
{
    encoder_reader_handle_t encoder_reader_handle;
    TaskHandle_t encoder_task_handle;
    QueueHandle_t sleep_request_queue_handle;
};

/**
 * @brief Opaque type representing a single encoder_reader
 */
typedef struct encoder *encoder_handle_t;

/**
 *
 * Handle select button click
 *
 * @param int8_t previous direction
 * @param encoder_tick_t* tick
 * @return void.
 */
void handle_select(int8_t prev_direction, encoder_tick_t *tick);

/**
 *
 * Handle up/down action
 *
 * @param int8_t previous direction
 * @param encoder_tick_t* tick
 * @param action_t* initiated action.
 * @return void.
 */
int8_t handle_up_down(int8_t prev_direction, encoder_tick_t *tick, action_t *action);

/**
 *
 * Handle encoder ticks and modify the bpm or signature based on them
 *
 * @param arg Arguments passed to the event.
 * @return void.
 */
void encoder_task(void *arg);

/**
 * Create encoder, set it up based on given parameters and start
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t setup_encoder(encoder_handle_t encoder);

/**
 * Start encoder
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t start_encoder(encoder_handle_t encoder);

/**
 * Stop encoder
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t stop_encoder(encoder_handle_t encoder);


#endif // ENCODER_H