#include "animation.h"

#include <iostream>

namespace vi_anim {

  void Character::set_position(float dv, float sb, uint8_t eb) {
    position = {dv, position.current_value, position.current_value, sb, eb};
  }

  void Character::set_size(float dv, float sb, uint8_t eb) {
    size = {dv, size.current_value, size.current_value, sb, eb};
  }

/*
    float dest_value;
    float start_value;
    float current_value;
    float start_beat;
    uint8_t dest_beat;
*/
  void Character::update(float current_beat) {
    // Update the LFB, based on size and position.
    if (current_beat <= position.start_beat + position.dest_beat) {
	    float beat_percentage = ((current_beat - position.start_beat) / (float) position.dest_beat);
	    position.current_value = position.start_value + (position.dest_value - position.start_value) * beat_percentage; 
	}
	else {
		position.current_value = position.dest_value;
	}

	if (current_beat <= size.start_beat + size.dest_beat) {
		float beat_percentage = ((current_beat - size.start_beat) / (float) size.dest_beat);
		size.current_value = size.start_value + (size.dest_value - size.start_value) * beat_percentage; 
	}
	else {
		size.current_value = size.dest_value;
	}



    int local_position = (NUM_LEDS-1) * position.current_value;
    int local_size = (NUM_LEDS-1) * size.current_value;
    //std::cout << "pos: " << position.current_value << " | size: " << size.current_value << " | curr beat: " << current_beat << std::endl;
    std::cout 	<< "pos: " << position.dest_value  << " | " << position.start_value  << " | " << position.current_value  << " | " 
    			<< position.start_beat  << " | " << (int)position.dest_beat << std::endl;

    int lower = floor(local_position - local_size / 2.f);
    int upper = floor(local_position + local_size / 2.f);

    std::cout << "local pos " << local_position << " -- loc size " << local_size << " - lower " << lower << " - upper " << upper << std::endl;

    for (int i = 0; i < NUM_LEDS;  i++) {

      lfb[i] = CRGB::Black;

      if (i >= lower && i <= upper) {
        lfb[i] = CRGB::Red;
        std::cout << i << std::endl;
      } 

    }

  }

  Character::Character() {
    size = {0.f,0.f,0.f,0.f,(uint8_t)1};
    position = {0.f,0.f,0.f,0.f,(uint8_t)1};
  }

}
