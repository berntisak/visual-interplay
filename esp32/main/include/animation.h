#pragma once
#ifndef _VI_ANIMATION_H
#define _VI_ANIMATION_H

#include "configuration.h"
#include "led_control.h"

namespace vi_anim {

  struct Param {
    float value;
    uint8_t dest_time;
  }

  class Character {

    Param position;
    Param size;

    public:
      CRGB lfb[NUM_LEDS];

      Character();

      void set_position(float v, uint8_t b);
      void set_size(float v, uint8_t b);

      void update();

  };

}

#endif
