#ifndef SCREEN_HANDLER_H
#define SCREEN_HANDLER_H

/**
 * Populate input array with indexes for correct images to show on screen based on bpm and signature mode
 *
 * @param arr Array to populate.
 * @return void.
 */
void get_indexes(uint16_t *arr);

/**
 * Based on if bpm is selected decide if screen should blink and calculate frames for blinking effect
 *
 * @param void.
 * @return bool, true if screen should be dim
 */
bool is_screen_dim();

/**
 * Handle encoder ticks and modify the bpm or signature based on them
 *
 * @param void.
 * @return void.
 */
void screen_update_handler_task(void *arg);

/**
 * Convert bitmaps to images
 *
 * @param uint8_t segment_display[][192] segment display array.
 * @param uint8_t *segment_image segment image array.
 * @param size_t image_count
 * @return esp_err_t return fail in case anything fails during conversion.
 */
esp_err_t conver_bitmap_to_image(uint8_t segment_display[][192], uint8_t *segment_image, size_t image_count);

/**
 * Setup the screen and the load the images to buffer
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t start_screen_handler(void);

#endif // SCREEN_HANDLER_H