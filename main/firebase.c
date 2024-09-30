#include "firebase.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "time_sync.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "tasks_common.h"
#include "mqtt.h"
static const char *TAG = "firebase";
extern const uint8_t certificate_pem_start[] asm("_binary_certificate_pem_start");
extern const uint8_t certificate_pem_end[] asm("_binary_certificate_pem_end");

void firebase_init(void)
{
    gpio_set_direction(RELAY_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY_GPIO_PIN, 0); 
}

bool is_day_in_repeat(const char *repeat_days, const char *current_day) 
{
    return strstr(repeat_days, current_day) != NULL;
}

// Hàm xử lý HTTP events
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  
    static int output_len;       

    if (evt->event_id == HTTP_EVENT_ERROR) {
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
    } else if (evt->event_id == HTTP_EVENT_ON_CONNECTED) {
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
    } else if (evt->event_id == HTTP_EVENT_HEADER_SENT) {
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
    } else if (evt->event_id == HTTP_EVENT_ON_HEADER) {
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
    } else if (evt->event_id == HTTP_EVENT_ON_DATA) {
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client)) {
            if (output_buffer == NULL) {
                output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                output_len = 0;
                if (output_buffer == NULL) {
                    ESP_LOGE(TAG, "Không thể cấp phát bộ nhớ cho output buffer");
                    return ESP_FAIL;
                }
            }
            memcpy(output_buffer + output_len, evt->data, evt->data_len);
            output_len += evt->data_len;
        }
    } else if (evt->event_id == HTTP_EVENT_ON_FINISH) {
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL) {
            ESP_LOGI(TAG, "HTTP Response: %s", output_buffer);
            cJSON *json = cJSON_Parse(output_buffer);
            if (json != NULL) {
                ESP_LOGI(TAG, "Phân tích JSON thành công");

                char current_time[64];
                char current_day[16];
                get_current_time(current_time, current_day);  
                
                for (int i = 0; current_day[i]; i++) {
                    current_day[i] = toupper(current_day[i]);
                }
                ESP_LOGI(TAG, "Current time: %s", current_time);
                ESP_LOGI(TAG, "Current day: %s", current_day);
                cJSON *schedule;
                cJSON_ArrayForEach(schedule, json) {
                    cJSON *name = cJSON_GetObjectItem(schedule, "name");
                    cJSON *time = cJSON_GetObjectItem(schedule, "time");
                    cJSON *repeat = cJSON_GetObjectItem(schedule, "repeat");
                    cJSON *action = cJSON_GetObjectItem(schedule, "action");

                    if (name && time && repeat && action) {
                        ESP_LOGI(TAG, "Tên lịch: %s, Thời gian: %s, Lặp lại: %s, Hành động: %s",
                                name->valuestring, time->valuestring, repeat->valuestring, action->valuestring);

                        if (strcmp(time->valuestring, current_time) == 0 && 
                            is_day_in_repeat(repeat->valuestring, current_day)) {
                            if (strcmp(action->valuestring, "ON") == 0) {
                                gpio_set_level(RELAY_GPIO_PIN, 1);  
                                ESP_LOGI(TAG, "Bật thiết bị theo lịch: %s", name->valuestring);
                                mqtt_publish_data("An3003/feeds/nutnhan1", "1"); // Gửi dữ liệu "1" lên topic
                            } else if (strcmp(action->valuestring, "OFF") == 0) {
                                gpio_set_level(RELAY_GPIO_PIN, 0); 
                                ESP_LOGI(TAG, "Tắt thiết bị theo lịch: %s", name->valuestring);
                                mqtt_publish_data("An3003/feeds/nutnhan1", "0"); // Gửi dữ liệu "1" lên topic
                            }
                        }
                    }
                }
                cJSON_Delete(json);
            } else {
                ESP_LOGE(TAG, "Không thể phân tích JSON");
            }

            free(output_buffer);
            output_buffer = NULL;
        }
    } else if (evt->event_id == HTTP_EVENT_DISCONNECTED) {
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        if (output_buffer != NULL) {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
    }
    return ESP_OK;
}

// Hàm lấy và kiểm tra lịch từ Firebase
void check_schedule_and_execute(void)
{
    esp_http_client_config_t config = {
        .url = "https://smartplug-b7468-default-rtdb.firebaseio.com/schedules.json",
        .buffer_size = 2048,
        .method = HTTP_METHOD_GET,
        .cert_pem = (const char *)certificate_pem_start,
        .timeout_ms = 5000,
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Yêu cầu HTTP thất bại với lỗi: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void get_schedule_from_firebase_task(void *pvParameters){
    obtain_time();

    firebase_init();

    while (true) {
        check_schedule_and_execute();
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void get_schedule_from_firebase(void){
    xTaskCreatePinnedToCore(&get_schedule_from_firebase_task, "get_schedule_from_firebase", GET_SCHEDULE_FROM_FIREBASE_TASK_STACK_SIZE, NULL, GET_SCHEDULE_FROM_FIREBASE_PRIORITY, NULL, GET_SCHEDULE_FROM_FIREBASE_TASK_CORE_ID);
}