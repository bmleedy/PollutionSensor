#ifndef ANALOGSENSOR_H
#define ANALOGSENSOR_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

#define SHORT_NAME_LEN 5
struct sensor_config {
  char short_name[SHORT_NAME_LEN];
  float accum_rate = 0.1;
  uint8_t display_column = 0;
  uint8_t display_row = 0;
  uint8_t analog_pin = A0;
};

struct sensor_state {
  uint16_t last_value = 0;
  float avg_value = 0.0;
};

class AnalogSensor {
  sensor_config config[6];  //statically allocated.  We'll use it anyway
  sensor_state state[6];
  uint8_t num_sensors = 0;

public:
  AnalogSensor(){}
  void add_sensor(const char short_name[SHORT_NAME_LEN], 
              uint8_t column,
              uint8_t row,
              uint8_t analog_pin,
              float accum_rate);
  void sense_all();
  uint16_t sense(uint8_t id);
  void log_all(LiquidCrystal_I2C* lcd, File * log_file);
  void log(LiquidCrystal_I2C* lcd, File * log_file, uint8_t id);
};
#endif
