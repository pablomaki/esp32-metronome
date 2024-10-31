#ifndef ENCODER_HANDLER_H
#define ENCODER_HANDLER_H

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
void encoder_handler_task(void *arg);

/**
 * Create encoder, set it up based on given parameters and start
 *
 * @param queue Queue used by the encoder to pass ticks.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t start_encoder_handler(void);

#endif // ENCODER_HANDLER_H