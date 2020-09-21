#include "esp_system.h"
#include <nvs_flash.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Local

#include "configuration.h"
#include "wifi_connect.h"
#include "mqtt_connect.h"
#include "link_connect.h"

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

  QueueHandle_t message_q = xQueueCreate( 10, sizeof(float) );

  vi_wifi::init();
  vi_mqtt::init(message_q);
  auto start_link = new vi_link::LinkConnect(message_q);

}
