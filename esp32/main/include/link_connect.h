#ifndef _VI_LINK_H
#define _VI_LINK_H
#include <driver/timer.h>
#include <ableton/Link.hpp>

#include "led_control.h" 
#include "freertos/queue.h"

#define PRINT_LINK_STATE false

namespace vi_link {

  class LinkConnect
  {
    public:
      LinkConnect(QueueHandle_t _message_q);
  };

}

#endif
