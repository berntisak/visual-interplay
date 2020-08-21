#include <ableton/Link.hpp>
#include <driver/gpio.h>
#include <driver/timer.h>

#include "esp_event.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_netif.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include <nvs_flash.h>
#include <protocol_examples_common.h>

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "FastLED.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"


static const char *TAG = "MQTT_LINK_1";

#define CONFIG_BROKER_URL "mqtt://192.168.0.193"

#define EXAMPLE_ESP_WIFI_SSID      "Skynet"
#define EXAMPLE_ESP_WIFI_PASS      "hemmelig"


#define EXAMPLE_ESP_MAXIMUM_RETRY  5

#define CONFIG_ESP_SYSTEM_PANIC_GDBSTUB true 

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

//static const char *TAG = "wifi station";

static int s_retry_num = 0;


CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 IRAM_ATTR myRedWhiteBluePalette_p;

#include "palettes.h"

#define LED GPIO_NUM_2
#define PRINT_LINK_STATE false

#define NUM_LEDS 60
#define DATA_PIN 33 
#define BRIGHTNESS  80
#define LED_TYPE    WS2812
#define COLOR_ORDER BGR
CRGB leds[NUM_LEDS];

/**** MQTT ***/
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/*
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}
*/

static void mqtt_app_start(void)
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


/**** WiFi ***/

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));


    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.sta.ssid, EXAMPLE_ESP_WIFI_SSID);
    strcpy((char*)wifi_config.sta.password,EXAMPLE_ESP_WIFI_PASS);
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    wifi_config.sta.pmf_cfg = {
                .capable = true,
                .required = false
            };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}


/**** FastLED ***/

void ChangePalettePeriodically(){

  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
    if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
    if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
    if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
    if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
    if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
    if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
    if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
    if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
    if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
    if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
  }

}

#define N_COLORS 5
CRGB colors[N_COLORS] = { 
  CRGB::Black,
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue,
  CRGB::White
};

auto gPhase = 0.0f;

void blinkLeds_interesting(void *pvParameters){
  while(1){
    printf("blink leds\n");
    ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, startIndex, 64, currentBlending);
        startIndex += 3;
    }
    printf("show leds\n");
    FastLED.show();
    delay(400);
  };

};

void linkBlink(void *pvParameters) {
    FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    bool hasTriggered = false;
    int cycle = 0;

    while(1) {
      auto trigger = gPhase < 0.1;
      if (trigger && !hasTriggered) {
        hasTriggered = true;
        // test with some random numbers
        
        leds[0] = colors[cycle];
        cycle++;
        if (cycle >= 3)
          cycle = 0;
        
        printf("Trigger!\n");
        FastLED.show();
      }
      else if (!trigger) {
        hasTriggered = false;
      }
      portYIELD();
    };
};

/*********************/


unsigned int if_nametoindex(const char* ifName)
{
  return 0;
}

char* if_indextoname(unsigned int ifIndex, char* ifName)
{
  return nullptr;
}

void IRAM_ATTR timer_group0_isr(void* userParam)
{
  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  TIMERG0.int_clr_timers.t0 = 1;
  TIMERG0.hw_timer[0].config.alarm_en = 1;

  xSemaphoreGiveFromISR(userParam, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken)
  {
    portYIELD_FROM_ISR();
  }
}

void timerGroup0Init(int timerPeriodUS, void* userParam)
{
  timer_config_t config = {.alarm_en = TIMER_ALARM_EN,
    .counter_en = TIMER_PAUSE,
    .intr_type = TIMER_INTR_LEVEL,
    .counter_dir = TIMER_COUNT_UP,
    .auto_reload = TIMER_AUTORELOAD_EN,
    .divider = 80};

  timer_init(TIMER_GROUP_0, TIMER_0, &config);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, timerPeriodUS);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_register(TIMER_GROUP_0, TIMER_0, &timer_group0_isr, userParam, 0, nullptr);

  timer_start(TIMER_GROUP_0, TIMER_0);
}

