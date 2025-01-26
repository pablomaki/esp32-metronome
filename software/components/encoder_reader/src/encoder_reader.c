#include "../include/encoder_reader.h"

SemaphoreHandle_t semaphore;
static void IRAM_ATTR pin_a_isr_handler(void *arg)
{
    encoder_reader_handle_t encoder_handle = (encoder_reader_handle_t)arg;
    if (xSemaphoreTakeFromISR(semaphore, NULL) == pdTRUE)
    {
        if (!esp_timer_is_active(encoder_handle->pin_a_timer))
        {
            ESP_ERROR_CHECK(esp_timer_start_once(encoder_handle->pin_a_timer, encoder_handle->a_debounce_us));
        }
        else
        {
            ESP_ERROR_CHECK(esp_timer_restart(encoder_handle->pin_a_timer, encoder_handle->a_debounce_us));
        }
        xSemaphoreGiveFromISR(semaphore, NULL); // Release the semaphore
    }
}

static void IRAM_ATTR pin_b_isr_handler(void *arg)
{
    encoder_reader_handle_t encoder_handle = (encoder_reader_handle_t)arg;
    if (xSemaphoreTakeFromISR(semaphore, NULL) == pdTRUE)
    {
        encoder_handle->pin_b_current_value = gpio_get_level(encoder_handle->pin_b);
        if (!esp_timer_is_active(encoder_handle->pin_b_timer))
        {
            ESP_ERROR_CHECK(esp_timer_start_once(encoder_handle->pin_b_timer, encoder_handle->b_debounce_us));
        }
        else
        {
            ESP_ERROR_CHECK(esp_timer_restart(encoder_handle->pin_b_timer, encoder_handle->b_debounce_us));
        }
        xSemaphoreGiveFromISR(semaphore, NULL); // Release the semaphore
    }
}

static void IRAM_ATTR pin_sw_isr_handler(void *arg)
{
    encoder_reader_handle_t encoder_handle = (encoder_reader_handle_t)arg;
    if (xSemaphoreTakeFromISR(semaphore, NULL) == pdTRUE)
    {
        bool neg_edge = gpio_get_level(encoder_handle->pin_sw) ? false : true;
        if (neg_edge)
        {
            // Handle long press timer
            if (!esp_timer_is_active(encoder_handle->pin_sw_longpress_timer))
            {
                ESP_ERROR_CHECK(esp_timer_start_once(encoder_handle->pin_sw_longpress_timer, encoder_handle->sw_longpress_us));
            }
            else
            {
                ESP_ERROR_CHECK(esp_timer_restart(encoder_handle->pin_sw_longpress_timer, encoder_handle->sw_longpress_us));
            }
        }
        else
        {
            // Stop long press timer            
            if (esp_timer_is_active(encoder_handle->pin_sw_longpress_timer))
            {
                ESP_ERROR_CHECK(esp_timer_stop(encoder_handle->pin_sw_longpress_timer));
            }
            // Handle debounce timer
            if (!esp_timer_is_active(encoder_handle->pin_sw_timer))
            {
                ESP_ERROR_CHECK(esp_timer_start_once(encoder_handle->pin_sw_timer, encoder_handle->sw_debounce_us));
            }
            else
            {
                ESP_ERROR_CHECK(esp_timer_restart(encoder_handle->pin_sw_timer, encoder_handle->sw_debounce_us));
            }
        }
        xSemaphoreGiveFromISR(semaphore, NULL); // Release the semaphore
    }
}

static void pin_a_debounce_cb(void *arg)
{
    encoder_reader_handle_t encoder_handle = (encoder_reader_handle_t)arg;
    if (xSemaphoreTakeFromISR(semaphore, NULL) == pdTRUE)
    {
        if (gpio_get_level(encoder_handle->pin_a) == 0)
        {
            encoder_tick_t encoder_tick = {
                .direction = (encoder_handle->pin_b_value == 1) ? 1 : -1,
                .time = esp_timer_get_time()};
            xQueueSendFromISR(encoder_handle->tick_queue, &encoder_tick, NULL);
        }
        xSemaphoreGiveFromISR(semaphore, NULL); // Release the semaphore
    }
}

static void pin_b_debounce_cb(void *arg)
{
    encoder_reader_handle_t encoder_handle = (encoder_reader_handle_t)arg;
    if (xSemaphoreTakeFromISR(semaphore, NULL) == pdTRUE)
    {
        encoder_handle->pin_b_value = encoder_handle->pin_b_current_value;
        xSemaphoreGiveFromISR(semaphore, NULL); // Release the semaphore
    }
}

