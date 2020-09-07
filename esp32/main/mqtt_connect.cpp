#include "mqtt_connect.h"

namespace vi_mqtt {

  QueueHandle_t message_q;
  /**** MQTT ***/
  static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
  {
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
      case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        //msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/point/#", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;
      case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
      case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
      case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
      case MQTT_EVENT_DATA:
        {
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        std::string vi_topic(event->topic, event->topic_len);
        if (vi_topic.compare("/point/0/size/value") == 0) {
          float received_data = atof(event->data);
          //xQueueSend(message_q, ( void * ) &received_data, ( TickType_t ) 10);
          printf("Found topic %s with data %f\n", vi_topic.c_str(), received_data);
          printf("In other words: %.*s\n", event->data_len, event->data);
        }
        else if (vi_topic.compare("/point/0/size/func") == 0) {
          std::string size_func(event->data, event->data_len);
          printf("Found topic %s with data %s\n", vi_topic.c_str(), size_func.c_str());
        }
        else if (vi_topic.compare("/point/0/pos/value") == 0) {
          float position = atof(event->data);
          xQueueSend(message_q, ( void * ) &position, ( TickType_t ) 10);
          printf("Found topic %s with data %f\n", vi_topic.c_str(), position);
          printf("In other words: %.*s\n", event->data_len, event->data);
        }
        else 
            printf("No match found for TOPIC=%.*s with DATA=%.*s\r\n", event->topic_len, event->topic, event->data_len, event->data);
        break;
        }
      case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
      default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
  }

    /*
     static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
     ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
     mqtt_event_handler_cb(event_data);
     }
     */

  void mqtt_app_start(void)
  {
    esp_mqtt_client_config_t mqtt_cfg = {
      .uri = CONFIG_BROKER_URL,
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
      int count = 0;
      printf("Please enter url of mqtt broker\n");
      while (count < 128) {
        int c = fgetc(stdin);
        if (c == '\n') {
          line[count] = '\0';
          break;
        } else if (c > 0 && c < 127) {
          line[count] = c;
          ++count;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
      mqtt_cfg.uri = line;
      printf("Broker url: %s\n", line);
    } else {
      ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
      abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    mqtt_cfg.event_handle = mqtt_event_handler_cb;

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    //esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
  }

  void init(QueueHandle_t _message_q) {
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
    message_q = _message_q;
    mqtt_app_start();
  }
}
