/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <sys/param.h>
#include "driver/gpio.h"

#include "http_utils.h"
#include "wifi_utils.h"
#include "mqtt_utils.h"
#include "est_utils.h"
#include "certstore_util.h"
#include "AM2320.h"
#include "spiffs_util.h"
#include "buzzer_util.h"
#include "LGFX_ST7789.hpp"
#include "button.h"


#define LED_RED (gpio_num_t)5
#define LED_GREEN (gpio_num_t)17
#define BUTTON_RED (gpio_num_t)13
#define BUTTON_GREEN (gpio_num_t)12
#define BUZZER_IO (gpio_num_t)14

#define TASK_LED_MODE_OFF 0
#define TASK_LED_MODE_ON 1
#define TASK_LED_MODE_RED_ON 2
#define TASK_LED_MODE_RED_OFF 3
#define TASK_LED_MODE_GREEN_ON 4
#define TASK_LED_MODE_GREEN_OFF 5
#define TASK_LED_MODE_RED_ON_500ms 6
#define TASK_LED_MODE_GREEN_ON_500ms 7
#define TASK_LED_MODE_ALTERNATE 10

#define TASK_MAIN_MODE_ERROR 0
#define TASK_MAIN_MODE_IDDLE 1
#define TASK_MAIN_MODE_PUBLISH 2
#define TASK_MAIN_MODE_FACTORY_DEFAULTS 3
#define TASK_MAIN_MODE_PROVISION 4

static const char *TAG = "IoT Demo";


// AWS MQTT SERVER
const uint8_t broker_cert_pem[] = "-----BEGIN CERTIFICATE-----\n\
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n\
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n\
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n\
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n\
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n\
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n\
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n\
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n\
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n\
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n\
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n\
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n\
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n\
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n\
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n\
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n\
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n\
rqXRfboQnoZsG4q5WTP468SQvvG5\n\
-----END CERTIFICATE-----";

const char default_device_cert_pem[] = "subject=CN = aws-device-101, serialNumber = 0001001, C = CH\n\
\n\
issuer=CN = DEVIOTSUBCA4\n\
\n\
-----BEGIN CERTIFICATE-----\n\
MIIBgTCCASigAwIBAgIUXGmN3xEDTe8DeYOlgqgzphrg/yQwCgYIKoZIzj0EAwIw\n\
FzEVMBMGA1UEAwwMREVWSU9UU1VCQ0E0MB4XDTIzMDQyNzAwMDAwMFoXDTQ0MTIx\n\
MjE2MDIzMVowODEXMBUGA1UEAwwOYXdzLWRldmljZS0xMDExEDAOBgNVBAUTBzAw\n\
MDEwMDExCzAJBgNVBAYTAkNIMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEFb38\n\
Z3J2Ejr6Rng1o7xVlifSRC6naDO96Z37szeuWOFuSD2DE3aUoeEUyyTpG5ArYGj/\n\
ywPWS/yEeBKakxbeFqMxMC8wHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMC\n\
MA4GA1UdDwEB/wQEAwIFoDAKBggqhkjOPQQDAgNHADBEAiA4Bf+uM3Y1yjfTdAL4\n\
lIdhLcfQ+9JenGEbpEZm3/LN6AIgLmxWKbVIcsMLO4YtKJ4QAwpm17zmJC5YmvVO\n\
uxiVQMo=\n\
-----END CERTIFICATE-----";

const char default_device_key_pem[] = "-----BEGIN EC PRIVATE KEY-----\n\
MHcCAQEEIO6xZ2LSDW6AXHSVn5/bghYv6MBLFnJgASuYWqsEu+vOoAoGCCqGSM49\n\
AwEHoUQDQgAEFb38Z3J2Ejr6Rng1o7xVlifSRC6naDO96Z37szeuWOFuSD2DE3aU\n\
oeEUyyTpG5ArYGj/ywPWS/yEeBKakxbeFg==\n\
-----END EC PRIVATE KEY-----";

const char default_device_csr_pem[] = "-----BEGIN CERTIFICATE REQUEST-----\n\
MIHlMIGNAgEAMCsxFzAVBgNVBAMMDmF3cy1kZXZpY2UtMTAxMRAwDgYDVQQFEwcw\n\
MDAxMDAxMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEFb38Z3J2Ejr6Rng1o7xV\n\
lifSRC6naDO96Z37szeuWOFuSD2DE3aUoeEUyyTpG5ArYGj/ywPWS/yEeBKakxbe\n\
FqAAMAoGCCqGSM49BAMCA0cAMEQCIGCgA1dhdbmEDrpCT1vx+h4WC7ZG3dySFl3u\n\
HdMyYrBbAiBdQuqdB+pzFPtNGcGYwHTUuCfy+8uv9BCuF5QRfRu1ZQ==\n\
-----END CERTIFICATE REQUEST-----";


