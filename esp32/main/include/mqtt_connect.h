#pragma once
#ifndef _VI_MQTT_H
#define _VI_MQTT_H
#include <string>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"
#include "configuration.h"

// TODO: Move to menuconfig
#define CONFIG_BROKER_URL "ws://192.168.0.193:9001"

namespace vi_mqtt {

  //void init(QueueHandle_t _message_q);
  
  class MQTTConnect {
    public:
      QueueHandle_t message_q;
      MQTTConnect();
  };

}
#endif
