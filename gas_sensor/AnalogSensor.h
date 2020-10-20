// Copyright 2020 Brett M. Leedy

#ifndef GAS_SENSOR_ANALOGSENSOR_H_
#define GAS_SENSOR_ANALOGSENSOR_H_

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
  uint16_t zero_adjust = 0;
};

struct sensor_state {
  uint16_t last_value = 0;
  float avg_value = 0.0;
};

class AnalogSensor {
  sensor_config config[6];  // statically allocated.  We'll use it anyway
  sensor_state state[6];
  uint8_t num_sensors = 0;
  LiquidCrystal_I2C* lcd = NULL;
  bool display_raw = false;

 public:
  explicit AnalogSensor(LiquidCrystal_I2C* lcd);
  void add_sensor(const char short_name[SHORT_NAME_LEN],
              uint8_t column,
              uint8_t row,
              uint8_t analog_pin,
              float accum_rate);
  void sense_all();
  uint16_t sense(uint8_t id);
  void log_all(File * log_file);
  void log(File * log_file, uint8_t id);
  void log_all_serial_only();
  void log_serial(uint8_t id);
  void update_lcd();
  void set_display_raw(bool display_raw);
  void set_zero(uint8_t sensor_id, uint16_t zero_adjust);
  uint16_t get_sensor_avg(uint8_t sensor_id);
  uint8_t get_num_sensors() {return num_sensors;}
  char * get_short_name(uint8_t i) {return config[i].short_name;}
};
#endif  // GAS_SENSOR_ANALOGSENSOR_H_
