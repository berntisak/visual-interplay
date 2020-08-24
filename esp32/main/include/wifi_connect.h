#pragma once
#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_log.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>
#include <protocol_examples_common.h>

#include "configuration.h"

// TODO: Move to menuconfig
#define EXAMPLE_ESP_WIFI_SSID "Skynet"
#define EXAMPLE_ESP_WIFI_PASS "hemmelig"

#define EXAMPLE_ESP_MAXIMUM_RETRY  5

#define CONFIG_ESP_SYSTEM_PANIC_GDBSTUB true

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

namespace vi_wifi {

  void init(void);

}

