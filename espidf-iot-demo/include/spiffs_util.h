#ifndef H_SPIFFS_UTIL
#define H_SPIFFS_UTIL

#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

bool spiffs_init(bool check_spiffs);
void spiffs_finish(void);

#ifdef __cplusplus
}
#endif

#endif