// WISEKEY ROOT GB
const char estserver_root_cert_pem[] = "-----BEGIN CERTIFICATE-----\n\
MIIDtTCCAp2gAwIBAgIQdrEgUnTwhYdGs/gjGvbCwDANBgkqhkiG9w0BAQsFADBt\n\
MQswCQYDVQQGEwJDSDEQMA4GA1UEChMHV0lTZUtleTEiMCAGA1UECxMZT0lTVEUg\n\
Rm91bmRhdGlvbiBFbmRvcnNlZDEoMCYGA1UEAxMfT0lTVEUgV0lTZUtleSBHbG9i\n\
YWwgUm9vdCBHQiBDQTAeFw0xNDEyMDExNTAwMzJaFw0zOTEyMDExNTEwMzFaMG0x\n\
CzAJBgNVBAYTAkNIMRAwDgYDVQQKEwdXSVNlS2V5MSIwIAYDVQQLExlPSVNURSBG\n\
b3VuZGF0aW9uIEVuZG9yc2VkMSgwJgYDVQQDEx9PSVNURSBXSVNlS2V5IEdsb2Jh\n\
bCBSb290IEdCIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2Be3\n\
HEokKtaXscriHvt9OO+Y9bI5mE4nuBFde9IllIiCFSZqGzG7qFshISvYD06fWvGx\n\
WuR51jIjK+FTzJlFXHtPrby/h0oLS5daqPZI7H17Dc0hBt+eFf1Biki3IPShehtX\n\
1F1Q/7pn2COZH8g/497/b1t3sWtuuMlk9+HKQUYOKXHQuSP8yYFfTvdv37+ErXNk\n\
u7dCjmn21HYdfp2nuFeKUWdy19SouJVUQHMD9ur06/4oQnc/nSMbsrY9gBQHTC5P\n\
99UKFg29ZkM3fiNDecNAhvVMKdqOmq0NpQSHiB6F4+lT1ZvIiwNjeOvgGUpuuy9r\n\
M2RYk61pv48b74JIxwIDAQABo1EwTzALBgNVHQ8EBAMCAYYwDwYDVR0TAQH/BAUw\n\
AwEB/zAdBgNVHQ4EFgQUNQ/INmNe4qPs+TtmFc5RUuORmj0wEAYJKwYBBAGCNxUB\n\
BAMCAQAwDQYJKoZIhvcNAQELBQADggEBAEBM+4eymYGQfp3FsLAmzYh7KzKNbrgh\n\
cViXfa43FK8+5/ea4n32cZiZBKpDdHij40lhPnOMTZTg+XHEthYOU3gf1qKHLwI5\n\
gSk8rxWYITD+KJAAjNHhy/peyP34EEY7onhCkRd0VQreUGdNZtGn//3ZwLWoo4rO\n\
ZvUPQ82nK1d7Y0Zqqi5S2PTt4W2tKZB4SLrhI6qjiey1q5bAtEuiHZeeevJuQHHf\n\
aPFlTc58Bd9TZaml8LGXBHAVRgOY1NK/VLSgWH1Sb9pWJmLU2NuJMW8c8CLC02Ic\n\
Nc1MaRVUGpCY3useX8p3x8uOPUNpnJpY0CQ73xtAln41rYHHTnG6iBM=\n\
-----END CERTIFICATE-----";

