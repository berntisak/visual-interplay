#include "animation.h"

namespace vi_anim {

  void Character::set_position(float v, float sb, uint8_t eb) {
    position = {v, sb, eb, position.current_value, position.current_value};
  }

  void Character::set_size(float v, float sb, uint8_t eb) {
    size = {v, sb, eb, size.current_value, size.current_value};
  }

  void Character::update(float current_beat) {
    // Update the LFB, based on size and position.
    
    position.current_value = position.start_value + (position.dest_value - position.start_value) * ((current_beat - position.start_beat) / (float) position.dest_beat); 
    size.current_value = size.start_value + (size.dest_value - size.start_value) * ((current_beat - size.start_beat) / (float) size.dest_beat); 

    int position = NUM_LEDS * position.current_value;
    int size = NUM_LEDS * size.current_value;
    
    for (int i = 0; i < NUM_LEDS;  i++) {

      lfb[i] = CRGB::black;

      if (i >= position - size / 2 && i <= position + size / 2) {
        lfb[i] = CRGB::red;
      } 

    }

  }

  Character::Character() {
    size = {0,0,0,0}
    position = {0,0,0,0}
  }

}
