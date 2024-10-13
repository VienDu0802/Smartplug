/*
 * wifi_app.c
 *
 *  Created on: 10 Jul 2024
 *      Author: duvan
 */

#include "nvs_flash.h"
#include "wifi_reset_button.h"
#include "wifi_app.h"
#include "mqtt.h"
#include "schedule.h"
#include "freertos/task.h"
#include "http_server.h"

void app_main(void)
{
    // Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Start Wifi
	wifi_app_start();

	MQTT_task_start();
	
	get_schedule_from_firebase();

	char version[64];
    esp_err_t err = get_firmware_version_from_nvs(version, sizeof(version));
	erase_firmware_version_from_nvs();
	write_firmware_version_to_nvs("1.0.0");
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Ghi phiên bản ban đầu
        printf("Version not found, writing default version 1.0.0\n");
        write_firmware_version_to_nvs("1.0.0");
    } else if (err != ESP_OK) {
        printf("Error reading version: %s\n", esp_err_to_name(err));
    } else {
        printf("Current version: %s\n", version);
    }
}
