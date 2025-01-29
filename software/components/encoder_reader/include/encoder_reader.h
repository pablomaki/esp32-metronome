#include "sys/queue.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "freertos/semphr.h"

struct encoder_reader
{
    uint64_t pin_a;
    uint64_t pin_b;
    uint64_t pin_sw;
    uint64_t a_debounce_us;
    uint64_t b_debounce_us;
    uint64_t sw_debounce_us;
    uint64_t sw_longpress_us;
    uint8_t pin_b_value;
    uint8_t pin_b_current_value;
    QueueHandle_t tick_queue;
    esp_timer_handle_t pin_a_timer;
    esp_timer_handle_t pin_b_timer;
    esp_timer_handle_t pin_sw_timer;
    esp_timer_handle_t pin_sw_longpress_timer;
    void *arg;
    // LIST_ENTRY(encoder_reader)
    // list_entry;
};

/**
 * @brief Opaque type representing a single encoder_reader
 */
typedef struct encoder_reader *encoder_reader_handle_t;

/**
 * @brief Encoder state change type, keeps track of time as well
 */
typedef struct
{
    uint64_t time;
    int8_t direction;
} encoder_tick_t;

/**
 * @brief Encoder reader configuration passed to encoder_reader_create
 */
typedef struct
{
    uint64_t pin_a;
    uint64_t pin_b;
    uint64_t pin_sw;
    uint64_t a_debounce_us;
    uint64_t b_debounce_us;
    uint64_t sw_debounce_us;
    uint64_t sw_longpress_us;
    QueueHandle_t tick_queue; //!< Function to call when tick to either direction happens
} encoreder_reader_settings_t;

/**
 * @brief Create an encoder reader instance
 *
 * @param args   Pointer to a structure with timer creation arguments.
 *                      Not saved by the library, can be allocated on the stack.
 * @param[out] out_handle  Output, pointer to encoder_reader_handle_t variable which
 *                         will hold the created timer handle.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if some of the create_args are not valid
 *      - ESP_ERR_INVALID_STATE if encoder_reader library is not initialized yet
 *      - ESP_ERR_NO_MEM if memory allocation fails
 */
esp_err_t encoder_reader_setup(const encoreder_reader_settings_t *args,
                               encoder_reader_handle_t *out_handle);

/**
 * @brief Start an encoder reader instance
 *
 * @param[out] encoder_handle  Output, pointer to encoder_reader_handle_t variable which
 *                         will hold the created timer handle.
 *
 * @return
 *      - ESP_OK on success
 */
esp_err_t encoder_reader_enable(encoder_reader_handle_t encoder_handle);

/**
 * @brief Disable an encoder reader instance
 *
 * @param[out] encoder_handle  Output, pointer to encoder_reader_handle_t variable which
 *                         will hold the created timer handle.
 *
 * @return void
 */
void encoder_reader_disable(encoder_reader_handle_t encoder_handle);