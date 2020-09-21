#include "link_connect.h"

namespace vi_link {

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

      if (PRINT_LINK_STATE)
      {
        xTaskCreate(printTask, "print", 8192, &link, 1, nullptr);
      }

      while (true)
      {
        xSemaphoreTake(userParam, portMAX_DELAY);

        const auto state = link.captureAudioSessionState();
        const auto phase = state.phaseAtTime(link.clock().micros(), 1.);

        // TODO: Call out to LED Control

        portYIELD();
      }
    }

    LinkConnect::LinkConnect() {

      BaseType_t xReturned;
      TaskHandle_t xHandle = NULL;

      SemaphoreHandle_t tickSemphr = xSemaphoreCreateBinary();
      timerGroup0Init(100, tickSemphr);
      //xTaskCreate(tickTask, "tick", 8192, tickSemphr, configMAX_PRIORITIES - 1, nullptr);
      xReturned = xTaskCreate(tickTask, "tick", 8192, tickSemphr, configMAX_PRIORITIES - 1, &xHandle);
      if( xReturned != pdPASS )
      {
        //ErrorHandler();
        printf("ERROR1!\n");
      }
    }

}
