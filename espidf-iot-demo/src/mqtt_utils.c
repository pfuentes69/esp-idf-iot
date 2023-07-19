/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"



#include "wifi_utils.h"
#include "mqtt_utils.h"

static const char *TAG = "mqtt_utils";


esp_mqtt_client_handle_t client;

void mqtt_publish(char *topic, char *message)
{
    esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
}

void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}



void mqtt_app_launch(esp_mqtt_client_config_t *mqtt_cfg, void *mqtt_event_handler, bool auto_start)
{

    client = esp_mqtt_client_init(mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    if (auto_start)
        esp_mqtt_client_start(client);
}

void mqtt_app_start(void)
{
    esp_mqtt_client_start(client);
}

void mqtt_app_stop(void)
{
    esp_mqtt_client_stop(client);
}
