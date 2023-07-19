#ifndef MAIN_LGFX_ST7789_HPP_
#define MAIN_LGFX_ST7789_HPP_

#include "LovyanGFX.hpp"

/* ----------------------*/
class LGFX_ST7789 : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
//  lgfx::Light_PWM     _light_instance;

public:
  LGFX_ST7789(void);
  bool TFT_show_PNG(char * file, int width, int height);
};


#endif /* MAIN_LGFX_ST7789_HPP_ */

