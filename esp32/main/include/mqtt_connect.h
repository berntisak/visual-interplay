#pragma once
#include <string>
#include <stdlib.h>
#include "mqtt_client.h"
#include "esp_log.h"
#include "freertos/queue.h"

#include "configuration.h"

// TODO: Move to menuconfig
#define CONFIG_BROKER_URL "mqtt://192.168.0.193"

namespace vi_mqtt {

  void init(QueueHandle_t _message_q);

}
