/*
#include <driver/gpio.h>
#include <driver/timer.h>

#include "esp_event.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>
#include <protocol_examples_common.h>

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
*/

#include "esp_system.h"
#include <nvs_flash.h>
#include "esp_log.h"

// Local

#include "configuration.h"
#include "wifi_connect.h"
#include "link_connect.h"
#include "mqtt_connect.h"
#include "led_control.h"

extern "C" void app_main()
{

  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  esp_log_level_set("*", ESP_LOG_INFO);

  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  vi_wifi::init();
  vi_link::init();
  vi_led::init();
  vi_mqtt::init();

}
