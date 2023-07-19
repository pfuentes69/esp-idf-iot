/* ESPIDF Code based on Arduino library asukiaaa/AM2320_asukiaaa @ ^1.1.4*/

#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include <stdio.h>

#include "config.h"

//static const char *TAG = "AM2320";


#define I2C_MASTER_SCL_IO           SDA_PIN      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           SCL_PIN      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

#define AM2320_SUCCESS  0
#define AM2320_ERROR    1

#define AM2320_SENSOR_ADDR 0x5C   /*!< slave address for BH1750 sensor */
#define AM2320_CMD_START   0x03   /*!< Operation mode */

static i2c_config_t conf;


static uint16_t CRC16(uint8_t *ptr, uint8_t length) {
  uint16_t crc = 0xFFFF;
  uint8_t s = 0x00;

  while(length--) {
    crc ^= *ptr++;
    for(s = 0; s < 8; s++) {
      if((crc & 0x01) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else crc >>= 1;
    }
  }
  return crc;
}

static esp_err_t i2c_master_init(void)
{

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.scl_io_num = SCL_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 10000;
    i2c_param_config(I2C_NUM_0, &conf);

    return i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0);
}

esp_err_t AM2320_read(float *temp, float *hum) {
    uint8_t scanTries = 10;
    uint8_t buffLen = 8;
    uint8_t buf[buffLen];
    esp_err_t ret;
    i2c_cmd_handle_t cmd;
    float temperatureC, humidity;

  ESP_ERROR_CHECK(i2c_master_init());
//  ESP_LOGI(TAG, "I2C initialized successfully");

//  ESP_LOGI(TAG, "Update Start");

  // Wakeup sensor
  while (scanTries) {
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AM2320_SENSOR_ADDR << 1) | I2C_MASTER_READ, 1 );
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK)
      break;
    else {
      vTaskDelay(10 / portTICK_PERIOD_MS);
      scanTries--;
    }
  }
  if (scanTries) {
//      ESP_LOGI(TAG, "Found device at: 0x%2x\n", AM2320_SENSOR_ADDR);
  } else
    return ret;

  vTaskDelay(2 / portTICK_PERIOD_MS);

//  ESP_LOGI(TAG, "WakeUp sent");

  buf[0] = AM2320_CMD_START;
  buf[1] = 0x00;
  buf[2] = 0x04;

  // Send command
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (AM2320_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_DIS);
  i2c_master_write(cmd, buf, 3, ACK_CHECK_DIS);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
//  ret = i2c_master_cmd_begin(i2c_master_port, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);

  if (ret != ESP_OK) {
      return ret;
  }

//  ESP_LOGI(TAG, "Read command sent");
  
//  vTaskDelay(10 / portTICK_PERIOD_MS);
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (AM2320_SENSOR_ADDR << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
  i2c_master_read(cmd, buf, buffLen, ACK_VAL);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);

  // Fusion Code check
  if (buf[0] != 0x03) return AM2320_ERROR;

//  ESP_LOGI(TAG, "Data received");

  // CRC check
  uint16_t Rcrc = buf[7] << 8;
  Rcrc += buf[6];
  if (Rcrc != CRC16(buf, 6)) return AM2320_ERROR;

  uint16_t t = (((uint16_t) buf[4] & 0x7F) << 8) | buf[5];
  temperatureC = t / 10.0;
  temperatureC = ((buf[4] & 0x80) >> 7) == 1 ? temperatureC * (-1) : temperatureC;

  uint16_t h = ((uint16_t) buf[2] << 8) | buf[3];
  humidity = h / 10.0;

  ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
//  ESP_LOGI(TAG, "I2C de-initialized successfully");

    *temp = temperatureC;
    *hum = humidity;

  return AM2320_SUCCESS;
}
