#pragma once
#ifndef _VI_LED_H
#define _VI_LED_H

#include "FastLED.h"

#define LED GPIO_NUM_2
#define NUM_LEDS 60
#define DATA_PIN 33
#define BRIGHTNESS  80
#define LED_TYPE    WS2812
#define COLOR_ORDER BGR

/*
namespace vi_led {

  class LedControl {
    public:
      CRGB leds[NUM_LEDS];
      LedControl();
  };

}
*/
#endif
