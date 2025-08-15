#ifndef KEYINPUT_H
#define KEYINPUT_H

#include <Arduino.h>
#include "configs.h"  // Include centralized configuration

// Function to display a keyboard and prompt for input
const char* prompt_keyboard();

// Initialize analog inputs and button
void init_controls();

// Read potentiometer position and return movement direction
int get_x_movement();
int get_y_movement();

// Check if the select button was pressed with debouncing
bool select_button_pressed();

// Draw the keyboard interface
void draw_keyboard(uint8_t cursor_x, uint8_t cursor_y, const char* current_text);

// The keyMap might need to be accessible in other files
extern const char keyMap[6][18];

#endif // KEYINPUT_H
