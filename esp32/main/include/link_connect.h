#pragma once
#ifndef _VI_LINK_H
#define _VI_LINK_H
#include <ableton/Link.hpp>
#include <driver/timer.h>
#include <freertos/queue.h>

#include "led_control.h" 

#define PRINT_LINK_STATE false

namespace vi_link {

  class LinkConnect
  {
    public:
      LinkConnect(QueueHandle_t _message_q);
  };

}

#endif
