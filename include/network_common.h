#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_netif.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
/* home
#define MYSSID "Racoon&Masters"
#define MYPWD "gotomunich2019"
#define MYMQTT "mqtt://192.168.178.42:1883"
//*/
///*Professor's room
#define MYSSID "CAPS"
#define MYPWD "caps!schulz-wifi"
#define MYMQTT "mqtt://131.159.85.131:1883"
//*/
/* my phone
#define MYSSID "JOJO!"
#define MYPWD "11111111"
#define MYMQTT "mqtt://192.168.43.197:1883"
//*/
/* seminar room
#define MYSSID "capslab"
#define MYPWD "12356789"
#define MYMQTT "mqtt://172.20.10.4:1883"
//*/
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define MQTT_CONNECTED_BIT BIT0
#define MQTT_ERROR_BIT BIT1

struct mqtt_msg
{
    char topic[20];
    char msg[20];
};

/**
 * @brief start wifi, reconnect when disconnect
 * @return ESP_OK when success
 */
esp_err_t start_wifi(void);
/**
 * @brief start mqtt
 * @param a pointer to client handle (pointer to pointer)
 * @return ESP_OK on success
 */
esp_err_t start_mqtt(esp_mqtt_client_handle_t *client);
/**
 * @brief publish message with QoS 1
 * @param client the cilent handle
 * @param topic a string
 * @param data a string
 */
void mqtt_send(esp_mqtt_client_handle_t client, const char *topic, const char *data);
void mqtt_listen(esp_mqtt_client_handle_t client, const char *topic, QueueHandle_t *msg_q);