// FACTORY CERTIFICATE DEMO
const char factory_cert_pem[] = "-----BEGIN CERTIFICATE-----\n\
MIIB3jCCAYSgAwIBAgIUL8bYDncDVkYNMa+70vdD1r7scnkwCgYIKoZIzj0EAwIw\n\
OzEVMBMGA1UEAwwMREVWSU9UU1VCQ0ExMRUwEwYDVQQKDAxXSVNFS0VZIERFTU8x\n\
CzAJBgNVBAYTAkNIMB4XDTIzMDQyNTExMjY0NFoXDTQ0MTAzMTE1MTA0NVowKzEp\n\
MCcGA1UEAwwgYmI3MmVjMGU0MmE4NGI0YWFkNTA3NTdkZTZlNDY3ZmEwWTATBgcq\n\
hkjOPQIBBggqhkjOPQMBBwNCAASz0scz6mTQXe/oKl7q6Y1KlpBFymPnVkxuxZJ6\n\
IJQTqF+3ML+lBzEPcwsU91FyVr1GMmYq8ueZb/SDxuHoOlQpo3YwdDBDBgNVHR8E\n\
PDA6MDigNqA0hjJodHRwOi8vcHVibGljLndpc2VrZXlkZW1vLmNvbS9jcmwvZGV2\n\
aW90c3ViY2ExLmNybDAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDgYD\n\
VR0PAQH/BAQDAgWgMAoGCCqGSM49BAMCA0gAMEUCIQC1uCOETBcOf1t1b3ttNcmG\n\
OAFHPKlkTsP+KW26hYNMeAIgRwoErMFzScq0vRU773fuaJTD6oWstJER5hMzIeCm\n\
PnM=\n\
-----END CERTIFICATE-----";

const char factory_key_pem[] = "-----BEGIN EC PRIVATE KEY-----\n\
MHcCAQEEIMFfXKF5ivjRxoCwbnmnLV+KlyhyGOUzOxSh8QwFF+NvoAoGCCqGSM49\n\
AwEHoUQDQgAEs9LHM+pk0F3v6Cpe6umNSpaQRcpj51ZMbsWSeiCUE6hftzC/pQcx\n\
D3MLFPdRcla9RjJmKvLnmW/0g8bh6DpUKQ==\n\
-----END EC PRIVATE KEY-----";

char device_cert_pem[MAX_HTTP_OUTPUT_BUFFER], device_key_pem[MAX_HTTP_OUTPUT_BUFFER], device_csr_pem[MAX_HTTP_OUTPUT_BUFFER];


/*
esp_mqtt_client_config_t mqtt_cfg = {
	.broker.address.uri = CONFIG_BROKER_URL,
};
*/

/*
esp_mqtt_client_config_t mqtt_cfg = {
	.broker.address.uri = "mqtts://aofwbgp3py5sf-ats.iot.eu-west-1.amazonaws.com", //test.mosquitto.org:8884",
	.broker.verification.certificate = (const char *)broker_cert_pem,
	.credentials = {
		.authentication = {
			.certificate = (const char *)default_device_cert_pem,
			.key = (const char *)default_device_key_pem,
		},
	}
};
*/

static LGFX_ST7789 lcd;

esp_mqtt_client_config_t mqtt_cfg;

static const char *states[] = {
    [BUTTON_PRESSED]      = "pressed",
    [BUTTON_RELEASED]     = "released",
    [BUTTON_CLICKED]      = "clicked",
    [BUTTON_PRESSED_LONG] = "pressed long",
};

static button_t btn_green, btn_red;
static const char RED_BUTTON[] = "RED";
static const char GREEN_BUTTON[] = "GREEN";

// Global task handlers

TaskHandle_t h_task_led;
int task_led_mode;
TaskHandle_t h_task_mqtt;
int task_main_mode = TASK_MAIN_MODE_IDDLE;


// Local function definitions
void task_led(void *pvParameters);
void task_op_mode(void *pvParameters);


////////
// UTILITY FUNCTIONS
////////
bool ines_reprovision(void)
{

    ESP_LOGI(TAG, ">>>>>>>>>   est_simpleenroll");
//    if (est_simpleenroll((char *)"testrfc7030.com", 8443, (char *)estserver_root_cert_pem, "estuser", "estpwd", (char *)device_csr_pem, device_cert_pem))
//    if (est_simpleenroll((char *)"wksa-est.certifyiddemo.com", 443, (char *)estserver_root_cert_pem, "bb72ec0e42a84b4aad50757de6e467fa", "d28ba1b7b80445cbab7248036ac9b2a1", (char *)device_csr_pem, device_cert_pem))
    if (est_simpleenroll_certauth((char *)"wksa-est.certifyiddemo.com", 443, (char *)estserver_root_cert_pem, (char *)factory_cert_pem, (char *)factory_key_pem, (char *)device_csr_pem, device_cert_pem))
        printf("DEVICE CERT (PKCS#7):\n%s\n", device_cert_pem);
    else {
        printf("ERROR GETTING CERTIFICATE\n");
		return false;
	}

    ESP_LOGI(TAG, ">>>>>>>>>   est_convert_p7_to_pem");
    if (est_convert_p7_to_pem(device_cert_pem))
        printf("DEVICE CERT (PEM):\n%s\n", device_cert_pem);
    else {
        printf("ERROR CONVERTING PKCS#7 to PEM\n");
		return false;
	}

    certstore_save_file((char *)"/spiffs/device.crt", device_cert_pem);

    ESP_LOGI(TAG, "NEW MATERIAL:");
    certstore_dump_material(device_cert_pem, device_key_pem, device_csr_pem);

	return true;
}

