#ifndef SHARED_VARIABLES_H
#define SHARED_VARIABLES_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * Initialize the semaphores
 *
 * @param void
 * @return esp_err_t fail if creation of mutexes fails.
 */
esp_err_t init_semaphores(void);

/**
 * Change signature mode
 *
 * @param void
 * @return void.
 */
void change_signature_mode(void);

/**
 * Return current chosen signature mode
 *
 * @param void
 * @return : current signature mode.
 */
uint16_t get_signature_mode(void);

/**
 * Change the bpm candidate by bpm_delta but keep the bpm within limits of 1 and 999
 *
 * @param uint16_t bpm_delta : Change to apply to the candidate bpm
 * @return void.
 */
void change_bpm(uint16_t bpm_delta);

/**
 * Select the candidate bpm as the current selected bpm
 *
 * @param void
 * @return void.
 */
void select_bpm(void);

/**
 * Return the current selected bpm
 *
 * @param void
 * @return selected bpm.
 */
uint16_t get_selected_bpm(void);

/**
 * Return the current candidate bpm
 *
 * @param void
 * @return candidate bpm.
 */
uint16_t get_candidate_bpm(void);

/**
 * Reset the candidate bpm to selected bpm
 *
 * @param void
 * @return candidate bpm.
 */
void reset_candidate_bpm(void);

/**
 * Is the candidate equal to the selected bpm
 *
 * @param void
 * @return candidate bpm == selected bpm.
 */
bool bpm_selcted(void);

#endif // SHARED_VARIABLES_H