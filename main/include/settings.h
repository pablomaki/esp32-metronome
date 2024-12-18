#ifndef SETTINGS_H
#define SETTINGS_H

// ******* PINSETUP *******
// ENCODER
#define ENC_A_PIN 4
#define ENC_B_PIN 5
#define ENC_SW_PIN 18
// SCREEN
#define SSD1306_SDA_PIN 22
#define SSD1306_SCL_PIN 23
#define SSD1306_RST_PIN -1
// OUTPUT
#define OUTPUT_PIN 2
#define LED_PIN 15

// ******* OTHER SETTINGS *******
// OUTPUT
#define OUTPUT_ACTIVATION_DURATION 100 // milliseconds
#define BPM_START 80                   // BPM to start with
#define SIGNATURE_START 0              // index

// INPUT
#define FAST_CHANGE_MULTIPLIER 5
#define DOUBLE_CLICK_US 5E5       // microseconds
#define FAST_CHANGE_US 1E5        // microseconds
#define FAST_CHANGE_EXPIRE_US 1E6 // microseconds
#define ENC_A_DEBOUNCE 1000       // microseconds
#define ENC_B_DEBOUNCE 1000       // microseconds
#define ENC_SW_DEBOUNCE 100000    // microseconds

#endif // SETTINGS_H