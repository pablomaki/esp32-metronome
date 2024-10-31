
#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdint.h>

#define SIGNATURE_IMAGES 3
#define NUMBER_IMAGES 10

extern uint16_t signature_modes[SIGNATURE_IMAGES];
extern uint8_t segment_display_signatures[SIGNATURE_IMAGES][192];
extern uint8_t segment_display_numbers[NUMBER_IMAGES][192];

#endif // RESOURCES_H