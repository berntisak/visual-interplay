#include "link_connect.h"

namespace vi_link {

    vi_mqtt::MQTTConnect *mqtt;
    CRGB leds[NUM_LEDS];

    float value = 0.0f;
    uint8_t pos = 0;

    unsigned int if_nametoindex(const char* ifName)
    {
      return 0;
    }

    char* if_indextoname(unsigned int ifIndex, char* ifName)
    {
      return nullptr;
    }

    void IRAM_ATTR timer_group0_isr(void* userParam)
    {
      static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

      TIMERG0.int_clr_timers.t0 = 1;
      TIMERG0.hw_timer[0].config.alarm_en = 1;

      xSemaphoreGiveFromISR(userParam, &xHigherPriorityTaskWoken);
      if (xHigherPriorityTaskWoken)
      {
        portYIELD_FROM_ISR();
      }
    }

    void timerGroup0Init(int timerPeriodUS, void* userParam)
    {
      timer_config_t config = {.alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_EN,
        .divider = 80};

      timer_init(TIMER_GROUP_0, TIMER_0, &config);
      timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
      timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, timerPeriodUS);
      timer_enable_intr(TIMER_GROUP_0, TIMER_0);
      timer_isr_register(TIMER_GROUP_0, TIMER_0, &timer_group0_isr, userParam, 0, nullptr);

      timer_start(TIMER_GROUP_0, TIMER_0);
    }

    void printTask(void* userParam)
    {
      auto link = static_cast<ableton::Link*>(userParam);
      const auto quantum = 4.0;

      while (true)
      {
        const auto sessionState = link->captureAppSessionState();
        const auto numPeers = link->numPeers();
        const auto time = link->clock().micros();
        const auto beats = sessionState.beatAtTime(time, quantum);

        std::cout << std::defaultfloat << "| peers: " << numPeers << " | "
          << "tempo: " << sessionState.tempo() << " | " << std::fixed
          << "beats: " << beats << " |" << std::endl;
        std::cout << std::defaultfloat << "| phase: " << std::endl;

        vTaskDelay(800 / portTICK_PERIOD_MS);
      }
    }

    void tickTask(void* userParam)
    {
      ableton::Link link(120.0f);
      link.enable(true);
      const auto quantum = 4.0;

      link.setTempoCallback([](double bpm) {
        std::cout << "new tempo: " << bpm << std::endl;
      });

      if (PRINT_LINK_STATE)
      {
        xTaskCreate(printTask, "print", 8192, &link, 1, nullptr);
      }

      int beats_last = 0;
      int beats_next = 0;
      uint8_t rand_pos = 0;

      long prev_time = 0;
      uint8_t pos = (NUM_LEDS-1);
      bool updateLED = false;
      long time_between_beats = (60000 / 120*4) * 1000;
      long next_beat_in_time = 0;
      long time_for_each_led = 0;
      int led_idx = 0;
      
      while (true)
      {
        xSemaphoreTake(userParam, portMAX_DELAY);

        const auto state = link.captureAudioSessionState();
        const auto time = link.clock().micros();
        const auto beats = state.beatAtTime(time, quantum);
        const auto phase = state.phaseAtTime(link.clock().micros(), 1.);

        beats_last = floor(beats);

        if (beats_last == beats_next) {
          FastLED.clear();
          rand_pos = floor((NUM_LEDS-1) * ((float) std::rand() / RAND_MAX)*0.5);

          time_between_beats = time.count() - prev_time;  
          next_beat_in_time = time.count() + time_between_beats;
          int lighting_leds = 10; 
          time_for_each_led = (time_between_beats / lighting_leds) * 4;

          prev_time = time.count();  
          led_idx = 0;
          updateLED = true;
        }


        if (time.count() >= prev_time + (time_for_each_led*led_idx)) {
          updateLED = true; 
          //std::cout << "Turning on LED nr" << (int)rand_pos1+led_idx << " at time " << time.count() << " - next led at " << prev_time + (time_for_each_led*(led_idx+1)) << std::endl;
        }

        if( xQueueReceive( mqtt->message_q, &( value ), ( TickType_t ) 1) == pdPASS )
        {
            pos *= value;
            updateLED = true;
        }

        beats_next = ceil(beats);

        if (updateLED) {
          leds[rand_pos + led_idx] = CRGB::Red;
          led_idx += 1;
          FastLED.show();

          updateLED = false;
        }

        //portYIELD();
      }
    }

    LinkConnect::LinkConnect(vi_mqtt::MQTTConnect *_mqtt) {

      mqtt = _mqtt;

      FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(leds, NUM_LEDS);
      FastLED.setBrightness(BRIGHTNESS);

      BaseType_t xReturned;
      TaskHandle_t xHandle = NULL;

      SemaphoreHandle_t tickSemphr = xSemaphoreCreateBinary();
      timerGroup0Init(100, tickSemphr);
      //xTaskCreate(tickTask, "tick", 8192, tickSemphr, configMAX_PRIORITIES - 1, nullptr);
      //
      xReturned = xTaskCreate(tickTask, "tick", 8192, tickSemphr, configMAX_PRIORITIES - 1, &xHandle);
      if( xReturned != pdPASS )
      {
        printf("ERROR1!\n");
      }
    }

}
