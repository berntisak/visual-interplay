#pragma once

#include "FastLED.h"
#include "freertos/queue.h"

#include "configuration.h"

#define LED GPIO_NUM_2
#define NUM_LEDS 60
#define DATA_PIN 33
#define BRIGHTNESS  80
#define LED_TYPE    WS2812
#define COLOR_ORDER BGR

namespace vi_led {

  void init(QueueHandle_t _message_q);

}
