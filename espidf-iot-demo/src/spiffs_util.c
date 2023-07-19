#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"


static esp_vfs_spiffs_conf_t spiffs_conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true
};

static const char *TAG = "spiffs_util";

bool spiffs_init(bool check_spiffs)
{
    esp_err_t ret = esp_vfs_spiffs_register(&spiffs_conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return false;
    }

    if (check_spiffs) {
        ESP_LOGI(TAG, "Performing SPIFFS_check().");
        ret = esp_spiffs_check(spiffs_conf.partition_label);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return false;
        } else {
            ESP_LOGI(TAG, "SPIFFS_check() successful");
        }

        size_t total = 0, used = 0;
        ret = esp_spiffs_info(spiffs_conf.partition_label, &total, &used);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
            esp_spiffs_format(spiffs_conf.partition_label);
            return false;
        } else {
            ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
        }

        // Check consistency of reported partiton size info.
        if (used > total) {
            ESP_LOGW(TAG, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
            ret = esp_spiffs_check(spiffs_conf.partition_label);
            // Could be also used to mend broken files, to clean unreferenced pages, etc.
            // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
                return false;
            } else {
                ESP_LOGI(TAG, "SPIFFS_check() successful");
            }
        }
    }
    ESP_LOGI(TAG, "SPIFFS mounted");
    return true;
}

void spiffs_finish(void)
{
    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(spiffs_conf.partition_label);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}


void spiffs_directory(const char * path) {
	DIR* dir = opendir(path);
	assert(dir != NULL);
	while (true) {
		struct dirent*pe = readdir(dir);
		if (!pe) break;
		ESP_LOGI(__FUNCTION__,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
	}
	closedir(dir);
}

