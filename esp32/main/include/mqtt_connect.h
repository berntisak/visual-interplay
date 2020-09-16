#pragma once
#include <string>
#include <stdlib.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"

#include "mqtt_client.h"

#include "configuration.h"

// TODO: Move to menuconfig
#define CONFIG_BROKER_URL "mqtt://192.168.0.193"

namespace vi_mqtt {

  void init(QueueHandle_t _message_q);

}
