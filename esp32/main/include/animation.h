#pragma once
#ifndef _VI_ANIMATION_H
#define _VI_ANIMATION_H

#include "configuration.h"

namespace vi_anim {

  struct Param {
    float dest_value;
    float start_beat;
    uint8_t dest_beat;
    float start_value;
    float current_value;
  }

  class Character {

    Param position;
    Param size;

    public:
      CRGB lfb[NUM_LEDS];

      Character();

      void set_position(float v, float sb, uint8_t eb);
      void set_size(float v, float sb, uint8_t eb);

      void update(float current_beats);

  };

}

#endif
