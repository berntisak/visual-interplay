#pragma once
#ifndef _VI_LINK_H
#define _VI_LINK_H
#include <ableton/Link.hpp>
#include <chrono>
#include <driver/timer.h>
#include <freertos/queue.h>

#include "mqtt_connect.h"
#include "led_control.h" 

#define PRINT_LINK_STATE false

namespace vi_link {

  class LinkConnect
  {
    public:
      LinkConnect(vi_mqtt::MQTTConnect *_mqtt);
  };

}

#endif
