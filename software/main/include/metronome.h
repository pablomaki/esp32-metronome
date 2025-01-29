#ifndef METRONOME_H
#define METRONOME_H

#include "esp_err.h"

/**
 * Handle sleep state for esp
 *
 * @param void.
 * @return void.
 */
void handle_sleep_state_task(void* arg);

/**
 * Setup the metronome
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t enable_peripherals();

/**
 * Setup the metronome
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t disable_peripherals();

/**
 * Setup the metronome
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t setup_peripherals();

/**
 * Setup the metronome
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t start_metronome(void);

#endif // METRONOME_H