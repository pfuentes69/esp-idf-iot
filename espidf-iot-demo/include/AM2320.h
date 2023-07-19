#ifndef H_AM2320
#define H_AM2320

#include <freertos/FreeRTOS.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t AM2320_read(float *temperatureC, float *humidity);

#ifdef __cplusplus
}
#endif

#endif