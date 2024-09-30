/*
 * tasks_common.h
 *
 *  Created on: 10 Jul 2024
 *      Author: duvan
 */

#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

// WiFi application task
#define WIFI_APP_TASK_STACK_SIZE			4096
#define WIFI_APP_TASK_PRIORITY				5
#define WIFI_APP_TASK_CORE_ID				0

// HTTP Server task
#define HTTP_SERVER_TASK_STACK_SIZE			8192
#define HTTP_SERVER_TASK_PRIORITY			4
#define HTTP_SERVER_TASK_CORE_ID			0

// HTTP Server Monitor task
#define HTTP_SERVER_MONITOR_STACK_SIZE		4096
#define HTTP_SERVER_MONITOR_PRIORITY		3
#define HTTP_SERVER_MONITOR_CORE_ID			0

// Wifi Reset Button task
#define WIFI_RESET_BUTTON_TASK_STACK_SIZE   2048
#define WIFI_RESET_BUTTON_TASK_PRIORITY     6
#define WIFI_RESET_BUTTON_TASK_CORE_ID      0    

// HLW8032 task
#define HLW8032_APP_TASK_STACK_SIZE			4096
#define HLW8032_APP_TASK_PRIORITY			7
#define HLW8032_APP_TASK_CORE_ID			0

// Get data schedule from firebase task
#define GET_SCHEDULE_FROM_FIREBASE_TASK_STACK_SIZE			4096
#define GET_SCHEDULE_FROM_FIREBASE_PRIORITY			        4
#define GET_SCHEDULE_FROM_FIREBASE_TASK_CORE_ID			    0

#endif /* MAIN_TASKS_COMMON_H_ */
