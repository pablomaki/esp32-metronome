# ESP32 metronome

The inspiration for this project was mainly to learn to use freeRTOS (and RTOS in general) and the goal was to create a metronome. 

The project consists of a ESP32 microcontroller, generic rotary encoder, 128x32 OLED screen and an output device, which in my case is a relay driven with a MOSFET. The encoder is used to select the BPM and the signature mode for the metronome while the screen is used to show this information to the user. A circuit diagram/circuit design will be published as well once done.

Each of the components; encoder, output trigger and screen is handled by a separate task that is taken care of by the freeRTOS scheduler.