void printTask(void* userParam)
{
  auto link = static_cast<ableton::Link*>(userParam);
  const auto quantum = 4.0;

  while (true)
  {
    const auto sessionState = link->captureAppSessionState();
    const auto numPeers = link->numPeers();
    const auto time = link->clock().micros();
    const auto beats = sessionState.beatAtTime(time, quantum);

    std::cout << std::defaultfloat << "| peers: " << numPeers << " | "
              << "tempo: " << sessionState.tempo() << " | " << std::fixed
              << "beats: " << beats << " |" << std::endl;
    std::cout << std::defaultfloat << "| phase: " << gPhase << std::endl;       
      
    vTaskDelay(800 / portTICK_PERIOD_MS);
  }
}

void tickTask(void* userParam)
{
  ableton::Link link(120.0f);
  link.enable(true);

  if (PRINT_LINK_STATE)
  {
    xTaskCreate(printTask, "print", 8192, &link, 1, nullptr);
  }

  
  FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  bool hasTriggered = false;
  int cycle = 0;
  
  //xTaskCreate(linkBlink, "linkBlink", 8192, &link, configMAX_PRIORITIES - 2, nullptr);

  while (true)
  {
    xSemaphoreTake(userParam, portMAX_DELAY);

    const auto state = link.captureAudioSessionState();
    const auto phase = state.phaseAtTime(link.clock().micros(), 1.);
    //gPhase = phase;

    // This shit works!!!!
    auto trigger = phase < 0.1;

    if (trigger && !hasTriggered) {
      hasTriggered = true;

      /*
      // test with some random numbers
      leds[0] = colors[cycle];
      cycle++;
      if (cycle >= 3)
        cycle = 0;
      */

      ChangePalettePeriodically();
      
      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; /* motion speed */
      
      for( int i = 0; i < NUM_LEDS; i++) {
          leds[i] = ColorFromPalette( currentPalette, startIndex, 64, currentBlending);
          startIndex += 3;
      }

      FastLED.show();
    }
    else if (!trigger) {
      hasTriggered = false;
    }

    portYIELD();
    }
}

extern "C" void app_main()
{
  
  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
  esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
  //ESP_ERROR_CHECK(example_connect());
  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

  //tcpip_adapter_init();
  //ESP_ERROR_CHECK(esp_event_loop_create_default());
  //ESP_ERROR_CHECK(example_connect());

  wifi_init_sta();
  //xTaskCreatePinnedToCore(wifi_init_sta, "wifi", 4000, NULL, 5, NULL, 1);
  /**/

  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;
  
  SemaphoreHandle_t tickSemphr = xSemaphoreCreateBinary();
  timerGroup0Init(100, tickSemphr);

  printf("starting Link");

  //xTaskCreate(tickTask, "tick", 8192, tickSemphr, configMAX_PRIORITIES - 1, nullptr);
  xReturned = xTaskCreate(tickTask, "tick", 8192, tickSemphr, configMAX_PRIORITIES - 1, &xHandle);
  if( xReturned != pdPASS )
  {
    //ErrorHandler();
    printf("ERROR1!\n");
  }

  printf(" set max power\n");
  // I have a 2A power supply, although it's 12v
  //FastLED.setMaxPowerInVoltsAndMilliamps(12,2000);

  // change the task below to one of the functions above to try different patterns
  printf("create task for led blinking\n");
  //xReturned = xTaskCreate(linkBlink, "linkBlink", 8192, nullptr, configMAX_PRIORITIES - 2, &xHandle);
  //xTaskCreatePinnedToCore(&linkBlink, "blinkLeds", 4000, nullptr, 5, nullptr, 0);
  //xTaskCreatePinnedToCore(&blinkLeds_interesting, "blinkLeds", 4000, NULL, configMAX_PRIORITIES - 2, NULL, 0);
  //xTaskCreate(&blinkLeds_interesting, "blinkLeds", 4000, NULL, 5, NULL);
  //wifi_init_sta();
  if( xReturned != pdPASS )
  {
    //ErrorHandler();
    printf("ERROR2!\n");
  }

  mqtt_app_start();

}
