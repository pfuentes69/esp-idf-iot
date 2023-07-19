#ifndef H_MQTT_UTILS
#define H_MQTT_UTILS

#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

void mqtt_app_launch(esp_mqtt_client_config_t *mqtt_cfg, void *mqtt_event_handler, bool auto_start);
void mqtt_app_start(void);
void mqtt_app_stop(void);

void mqtt_publish(char *topic, char *message);
void log_error_if_nonzero(const char *message, int error_code);

#ifdef __cplusplus
}
#endif

#endif