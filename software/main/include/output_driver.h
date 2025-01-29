#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

#include "driver/gptimer.h"

// Output driver struct
struct output_dirver
{
    TaskHandle_t output_task_handle;
    gptimer_handle_t gp_timer;
    QueueHandle_t output_activation_queue;
};

/**
 * @brief Opaque type representing a single encoder_reader
 */
typedef struct output_dirver *output_driver_handle_t;

/**
 * Click the output x times
 *
 * @param long_click. If the click should have longer period between on & off of the relay
 * @param led_on. Bool for turning the led on for the duration or not
 * @return void.
 */
void click(bool long_click, bool led_on);

/**
 * Output handler, activate output in case the flag is set
 *
 * @param arg Arguments passed to the event.
 * @return void.
 */
void output_driver_task(void *arg);

/**
 * Setup output driver timer and start it.
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t setup_output_devices(output_driver_handle_t output_handle);

/**
 * start output driver timer and start it.
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t start_output_devices(output_driver_handle_t output_handle);

/**
 * stop output driver timer and start it.
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t stop_output_devices(output_driver_handle_t output_handle);

#endif // OUTPUT_HANDLER_H