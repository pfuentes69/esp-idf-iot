#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_log.h"

//#include "decode_png.h"
//#include "pngle.h"

#include "LGFX_ST7789.hpp"

#include "spiffs_util.h"

#define TAG "LGFX_ST7789"

#define DISPLAY_RST      4
#define DISPLAY_DC       2
#define DISPLAY_MOSI     23
#define DISPLAY_CS       -1
#define DISPLAY_SCLK     18
#define DISPLAY_LEDA    -1
#define DISPLAY_MISO    -1
#define DISPLAY_BUSY    -1
#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT  240

LGFX_ST7789 :: LGFX_ST7789(void)
{
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host    = VSPI_HOST; //SPI2_HOST;              // SPI2_HOST is in use by the RGB led
      cfg.spi_mode    = 2;                      // Set SPI communication mode (0 ~ 3)
      cfg.freq_write  = 40000000; //27000000;               // SPI clock when sending (max 80MHz, rounded to 80MHz divided by an integer)
      cfg.freq_read   = 16000000;               // SPI clock when receiving
      cfg.spi_3wire   = false;                   // Set true when receiving on the MOSI pin
      cfg.use_lock    = true;                  // Set true when using transaction lock
      cfg.dma_channel = SPI_DMA_CH_AUTO;        // Set the DMA channel to use (0=not use DMA / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=auto setting)

      cfg.pin_sclk    = DISPLAY_SCLK;           // set SPI SCLK pin number
      cfg.pin_mosi    = DISPLAY_MOSI;           // Set MOSI pin number for SPI
      cfg.pin_miso    = DISPLAY_MISO;           // Set MISO pin for SPI (-1 = disable)
      cfg.pin_dc      = DISPLAY_DC;             // Set SPI D/C pin number (-1 = disable)

     _bus_instance.config (cfg);                // Apply the setting value to the bus.
     _panel_instance.setBus (&_bus_instance);   // Sets the bus to the panel.
    }

    {
      auto cfg = _panel_instance.config();      // Obtain the structure for display panel settings.

      cfg.pin_cs   = DISPLAY_CS;                // Pin number to which CS is connected (-1 = disable)
      cfg.pin_rst  = DISPLAY_RST;               // pin number where RST is connected (-1 = disable)
      cfg.pin_busy = DISPLAY_BUSY ;             // pin number to which BUSY is connected (-1 = disable)

      cfg.panel_width       = DISPLAY_HEIGHT;   // actual displayable width. Note: width/height swapped due to the rotation
      cfg.panel_height      = DISPLAY_WIDTH;    // Actual displayable height Note: width/height swapped due to the rotation
      cfg.offset_x          = 0;               // Panel offset in X direction
      cfg.offset_y          = 0;                // Y direction offset amount of the panel
      cfg.offset_rotation   = 0;                // Rotation direction value offset 0~7 (4~7 are upside down)
      cfg.dummy_read_pixel  = 8;                // Number of bits for dummy read before pixel read
      cfg.dummy_read_bits   = 1;                // Number of dummy read bits before non-pixel data read
      cfg.readable          = true;             // set to true if data can be read
      cfg.invert            = true;
      cfg.rgb_order         = false;
      cfg.dlen_16bit        = false;            // Set to true for panels that transmit data length in 16-bit units with 16-bit parallel or SPI
      cfg.bus_shared        = true;             // If the bus is shared with the SD card, set to true (bus control with drawJpgFile etc.)

      // Please set the following only when the display is shifted with a driver with a variable number of pixels such as ST7735 or ILI9163.
      //cfg.memory_width  = 132;                  // Maximum width supported by driver IC
      //cfg.memory_height = 160;                  // Maximum height supported by driver IC

      _panel_instance.config(cfg);
    }

/*
    {
      auto cfg = _light_instance.config();

      cfg.pin_bl      = DISPLAY_LEDA;           // pin number to which the backlight is connected
      cfg.invert      = true;                   // true to invert backlight brightness
      cfg.freq        = 12000;                  // Backlight PWM frequency
      cfg.pwm_channel = 7;                      // PWM channel number to use

      _light_instance.config(cfg);
      _panel_instance. setLight (&_light_instance);
    }
*/
    setPanel (&_panel_instance);
}




/////////////
/// MOD PF
/////////////

bool LGFX_ST7789 :: TFT_show_PNG(char * file, int width, int height)
{
	bool result = true;

	if (!spiffs_init(false))
		return false;
	else {
		result = this->drawPngFile(file, 0, 0, CONFIG_WIDTH, CONFIG_HEIGHT);
		spiffs_finish();
	}

	return result;
}
