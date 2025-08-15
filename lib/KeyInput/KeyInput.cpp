#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "KeyInput.h"
#include "configs.h"

// Global display object
extern Adafruit_SSD1306 display;

// Static password buffer
static char password_buffer[50] = "";

// Movement timing variables
static unsigned long last_move_time = 0;

const char keyMap[6][18] = {
    {REMOVE_CHAR, LEFT_CHAR, RIGHT_CHAR, 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O'},
    {'P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g'},
    {'h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y'},
    {'z','0','1','2','3','4','5','6','7','8','9','!','?','@','#','$','%','^'},
    {'&','*','(',')','-','_','+','=','|','\\',':',';','"','\'','<','>',',','.'},
    {'/','~','`','[',']','{','}',SPACE_CHAR,' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
};

// Initialize potentiometers and button
void init_controls() {
    pinMode(BTN_SELECT, INPUT_PULLUP);
    
    // Set ADC resolution to 12 bits (0-4095)
    analogReadResolution(12);
    
    // Enable ADC for potentiometer pins
    analogSetPinAttenuation(POT_X_PIN, ADC_11db);  // For 0-3.3V range
    analogSetPinAttenuation(POT_Y_PIN, ADC_11db);
}

// Read horizontal potentiometer and return movement (-1, 0, 1)
int get_x_movement() {
    static int last_zone = 1;  // 0=left, 1=neutral, 2=right
    static unsigned long last_zone_time = 0;
    
    int pot_value = analogRead(POT_X_PIN);
    int current_zone;
    
    // Define zones with hysteresis to prevent jitter
    if(pot_value < 1200) {          // 0-29% = LEFT zone
        current_zone = 0;
    } else if(pot_value > 2895) {   // 71-100% = RIGHT zone  
        current_zone = 2;
    } else {                        // 29-71% = NEUTRAL zone
        current_zone = 1;
    }
    
    // Only register movement when entering a new zone
    if(current_zone != last_zone) {
        unsigned long current_time = millis();
        
        // Rate limiting: minimum 150ms between zone changes
        if(current_time - last_zone_time > 150) {
            last_zone_time = current_time;
            
            int movement = 0;
            if(last_zone == 1 && current_zone == 0) {
                movement = -1;  // Neutral → Left = move left
            } else if(last_zone == 1 && current_zone == 2) {
                movement = 1;   // Neutral → Right = move right
            } else if(last_zone == 0 && current_zone == 2) {
                movement = 1;   // Left → Right = move right
            } else if(last_zone == 2 && current_zone == 0) {
                movement = -1;  // Right → Left = move left
            }
            
            last_zone = current_zone;
            return movement;
        }
    }
    
    return 0;  // No movement
}

// Read vertical potentiometer and return movement (-1, 0, 1)
int get_y_movement() {
    static int last_zone = 1;  // 0=up, 1=neutral, 2=down
    static unsigned long last_zone_time = 0;
    
    int pot_value = analogRead(POT_Y_PIN);
    int current_zone;
    
    // Define zones with hysteresis
    if(pot_value < 1200) {          // 0-29% = UP zone
        current_zone = 0;
    } else if(pot_value > 2895) {   // 71-100% = DOWN zone
        current_zone = 2;
    } else {                        // 29-71% = NEUTRAL zone
        current_zone = 1;
    }
    
    // Only register movement when entering a new zone
    if(current_zone != last_zone) {
        unsigned long current_time = millis();
        
        if(current_time - last_zone_time > 150) {
            last_zone_time = current_time;
            
            int movement = 0;
            if(last_zone == 1 && current_zone == 0) {
                movement = -1;  // Neutral → Up = move up
            } else if(last_zone == 1 && current_zone == 2) {
                movement = 1;   // Neutral → Down = move down
            } else if(last_zone == 0 && current_zone == 2) {
                movement = 1;   // Up → Down = move down
            } else if(last_zone == 2 && current_zone == 0) {
                movement = -1;  // Down → Up = move up
            }
            
            last_zone = current_zone;
            return movement;
        }
    }
    
    return 0;  // No movement
}

// Button debouncing for select button
bool select_button_pressed() {
    static unsigned long last_press_time = 0;
    
    if(digitalRead(BTN_SELECT) == LOW) {
        unsigned long current_time = millis();
        if(current_time - last_press_time > 200) { // 200ms debounce
            last_press_time = current_time;
            return true;
        }
    }
    return false;
}

// Check if enough time has passed for movement
bool can_move() {
    unsigned long current_time = millis();
    if(current_time - last_move_time > MOVE_DELAY) {
        last_move_time = current_time;
        return true;
    }
    return false;
}

// Draw the keyboard interface
void draw_keyboard(uint8_t cursor_x, uint8_t cursor_y, const char* current_text) {
    display.clearDisplay();
    
    // Draw title
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Enter Password:");
    
    // Draw current input text (line 2)
    display.setCursor(0, 10);
    display.print("> ");
    display.println(current_text);
    
    // Draw keyboard starting from line 3 (y=20)
    for(int row = 0; row < 6; row++) {
        for(int col = 0; col < 18; col++) {
            int x = col * 7;  // 7 pixels per character
            int y = 20 + (row * 8);  // 8 pixels per row, starting at y=20
            
            // Skip if position would be off screen
            if(x >= SCREEN_WIDTH - 6) break;
            if(y >= SCREEN_HEIGHT - 8) break;
            
            char ch = keyMap[row][col];
            if(ch == 0 || ch == SPACE_CHAR) {
                if(ch == SPACE_CHAR) ch = '_';  // Show space as underscore
                else continue;
            }
            
            // Highlight cursor position
            if(row == cursor_y && col == cursor_x) {
                display.fillRect(x-1, y-1, 8, 10, SSD1306_WHITE);
                display.setTextColor(SSD1306_BLACK);
            } else {
                display.setTextColor(SSD1306_WHITE);
            }
            
            display.setCursor(x, y);
            
            // Handle special characters
            if(ch == REMOVE_CHAR) {
                display.print("DEL");
            } else if(ch == LEFT_CHAR) {
                display.print("<");
            } else if(ch == RIGHT_CHAR) {
                display.print(">");
            } else {
                display.print(ch);
            }
        }
    }
    
    // Draw instructions at bottom
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 56);
    display.print("Move:Pots Sel:Button");
    
    display.display();
}

const char* prompt_keyboard() {
    init_controls();
    
    // Clear password buffer
    memset(password_buffer, 0, sizeof(password_buffer));
    uint8_t text_pos = 0;
    uint8_t cursor_x = 3;  // Start at 'A'
    uint8_t cursor_y = 0;
    
    // Show initial keyboard
    draw_keyboard(cursor_x, cursor_y, password_buffer);
    
    while(true) {
        // Handle potentiometer movement
        if(can_move()) {
            int x_move = get_x_movement();
            int y_move = get_y_movement();
            
            if(x_move != 0 || y_move != 0) {
                // Update cursor position
                if(x_move == -1) {
                    cursor_x = (cursor_x == 0) ? 17 : cursor_x - 1;  // Wrap to right
                } else if(x_move == 1) {
                    cursor_x = (cursor_x == 17) ? 0 : cursor_x + 1;  // Wrap to left
                }
                
                if(y_move == -1) {
                    cursor_y = (cursor_y == 0) ? 5 : cursor_y - 1;   // Wrap to bottom
                } else if(y_move == 1) {
                    cursor_y = (cursor_y == 5) ? 0 : cursor_y + 1;   // Wrap to top
                }
                
                // Redraw keyboard with new cursor position
                draw_keyboard(cursor_x, cursor_y, password_buffer);
            }
        }
        
        // Handle button press
        if(select_button_pressed()) {
            char selected_char = keyMap[cursor_y][cursor_x];
            
            if(selected_char == REMOVE_CHAR) {
                // Delete last character
                if(text_pos > 0) {
                    text_pos--;
                    password_buffer[text_pos] = '\0';
                }
            } else if(selected_char == LEFT_CHAR) {
                // Special case: finish input (like BACK button)
                break;
            } else if(selected_char == RIGHT_CHAR) {
                // Special case: add space
                if(text_pos < 49) {
                    password_buffer[text_pos] = ' ';
                    text_pos++;
                    password_buffer[text_pos] = '\0';
                }
            } else if(selected_char != 0 && text_pos < 49) {
                // Add character to password
                if(selected_char == SPACE_CHAR) {
                    password_buffer[text_pos] = ' ';
                } else {
                    password_buffer[text_pos] = selected_char;
                }
                text_pos++;
                password_buffer[text_pos] = '\0';
            }
            
            // Redraw with updated text
            draw_keyboard(cursor_x, cursor_y, password_buffer);
        }
        
        delay(10);  // Small delay to prevent excessive polling
    }
    
    return password_buffer;
}
