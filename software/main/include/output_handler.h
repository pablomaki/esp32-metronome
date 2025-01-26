#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

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
void output_handler_task(void *arg);

/**
 * Setup output driver timer and start it.
 *
 * @param void.
 * @return esp_err_t return fail in case anything fails during startup.
 */
esp_err_t start_output_handler(void);

#endif // OUTPUT_HANDLER_H