/*
 * mqtt.h
 *
 *  Created on: 29 Aug 2024
 *      Author: duvan
 */

// Adafruit IO credentials
#define AIO_USERNAME "An3003"
#define AIO_KEY "aio_Lkvn76hhmzFfLY056UenudISdQEw"

void mqtt_publish_data(const char *topic, const char *data);
/**
 * Starts the MQTT task
 */
void MQTT_task_start(void);