////////
// HANDLERS
////////

static void on_button(button_t *btn, button_state_t state)
{
	char jsonstr[100];
	char but_name[10]; 

/*
	printf("6. TASK_LED_MODE_ALTERNATE\n");
	task_led_mode = TASK_LED_MODE_ALTERNATE;
	xTaskCreate(task_led, "task_led", 1024, NULL, 2, &h_task_led);
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	vTaskDelete(h_task_led);
*/
	switch ((int)state) {
		case BUTTON_CLICKED:
			if (btn == &btn_green)
				strcpy(but_name, "GREEN");
			else
				strcpy(but_name, "RED");
		    ESP_LOGI(TAG, "%s button %s", but_name, states[state]);
			strcpy(jsonstr, "{ \"Button\": \"");
			strcat(jsonstr, but_name);
			strcat(jsonstr, "\",\"State\": \"");
			strcat(jsonstr, states[state]);
			strcat(jsonstr, "\"}");
			mqtt_publish((char *)"/awsdevice/data", jsonstr);
			if (btn == &btn_green) {
				strcpy(but_name, GREEN_BUTTON);
				task_led_mode = TASK_LED_MODE_GREEN_ON_500ms;
				xTaskCreate(task_led, "task_led", 1024, NULL, 2, &h_task_led);
			} else {
				task_led_mode = TASK_LED_MODE_RED_ON_500ms;
				xTaskCreate(task_led, "task_led", 1024, NULL, 2, &h_task_led);
				strcpy(but_name, RED_BUTTON);
			}
			break;
		case BUTTON_PRESSED_LONG:
			if (btn == &btn_green) {
				// ACTION GREEN+LONG: ENABLE/DISABLE MQTT PUBLISH
				lcd.setColor(0x3d3c3cu);
				lcd.fillRect(0, 180, 239, 239);
				strcpy(jsonstr, "{ \"Message\": \"");
				if (task_main_mode == TASK_MAIN_MODE_PUBLISH) {
					ESP_LOGI(TAG, "DISABLE MQTT PUBLISH!");
					task_main_mode = TASK_MAIN_MODE_IDDLE;
					strcat(jsonstr, "DISABLE MQTT PUBLISH!\"}");
					mqtt_publish((char *)"/awsdevice/status", jsonstr);
					mqtt_app_stop();
					lcd.setCursor(40, 200);
					lcd.setTextColor(0x00ff00u, 0x3d3c3cu);
					lcd.println("Iddle Mode");
				} else {
					ESP_LOGI(TAG, "ENABLE MQTT PUBLISH!");
					task_main_mode = TASK_MAIN_MODE_PUBLISH;
					mqtt_app_start();
					strcat(jsonstr, "ENABLE MQTT PUBLISH!\"}");
					mqtt_publish((char *)"/awsdevice/status", jsonstr);
					lcd.setCursor(40, 200);
					lcd.setTextColor(0x00ff00u, 0x3d3c3cu);
					lcd.println("Op Mode");
				}
			} else {
				// ACTION RED+LONG
				ESP_LOGI(TAG, "REPROVISION DEVICE ID");
				lcd.setColor(0x3d3c3cu);
				lcd.fillRect(0, 180, 239, 239);
				lcd.setCursor(40, 200);
				lcd.setTextColor(0x00ff00u, 0x3d3c3cu);
				lcd.println("Factory Mode");
				task_led_mode = TASK_LED_MODE_ALTERNATE;
				xTaskCreate(task_led, "task_led", 1024, NULL, 2, &h_task_led);
				mqtt_app_stop();
				if (ines_reprovision()) {
					ESP_LOGI(TAG, "REPROVISION OK");
					vTaskDelete(h_task_led);
					task_main_mode = TASK_MAIN_MODE_IDDLE;
					task_led_mode = TASK_LED_MODE_OFF;
					xTaskCreate(task_led, "task_led", 1024, NULL, 2, &h_task_led);
					lcd.setColor(0x3d3c3cu);
					lcd.fillRect(0, 180, 239, 239);
					lcd.setCursor(40, 200);
					lcd.setTextColor(0x00ff00u, 0x3d3c3cu);
					lcd.println("Iddle Mode");
				} else {
					ESP_LOGI(TAG, "REPROVISION ERROR");
					vTaskDelete(h_task_led);
					task_main_mode = TASK_MAIN_MODE_ERROR;
					task_led_mode = TASK_LED_MODE_RED_ON;
					xTaskCreate(task_led, "task_led", 1024, NULL, 2, &h_task_led);
					lcd.setColor(0x3d3c3cu);
					lcd.fillRect(0, 180, 239, 239);
					lcd.setCursor(40, 200);
					lcd.setTextColor(0x00ff00u, 0x3d3c3cu);
					lcd.println("ERROR!");
				}
			}
			break;
	} 

}



