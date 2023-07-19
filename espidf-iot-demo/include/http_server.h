#ifndef H_HTTP_SERVER
#define H_HTTP_SERVER

#include "wifi_utils.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>

#ifdef __cplusplus
extern "C" {
#endif

httpd_handle_t start_webserver(void);
esp_err_t stop_webserver(httpd_handle_t server);

#ifdef __cplusplus
}
#endif


#endif