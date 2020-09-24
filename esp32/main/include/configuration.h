#pragma once
#ifndef _VI_CONFIG_H
#define _VI_CONFIG_H

#define LED GPIO_NUM_2
#define NUM_LEDS 60
#define DATA_PIN 33
#define BRIGHTNESS  80
#define LED_TYPE    WS2812
#define COLOR_ORDER BGR

static const char *TAG = "INTERPLAY";
CRGB leds[NUM_LEDS];

#endif