/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "/awsdevice/status", "{\"Message\": \"MQTT_EVENT_CONNECTED\"}", 0, 1, 0);
//        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/awsdevice/command", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

//        msg_id = esp_mqtt_client_subscribe(client, "/esp32/topic/qos1", 1);
//        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

//        msg_id = esp_mqtt_client_unsubscribe(client, "/esp32/topic/qos1");
//        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
//        msg_id = esp_mqtt_client_publish(client, "/awsdevice/status", "{\"Message\": \"MQTT_EVENT_DISCONNECTED\"}", 0, 1, 0);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
//        msg_id = esp_mqtt_client_publish(client, "/awsdevice/status", "{\"Message\": \"MQTT_EVENT_SUBSCRIBED\"}", 0, 0, 0);
//        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
        ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);
	    buzzer_beep(BUZZER_IO, 500, 500);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


///////
// TASKS
///////

void task_main(void *pvParameters)
{
	char buf1[10], jsonstr[100];
    float temp, humidity;


	mqtt_cfg.broker.address.uri = "mqtts://aofwbgp3py5sf-ats.iot.eu-west-1.amazonaws.com"; //test.mosquitto.org:8884",
	mqtt_cfg.broker.verification.certificate = (const char *)broker_cert_pem;
//	mqtt_cfg.credentials.authentication.certificate = (const char *)default_device_cert_pem;
//	mqtt_cfg.credentials.authentication.key = (const char *)default_device_key_pem;

	mqtt_cfg.credentials.authentication.certificate = device_cert_pem;
	mqtt_cfg.credentials.authentication.key = device_key_pem;

	mqtt_app_launch(&mqtt_cfg, (void *)&mqtt_event_handler, false);
// void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
// void mqtt_app_launch(esp_mqtt_client_config_t *mqtt_cfg, void *mqtt_event_handler, bool auto_start)

	while(1) {
		if (task_main_mode == TASK_MAIN_MODE_PUBLISH) {
			task_led_mode = TASK_LED_MODE_GREEN_ON_500ms;
			xTaskCreate(task_led, "task_led", 1024, NULL, 2, &h_task_led);
			AM2320_read(&temp, &humidity);
			strcpy(jsonstr, "{ \"Temperature\": ");
			sprintf(buf1, "%.2f", temp);
			strcat(jsonstr, buf1);
			strcat(jsonstr, ", \"Humidity\": ");
			sprintf(buf1, "%.2f", humidity);
			strcat(jsonstr, buf1);
			strcat(jsonstr, "}");

			ESP_LOGI(TAG, "MQTT PUBLISH TASK. Payload: %s", jsonstr);
			mqtt_publish((char *)"/awsdevice/data", jsonstr);

			vTaskDelay(5000 / portTICK_PERIOD_MS);
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void task_led(void *pvParameters)
{
	switch (task_led_mode) {
		case TASK_LED_MODE_OFF:
			gpio_set_level(LED_RED, 1);
			gpio_set_level(LED_GREEN, 1);
			break;
		case TASK_LED_MODE_ON:
			gpio_set_level(LED_RED, 0);
			gpio_set_level(LED_GREEN, 0);
			break;
		case TASK_LED_MODE_RED_ON:
			gpio_set_level(LED_RED, 0);
			break;
		case TASK_LED_MODE_GREEN_ON:
			gpio_set_level(LED_GREEN, 0);
			break;
		case TASK_LED_MODE_RED_OFF:
			gpio_set_level(LED_RED, 1);
			break;
		case TASK_LED_MODE_RED_ON_500ms:
			gpio_set_level(LED_RED, 0);
			vTaskDelay(500 / portTICK_PERIOD_MS);
			gpio_set_level(LED_RED, 1);
			break;
		case TASK_LED_MODE_GREEN_ON_500ms:
			gpio_set_level(LED_GREEN, 0);
			vTaskDelay(500 / portTICK_PERIOD_MS);
			gpio_set_level(LED_GREEN, 1);
			break;
		case TASK_LED_MODE_ALTERNATE:
			while(1) {
				gpio_set_level(LED_RED, 1);
				gpio_set_level(LED_GREEN, 0);
				vTaskDelay(250 / portTICK_PERIOD_MS);
				gpio_set_level(LED_RED, 0);
				gpio_set_level(LED_GREEN, 1);
				vTaskDelay(250 / portTICK_PERIOD_MS);
			}
			break;
	}
	vTaskDelete(NULL);
}


/////////
// HW INIT
/////////



bool TFT_Init(void)
{
  if (!lcd.init())
  {
    ESP_LOGW(TAG, "lcd.init() failed");
    return false;
  }

  lcd.setBrightness(128);

  return true;
}


void hardware_config(void)
{
	// BUTTON CONFIG

	btn_green.gpio = BUTTON_GREEN;
	btn_green.pressed_level = 0;
	btn_green.internal_pull = true;
	btn_green.autorepeat = false;
	btn_green.callback = on_button;

	btn_red.gpio = BUTTON_RED;
	btn_red.pressed_level = 0;
	btn_red.internal_pull = true;
	btn_red.autorepeat = false;
	btn_red.callback = on_button;

	ESP_ERROR_CHECK(button_init(&btn_green));
	ESP_ERROR_CHECK(button_init(&btn_red));

	// LED CONFIG
    esp_rom_gpio_pad_select_gpio(LED_RED);
    gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(LED_GREEN);
    gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
	gpio_set_level(LED_RED, 1);
	gpio_set_level(LED_GREEN, 1);
}

extern "C" void app_main(void)
{
//	uint8_t ascii[20];
//	char ascii[20];

//    char file[32];
    esp_err_t ret;

    esp_log_level_set("*", ESP_LOG_INFO);
//    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
//    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
//    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
//    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
//    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
//    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

	TFT_Init();
	lcd.setTextSize(2);
	lcd.TFT_show_PNG((char *)"/spiffs/Splash.png", CONFIG_WIDTH, CONFIG_HEIGHT);
	lcd.setCursor(40, 200);
	lcd.setTextColor(0x00ff00u, 0x3d3c3cu);
	lcd.println("ESP32 Start");

	//Initialize NVS
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	hardware_config();


	// Prepare certificate materials
    if (certstore_check_material()) {
        ESP_LOGI(TAG, "All required material present");
    } else {
        ESP_LOGI(TAG, "Material not present");
        if (certstore_reset_material((char *)default_device_cert_pem, (char *)default_device_key_pem, (char *)default_device_csr_pem))
            ESP_LOGI(TAG, "Default material created");
        else
            ESP_LOGI(TAG, "Error saving default material");
    }
    if (certstore_load_material(device_cert_pem, device_key_pem, device_csr_pem)) {
        ESP_LOGI(TAG, "Material loaded");
        certstore_dump_material(device_cert_pem, device_key_pem, device_csr_pem);
    } else {
        ESP_LOGI(TAG, "Material not loaded");
	}


// Erase screen
// lcdFillScreen(&tft_dev, BLACK);

    lcd.setColor(0x3d3c3cu);
    lcd.fillRect(0, 180, 239, 239);
	lcd.setCursor(40, 200);
	lcd.setTextColor(0x00ff00u, 0x3d3c3cu);
	lcd.println("WiFi Init");

	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	wifi_init();
	wifi_start();
//	wifi_scan();
	wifi_stop();
	wifi_init_sta();

    lcd.setColor(0x3d3c3cu);
    lcd.fillRect(0, 180, 239, 239);
	lcd.setCursor(40, 200);
	lcd.setTextColor(0x00ff00u, 0x3d3c3cu);
	lcd.println("Iddle Mode");

	task_main_mode = TASK_MAIN_MODE_IDDLE;
	xTaskCreate(task_main, "task_main", 1024*8, NULL, 2, &h_task_mqtt);

	// Never ends, to let tasks running
	while(1)
		vTaskDelay(1000 / portTICK_PERIOD_MS);
}
