#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "certstore_util.h"
#include "spiffs_util.h"


static const char *TAG = "certstore_util";


bool certstore_delete_all(void)
{
    struct stat st;

    if (!spiffs_init(false))
        return false;

    // Delete files
    if (stat("/spiffs/device.crt", &st) == 0) {
        unlink("/spiffs/device.crt");
        ESP_LOGI(TAG, "device.crt deleted");
    }
    if (stat("/spiffs/device.key", &st) == 0) {
        unlink("/spiffs/device.key");
        ESP_LOGI(TAG, "device.key deleted");
    }
    if (stat("/spiffs/device.csr", &st) == 0) {
        unlink("/spiffs/device.csr");
        ESP_LOGI(TAG, "device.csr deleted");
    }
    ESP_LOGI(TAG, "All cert material deleted");

    spiffs_finish();
    return true;
}

bool certstore_remove_client(void)
{
    struct stat st;

    if (!spiffs_init(false))
        return false;

    // Delete file
    if (stat("/spiffs/device.crt", &st) == 0) {
        unlink("/spiffs/device.crt");
        ESP_LOGI(TAG, "device.crt deleted");
    }

    spiffs_finish();
    return true;
}

bool certstore_reset_material(char *default_cert, char *default_key, char *default_csr)
{
    FILE* f;
    struct stat st;

    if (!spiffs_init(false))
        return false;

    // Delete files
    if (stat("/spiffs/device.crt", &st) == 0) {
        unlink("/spiffs/device.crt");
        printf("device.crt deleted\n");
    }
    if (stat("/spiffs/device.key", &st) == 0) {
        unlink("/spiffs/device.key");
    }
    if (stat("/spiffs/device.csr", &st) == 0) {
        unlink("/spiffs/device.csr");
    }
    ESP_LOGI(TAG, "All cert files deleted");

    // Write new device certificate file
    ESP_LOGI(TAG, "Opening file");
    f = fopen("/spiffs/device.crt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file device.crt for writing");
        spiffs_finish();
        return false;
    }
    fprintf(f, "%s", default_cert);
    fclose(f);
    ESP_LOGI(TAG, "File device.crt written");

    // Write new device key file
    ESP_LOGI(TAG, "Opening file");
    f = fopen("/spiffs/device.key", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file device.key for writing");
        spiffs_finish();
        return false;
    }
    fprintf(f, "%s", default_key);
    fclose(f);
    ESP_LOGI(TAG, "File device.key written");

    // Write new csr file
    ESP_LOGI(TAG, "Opening file");
    f = fopen("/spiffs/device.csr", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file device.csr for writing");
        spiffs_finish();
        return false;
    }
    fprintf(f, "%s", default_csr);
    fclose(f);
    ESP_LOGI(TAG, "File device.csr written");

    spiffs_finish();
    return true;
}

bool certstore_check_material(void)
{
    struct stat st;
    bool result = true;

    if (!spiffs_init(false))
        return false;

    if (stat("/spiffs/device.crt", &st) != 0) {
        result = false;
        ESP_LOGI(TAG, "File device.crt doesn't exist");
    }

    if (stat("/spiffs/device.key", &st) != 0) {
        result = false;
        ESP_LOGI(TAG, "File device.key doesn't exist");
    }

    if (stat("/spiffs/device.csr", &st) != 0) {
        result = false;
        ESP_LOGI(TAG, "File device.csr doesn't exist");
    }

    spiffs_finish();
    return result;
}

bool certstore_load_material(char *device_cert_pem, char *device_key_pem, char *device_csr_pem)
{
    FILE* f;
    int i;

    if (!spiffs_init(false))
        return false;

    // Read device.crt file
    f = fopen("/spiffs/device.crt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file device.crt for reading");
        spiffs_finish();
        return false;
    } else {
        i = 0;
        ESP_LOGE(TAG, "Reading file");
        while (!feof(f)) {
            device_cert_pem[i++] = fgetc(f);
//            printf("%c", buf[i-1]);
        }
        fclose(f);
        device_cert_pem[i-1] = '\0';
    }

    // Read device.key file
    f = fopen("/spiffs/device.key", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file device.key for reading");
        spiffs_finish();
        return false;
    } else {
        i = 0;
        ESP_LOGE(TAG, "Reading file");
        while (!feof(f)) {
            device_key_pem[i++] = fgetc(f);
//            printf("%c", buf[i-1]);
        }
        fclose(f);
        device_key_pem[i-1] = '\0';
    }

    // Read device.csr file
    f = fopen("/spiffs/device.csr", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file device.csr for reading");
        spiffs_finish();
        return false;
    } else {
        i = 0;
        ESP_LOGE(TAG, "Reading file");
        while (!feof(f)) {
            device_csr_pem[i++] = fgetc(f);
//            printf("%c", buf[i-1]);
        }
        fclose(f);
        device_csr_pem[i-1] = '\0';
    }

    spiffs_finish();
    return true;
}

bool certstore_save_file(char *file_name, char *file_content)
{
    FILE* f;

    if (!spiffs_init(false))
        return false;

    // Write file
    ESP_LOGI(TAG, "Opening file");
    f = fopen(file_name, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        spiffs_finish();
        return false;
    }
    fprintf(f, "%s", file_content);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    spiffs_finish();
    return true;
}

void certstore_dump_material(char *device_cert_pem, char *device_key_pem, char *device_csr_pem)
{
    printf("DEVICE CERT:\n%s\n", device_cert_pem);
    printf("DEVICE KEY:\n%s\n", device_key_pem);
    printf("DEVICE CSR:\n%s\n", device_csr_pem);
}
