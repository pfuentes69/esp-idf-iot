#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "http_utils.h"
#include "est_utils.h"

static const char *TAG = "EST_UTILS";


const char PEM_CERT_PREFIX[] = "-----BEGIN CERTIFICATE-----\n";
const char PEM_CERT_SUFIX[] = "\n-----END CERTIFICATE-----";


// Global variables to limit function memory size
char pp7_data[MAX_HTTP_OUTPUT_BUFFER] = {0};
char post_data[MAX_HTTP_OUTPUT_BUFFER] = {0};
char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
int content_length = 0;
bool result = false;
int status_code = 0;
int i, j;

bool est_cacerts(char *est_host, int est_port, char *root_cert, char *ca_cert)
{
    esp_http_client_config_t config = {
//        .url = "https://wksa-est.certifyiddemo.com/.well-known/est/cacerts",
        .host = est_host,
        .path = "/.well-known/est/cacerts",
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = root_cert,
        .port = est_port,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET Request
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) {
                status_code = esp_http_client_get_status_code(client);
                if (status_code == 200) {
                    output_buffer[data_read] = '\0';
                    strcpy(ca_cert, PEM_CERT_PREFIX);
                    strcat(ca_cert, output_buffer);
                    // Remove final \n if there's one
                    if (ca_cert[strlen(ca_cert) - 1] == '\n')
                        ca_cert[strlen(ca_cert) - 1] = '\0';
                    strcat(ca_cert, PEM_CERT_SUFIX);
                    result = true;
                } else {
                    ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRIu64,
                    status_code,
                    esp_http_client_get_content_length(client));
                    ESP_LOG_BUFFER_HEX(TAG, output_buffer, data_read);
                    output_buffer[data_read] = '\0';
                    printf("%s\n", output_buffer);
                }
            } else {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return result;
}


bool est_simpleenroll(char *est_host, int est_port, char *root_cert, char *bauth_us, char *bauth_pw, char* csr_data, char *device_cert)
{
    esp_http_client_config_t config = {
//        .url = "https://90af2231057f4505a92185e134d3fbe4:d847f238c8e1453cadc1ddf0f6ba491d@wksa-est.certifyiddemo.com/.well-known/est/simpleenroll",        
        .host = est_host,
        .path = "/.well-known/est/simpleenroll",
        .auth_type = HTTP_AUTH_TYPE_BASIC,
        .username = bauth_us,
        .password = bauth_pw,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = root_cert,
        .port = est_port,
        .timeout_ms = 10000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // POST Request
    strcpy(post_data, csr_data);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/pkcs10");
    esp_err_t err = esp_http_client_open(client, strlen(post_data));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, post_data, strlen(post_data));
        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) {
                status_code = esp_http_client_get_status_code(client);
                if (status_code == 200) {
                    output_buffer[data_read] = '\0';
                    strcpy(device_cert, output_buffer);
                    result = true;
                } else {
                    ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %"PRIu64,
                    status_code,
                    esp_http_client_get_content_length(client));
                    ESP_LOG_BUFFER_HEX(TAG, output_buffer, strlen(output_buffer));
                }
            } else {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return result;
}

bool est_simpleenroll_certauth(char *est_host, int est_port, char *root_cert, char *auth_cert, char *auth_key, char* csr_data, char *device_cert)
{
    esp_http_client_config_t config = {
//        .url = "https://90af2231057f4505a92185e134d3fbe4:d847f238c8e1453cadc1ddf0f6ba491d@wksa-est.certifyiddemo.com/.well-known/est/simpleenroll",        
        .host = est_host,
        .path = "/.well-known/est/simpleenroll",
//        .auth_type = HTTP_AUTH_TYPE_BASIC,
        .client_cert_pem = auth_cert,
        .client_key_pem = auth_key,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = root_cert,
        .port = est_port,
        .timeout_ms = 10000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // POST Request
    strcpy(post_data, csr_data);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/pkcs10");
    esp_err_t err = esp_http_client_open(client, strlen(post_data));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, post_data, strlen(post_data));
        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) {
                status_code = esp_http_client_get_status_code(client);
                if (status_code == 200) {
                    output_buffer[data_read] = '\0';
                    strcpy(device_cert, output_buffer);
                    result = true;
                } else {
                    ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %"PRIu64,
                    status_code,
                    esp_http_client_get_content_length(client));
                    ESP_LOG_BUFFER_HEX(TAG, output_buffer, strlen(output_buffer));
                }
            } else {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return result;
}

bool est_convert_p7_to_pem(char *device_cert)
{
    esp_http_client_config_t config = {
//        .url = "http://hotline.retrobyt.es/opensslp7",
        .host = "hotline.retrobyt.es",
        .path = "/opensslp7",
        .auth_type = HTTP_AUTH_TYPE_BASIC,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
        .port = 1880,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // P7 input

    strcpy(pp7_data, device_cert);

    // Clean input data
    j = 0;
    for (i = 0; i <= strlen(pp7_data); i++) {
        if ((pp7_data[i] != ' ') && (pp7_data[i] != '\n') && (pp7_data[i] != '\r'))
            post_data[j++] = pp7_data[i];
    }
    post_data[j] = '\0';
    
    // POST Request
//    strcpy(post_data, "MIIBvAYJKoZIhvcNAQcCoIIBrTCCAakCAQExADALBgkqhkiG9w0BBwGgggGRMIIBjTCCATOgAwIBAgIDB0kRMAoGCCqGSM49BAMCMBcxFTATBgNVBAMTDGVzdEV4YW1wbGVDQTAeFw0yMzA0MjExMzU1MDNaFw0yNDA0MjAxMzU1MDNaMCsxFzAVBgNVBAMMDmF3cy1kZXZpY2UtMTAxMRAwDgYDVQQFEwcwMDAxMDAxMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEFb38Z3J2Ejr6Rng1o7xVlifSRC6naDO96Z37szeuWOFuSD2DE3aUoeEUyyTpG5ArYGj/ywPWS/yEeBKakxbeFqNaMFgwCQYDVR0TBAIwADALBgNVHQ8EBAMCB4AwHQYDVR0OBBYEFDwv7/63+eLWe5ngjb5r2VvII1RPMB8GA1UdIwQYMBaAFBrfOYTCVuZszyq0JqX9DNJD9T0+MAoGCCqGSM49BAMCA0gAMEUCIAoLFEhCEb8Fn9cm9DNyiH6DMz3Va7uoNrWAjY9Xs/nAAiEA3VJUHqKEqt/jtfiTk50lP0qKuok2E7MHW06IaIcnm+YxAA==");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/raw");
    esp_err_t err = esp_http_client_open(client, strlen(post_data));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, post_data, strlen(post_data));
        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        } else {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0) {
                status_code = esp_http_client_get_status_code(client);
                if (status_code == 200) {
                    output_buffer[data_read] = '\0';
                    strcpy(device_cert, output_buffer);
                    result = true;
                } else {
                    ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRIu64,
                    status_code,
                    esp_http_client_get_content_length(client));
                    ESP_LOG_BUFFER_HEX(TAG, output_buffer, data_read);
                }
            } else {
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return result;
}
