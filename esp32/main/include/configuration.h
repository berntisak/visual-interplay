#pragma once
#ifndef _VI_CONFIG_H
#define _VI_CONFIG_H

#include "led_control.h"

#define LED GPIO_NUM_2
#define NUM_LEDS 60
#define DATA_PIN 33
#define BRIGHTNESS  80
#define LED_TYPE    WS2812
#define COLOR_ORDER BGR

static const char *TAG = "INTERPLAY";

#endif
