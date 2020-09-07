#include "led_control.h"

namespace vi_led {

  QueueHandle_t message_q;
  CRGBPalette16 currentPalette;
  TBlendType currentBlending;

  extern CRGBPalette16 myRedWhiteBluePalette;
  extern const TProgmemPalette16 IRAM_ATTR myRedWhiteBluePalette_p;

#include "palettes.h"

  CRGB leds[NUM_LEDS];

  void ChangePalettePeriodically(){

    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if( lastSecond != secondHand) {
      lastSecond = secondHand;
      if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
      if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
      if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
      if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
      if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
      if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
      if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
      if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
      if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
      if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
      if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }

  }

  const int N_COLORS = 5;
  CRGB colors[N_COLORS] = {
    CRGB::Black,
    CRGB::Red,
    CRGB::Green,
    CRGB::Blue,
    CRGB::White
  };

  auto gPhase = 0.0f;

  void blinkLeds_interesting(void *pvParameters){
    while(1){
      printf("blink leds\n");
      ChangePalettePeriodically();

      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; /* motion speed */

      for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, startIndex, 64, currentBlending);
        startIndex += 3;
      }
      printf("show leds\n");
      FastLED.show();
      delay(400);
    };

  };

  void linkBlink(void *pvParameters) {
    FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    int cycle = 0;
    bool hasTriggered = false;

    while(1) {
      auto trigger = gPhase < 0.1;
      if (trigger && !hasTriggered) {
        hasTriggered = true;
        // test with some random numbers

        leds[0] = colors[cycle];
        cycle++;
        if (cycle >= 3)
          cycle = 0;

        printf("Trigger!\n");
        FastLED.show();
      }
      else if (!trigger) {
        hasTriggered = false;
      }
      portYIELD();
    };
  };

  void trigger_light(float phase) {
    bool hasTriggered = false;

    auto trigger = phase < 0.1;

    if (trigger && !hasTriggered) {
      hasTriggered = true;

      ChangePalettePeriodically();

      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; /* motion speed */

      for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, startIndex, 64, currentBlending);
        startIndex += 3;
      }

      FastLED.show();
    }
    else if (!trigger) {
      hasTriggered = false;
    }
  }

  void init(QueueHandle_t _message_q) {
    message_q = _message_q;
    FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    /*
    while(1) {
      float value;
      if( xQueueReceive( message_q, &( value ), ( TickType_t ) 1) == pdPASS )
      {
          printf("Got: %f\n", value);
      }
    }*/

  }

}