static void pin_sw_debounce_cb(void *arg)
{
    encoder_reader_handle_t encoder_handle = (encoder_reader_handle_t)arg;
    if (xSemaphoreTakeFromISR(semaphore, NULL) == pdTRUE)
    {
        encoder_tick_t encoder_tick = {
            .direction = 0,
            .time = esp_timer_get_time()};
        xQueueSendFromISR(encoder_handle->tick_queue, &encoder_tick, NULL);
        xSemaphoreGiveFromISR(semaphore, NULL); // Release the semaphore
    }
}

static void pin_sw_longpress_cb(void *arg)
{
    encoder_reader_handle_t encoder_handle = (encoder_reader_handle_t)arg;
    if (xSemaphoreTakeFromISR(semaphore, NULL) == pdTRUE)
    {
        encoder_tick_t encoder_tick = {
            .direction = 10,
            .time = esp_timer_get_time()};
        xQueueSendFromISR(encoder_handle->tick_queue, &encoder_tick, NULL);
        xSemaphoreGiveFromISR(semaphore, NULL); // Release the semaphore
    }
}

esp_err_t encoder_reader_setup(const encoreder_reader_settings_t *args,
                               encoder_reader_handle_t *out_handle)
{
    // Check input arguments
    if (args == NULL || out_handle == NULL || args->pin_a == 0 ||
        args->pin_b == 0 || args->pin_sw == 0 || args->a_debounce_us == 0 ||
        args->b_debounce_us == 0 || args->sw_debounce_us == 0 || args->sw_longpress_us == 0 || args->tick_queue == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Allocate memory
    encoder_reader_handle_t result = (encoder_reader_handle_t)heap_caps_calloc(1, sizeof(*result), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (result == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    // Fill the created handle and return
    result->pin_a = args->pin_a;
    result->pin_b = args->pin_b;
    result->pin_sw = args->pin_sw;
    result->a_debounce_us = args->a_debounce_us;
    result->b_debounce_us = args->b_debounce_us;
    result->sw_debounce_us = args->sw_debounce_us;
    result->sw_longpress_us = args->sw_longpress_us;
    result->tick_queue = args->tick_queue;
    *out_handle = result;

    // Create the semaphore
    semaphore = xSemaphoreCreateBinary();
    if (semaphore == NULL)
    {
        // Handle semaphore creation failure
        return ESP_ERR_NO_MEM;
    }

    // Initialize the semaphore to available state
    xSemaphoreGive(semaphore);

    return ESP_OK;
}

esp_err_t encoder_reader_start(encoder_reader_handle_t encoder_handle)
{
    // Create timers for debouncing the switches
    const esp_timer_create_args_t pin_a_debounce_timer_args = {
        .callback = &pin_a_debounce_cb,
        .arg = encoder_handle,
        .name = "pin_a_debounce_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&pin_a_debounce_timer_args, &encoder_handle->pin_a_timer));
    const esp_timer_create_args_t pin_b_debounce_timer_args = {
        .callback = &pin_b_debounce_cb,
        .arg = encoder_handle,
        .name = "pin_b_debounce_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&pin_b_debounce_timer_args, &encoder_handle->pin_b_timer));
    const esp_timer_create_args_t pin_sw_debounce_timer_args = {
        .callback = &pin_sw_debounce_cb,
        .arg = encoder_handle,
        .name = "pin_sw_debounce_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&pin_sw_debounce_timer_args, &encoder_handle->pin_sw_timer));

    // Create timer for switch long press
    const esp_timer_create_args_t pin_sw_longpress_timer_args = {
        .callback = &pin_sw_longpress_cb,
        .arg = encoder_handle,
        .name = "pin_sw_longpress_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&pin_sw_longpress_timer_args, &encoder_handle->pin_sw_longpress_timer));

    // Enable ISR service and interrupts for pins
    encoder_reader_enable(encoder_handle);
    return ESP_OK;
}

void encoder_reader_enable(encoder_reader_handle_t encoder_handle)
{    
    gpio_install_isr_service(0);
    // Configure the GPIO pins as input, with pullup
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << encoder_handle->pin_a | 1ULL << encoder_handle->pin_b | 1ULL << encoder_handle->pin_sw),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE, // Trigger on rising and falling edges
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_intr_type(encoder_handle->pin_a, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(encoder_handle->pin_a, pin_a_isr_handler, (void *)encoder_handle);
    gpio_isr_handler_add(encoder_handle->pin_b, pin_b_isr_handler, (void *)encoder_handle);
    gpio_isr_handler_add(encoder_handle->pin_sw, pin_sw_isr_handler, (void *)encoder_handle);
}

void encoder_reader_disable(encoder_reader_handle_t encoder_handle)
{
    gpio_isr_handler_remove(encoder_handle->pin_a);
    gpio_isr_handler_remove(encoder_handle->pin_b);
    gpio_isr_handler_remove(encoder_handle->pin_sw);
    gpio_uninstall_isr_service();
}