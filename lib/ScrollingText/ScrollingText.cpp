#include "ScrollingText.h"

ScrollingText::ScrollingText(int max_chars, int pixel_w, unsigned long scroll_ms, unsigned long pause_ms) {
  display_width = max_chars;
  pixel_width = pixel_w;
  scroll_delay = scroll_ms;
  pause_delay = pause_ms;
  
  // Initialize state
  scroll_position = 0;
  pixel_offset = 0;
  last_scroll_time = 0;
  pause_start_time = 0;
  is_paused = false;
  scroll_direction = true;  // Right to left by default
  needs_scrolling = false;
  loop_enabled = true;
  smooth_scroll_enabled = true;
  pixels_per_char = 6;  // Default for text size 1
  
  text = "";
  display_text = "";
}

void ScrollingText::setText(const String& new_text) {
  text = new_text;
  reset();
  calculateScrollNeeds();
  updateDisplayText();
}

void ScrollingText::setDisplayWidth(int chars, int pixels) {
  display_width = chars;
  if (pixels > 0) {
    pixel_width = pixels;
  }
  calculateScrollNeeds();
  updateDisplayText();
}

void ScrollingText::setScrollDelay(unsigned long delay_ms) {
  scroll_delay = delay_ms;
}

void ScrollingText::setPauseDelay(unsigned long pause_ms) {
  pause_delay = pause_ms;
}

void ScrollingText::enableLoop(bool enable) {
  loop_enabled = enable;
}

void ScrollingText::enableSmoothScroll(bool enable, int pixels_per_char) {
  smooth_scroll_enabled = enable;
  if (pixels_per_char > 0) {
    this->pixels_per_char = pixels_per_char;
  }
}

void ScrollingText::setScrollDirection(bool right_to_left) {
  scroll_direction = right_to_left;
}

void ScrollingText::reset() {
  scroll_position = 0;
  pixel_offset = 0;
  last_scroll_time = millis();
  pause_start_time = millis();
  is_paused = true;  // Start with pause
}

void ScrollingText::pause() {
  is_paused = true;
  pause_start_time = millis();
}

void ScrollingText::resume() {
  is_paused = false;
  last_scroll_time = millis();
}

void ScrollingText::calculateScrollNeeds() {
  needs_scrolling = (text.length() > display_width);
}

void ScrollingText::updateDisplayText() {
  if (!needs_scrolling) {
    display_text = text;
    return;
  }
  
  if (smooth_scroll_enabled) {
    // For smooth scrolling, we need to show partial characters
    int char_start = scroll_position;
    int char_end = scroll_position + display_width + 1;  // +1 for partial character
    
    if (char_end > text.length()) {
      if (loop_enabled) {
        // Wrap around with loop
        display_text = text.substring(char_start) + " " + text.substring(0, char_end - text.length());
      } else {
        display_text = text.substring(char_start);
      }
    } else {
      display_text = text.substring(char_start, char_end);
    }
  } else {
    // Character-by-character scrolling
    int char_end = scroll_position + display_width;
    
    if (char_end > text.length()) {
      if (loop_enabled) {
        display_text = text.substring(scroll_position) + " " + text.substring(0, char_end - text.length());
      } else {
        display_text = text.substring(scroll_position);
      }
    } else {
      display_text = text.substring(scroll_position, char_end);
    }
  }
}

void ScrollingText::update() {
  if (!needs_scrolling) {
    return;
  }
  
  unsigned long current_time = millis();
  
  // Handle pause at start/end
  if (is_paused) {
    if (current_time - pause_start_time >= pause_delay) {
      is_paused = false;
      last_scroll_time = current_time;
    }
    return;
  }
  
  // Check if it's time to scroll
  if (current_time - last_scroll_time < scroll_delay) {
    return;
  }
  
  if (smooth_scroll_enabled) {
    // Smooth pixel-by-pixel scrolling
    pixel_offset += 2;  // Move 2 pixels at a time for smooth effect
    
    if (pixel_offset >= pixels_per_char) {
      pixel_offset = 0;
      scroll_position++;
      
      // Check for end of text
      if (scroll_position >= text.length()) {
        if (loop_enabled) {
          scroll_position = 0;
          is_paused = true;
          pause_start_time = current_time;
        } else {
          scroll_position = text.length() - display_width;
          is_paused = true;
          pause_start_time = current_time;
        }
      }
    }
  } else {
    // Character-by-character scrolling
    scroll_position++;
    
    if (scroll_position >= text.length()) {
      if (loop_enabled) {
        scroll_position = 0;
        is_paused = true;
        pause_start_time = current_time;
      } else {
        scroll_position = text.length() - display_width;
        is_paused = true;
        pause_start_time = current_time;
      }
    }
  }
  
  last_scroll_time = current_time;
  updateDisplayText();
}

String ScrollingText::getCurrentDisplayText() {
  return display_text;
}

void ScrollingText::draw(Adafruit_SSD1306* display, int x, int y, int text_size, uint16_t color) {
  display->setTextSize(text_size);
  display->setTextColor(color);
  display->setCursor(x - pixel_offset, y);  // Apply smooth scroll offset
  
  if (needs_scrolling) {
    display->print(display_text);
  } else {
    display->print(text);
  }
}

void ScrollingText::drawWithBackground(Adafruit_SSD1306* display, int x, int y, int text_size, 
                                     uint16_t text_color, uint16_t bg_color) {
  // Clear background area first
  int char_width = 6 * text_size;
  int char_height = 8 * text_size;
  int bg_width = display_width * char_width;
  
  display->fillRect(x, y, bg_width, char_height, bg_color);
  
  // Draw text on top
  display->setTextSize(text_size);
  display->setTextColor(text_color);
  display->setCursor(x - pixel_offset, y);
  
  if (needs_scrolling) {
    display->print(display_text);
  } else {
    display->print(text);
  }
}

bool ScrollingText::isScrolling() const {
  return needs_scrolling && !is_paused;
}

bool ScrollingText::needsScrolling() const {
  return needs_scrolling;
}

int ScrollingText::getScrollPosition() const {
  return scroll_position;
}

String ScrollingText::getOriginalText() const {
  return text;
}
