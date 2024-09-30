#include "time_sync.h"
#include "esp_sntp.h"
#include "esp_log.h"

static const char *TAG = "time_sync";

// Hàm đồng bộ thời gian qua NTP
void obtain_time(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "0.vn.pool.ntp.org");
    esp_sntp_init();

    // Chờ thời gian đồng bộ
    time_t now = 0;
    struct tm timeinfo = { 0 };
    while (timeinfo.tm_year < (2024 - 1900)) {
        ESP_LOGI(TAG, "Waiting for system time to be set...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    setenv("TZ", "ICT-7", 1);
    tzset();
    ESP_LOGI(TAG, "Time synchronized");
}

void get_current_time(char *time_buffer, char *day_buffer)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    strftime(time_buffer, 64, "%H:%M", &timeinfo);
    
    strftime(day_buffer, 16, "%a", &timeinfo);
}




