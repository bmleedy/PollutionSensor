// Copyright 2020 Brett M. Leedy

#ifndef GAS_SENSOR_CUSTOM_LCD_GLYPHS_H_
#define GAS_SENSOR_CUSTOM_LCD_GLYPHS_H_

byte file_ok_glyph[8] = {
    B11110,
    B10001,
    B11111,
    B10001,
    B11111,
    B10001,
    B11111,
  };

byte file_bad_glyph[8] = {
    B11110,
    B10001,
    B11011,
    B10101,
    B11011,
    B10001,
    B11111,
  };

byte skull_glyph[8] = {
    B00000,
    B01110,
    B10101,
    B11011,
    B01110,
    B01110,
    B00000,
    B00000
};


#endif  // GAS_SENSOR_CUSTOM_LCD_GLYPHS_H_
