#ifndef SCROLLINGTEXT_H
#define SCROLLINGTEXT_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

class ScrollingText {
private:
  String text;
  String display_text;
  int display_width;           // Maximum characters to display
  int pixel_width;            // Pixel width of display area
  int scroll_position;        // Current scroll position (in characters)
  unsigned long last_scroll_time;
  unsigned long scroll_delay;  // Delay between scroll steps (ms)
  unsigned long pause_delay;   // Pause at start/end (ms)
  unsigned long pause_start_time;
  bool is_paused;
  bool scroll_direction;       // true = right to left, false = left to right
  bool needs_scrolling;
  bool loop_enabled;
  
  // Smooth scrolling variables
  int pixel_offset;            // Current pixel offset for smooth scrolling
  int pixels_per_char;         // Pixels per character (usually 6 for size 1)
  bool smooth_scroll_enabled;
  
  void updateDisplayText();
  void calculateScrollNeeds();
  
public:
  // Constructor
  ScrollingText(int max_chars = 20, int pixel_w = 120, unsigned long scroll_ms = 150, unsigned long pause_ms = 1000);
  
  // Configuration methods
  void setText(const String& new_text);
  void setDisplayWidth(int chars, int pixels = -1);
  void setScrollDelay(unsigned long delay_ms);
  void setPauseDelay(unsigned long pause_ms);
  void enableLoop(bool enable = true);
  void enableSmoothScroll(bool enable = true, int pixels_per_char = 6);
  void setScrollDirection(bool right_to_left = true);
  
  // Control methods
  void reset();
  void pause();
  void resume();
  void update();  // Call this regularly to update scroll position
  
  // Display methods
  String getCurrentDisplayText();
  void draw(Adafruit_SSD1306* display, int x, int y, int text_size = 1, uint16_t color = SSD1306_WHITE);
  void drawWithBackground(Adafruit_SSD1306* display, int x, int y, int text_size = 1, 
                         uint16_t text_color = SSD1306_WHITE, uint16_t bg_color = SSD1306_BLACK);
  
  // Status methods
  bool isScrolling() const;
  bool needsScrolling() const;
  int getScrollPosition() const;
  String getOriginalText() const;
};

#endif // SCROLLINGTEXT_H
