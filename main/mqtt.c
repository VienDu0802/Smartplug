/*
 * mqtt.c
 *
 *  Created on: 29 Aug 2024
 *      Author: duvan
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "lwip/netdb.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "mqtt_client.h"

// #include "app_nvs.h"
// #include "tasks_common.h"
// #include "wifi_app.h"
// #include "http_server.h"
#include "mqtt.h"
#include "firebase.h"

// CONNECT TO MQTT
static const char *TAG = "MQTT";

uint32_t MQTT_CONNEECTED = 0;
esp_mqtt_client_handle_t client = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        MQTT_CONNEECTED = 1;

        msg_id = esp_mqtt_client_subscribe(client, "An3003/feeds/nutnhan1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        MQTT_CONNEECTED=0;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
//        printf("From TOPIC = %.*s\r\n", event->topic_len, event->topic);
//        printf("Receive DATA = %.*s\r\n", event->data_len, event->data);

        if(strncmp(event->topic, "An3003/feeds/nutnhan1", event->topic_len) == 0){
			if (strncmp(event->data, "1", event->data_len) == 0){
				gpio_set_level(RELAY_GPIO_PIN, 1);
				printf("RELAY: On\n");
			} else if (strncmp(event->data, "0", event->data_len) == 0) {
				gpio_set_level(RELAY_GPIO_PIN, 0); 
				printf("RELAY: Off\n");
			}
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_publish_data(const char *topic, const char *data) {
    if (MQTT_CONNEECTED) {
        int msg_id = esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
        ESP_LOGI(TAG, "Publish message, msg_id=%d", msg_id);
    } else {
        ESP_LOGI(TAG, "MQTT is not connected, can't publish data");
    }
}

static void mqtt_app_start(void)
{
	ESP_LOGI(TAG, "STARTING MQTT");
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://io.adafruit.com:1883",
		.credentials.username = AIO_USERNAME,
		.credentials.authentication.password = AIO_KEY,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void MQTT_task_start(void) {
    mqtt_app_start();
}



