// Copyright 2020 Brett M. Leedy

#include "AnalogSensor.h"

AnalogSensor::AnalogSensor(LiquidCrystal_I2C* lcd) {
  this->lcd = lcd;
}

void AnalogSensor::set_display_raw(bool display_raw) {
  this->display_raw = display_raw;
}

void AnalogSensor::add_sensor(const char short_name[SHORT_NAME_LEN],
              uint8_t column,
              uint8_t row,
              uint8_t analog_pin,
              float accum_rate) {
  // Init the config
  strncpy(this->config[this->num_sensors].short_name,
            short_name, SHORT_NAME_LEN);
  this->config[this->num_sensors].accum_rate = accum_rate;

  if (column > 19 || row > 3)
    Serial.println(F("WARNING: out of bounds display config!"));
  this->config[this->num_sensors].display_column = column;
  this->config[this->num_sensors].display_row = row;
  this->config[this->num_sensors].analog_pin = analog_pin;

  // Init the data
  this->state[this->num_sensors].last_value = 0;
  this->state[this->num_sensors].avg_value = 0.0;

  // Init the analog input pin
  pinMode(analog_pin, INPUT);

  this->num_sensors++;  // mark this sensor as inited
  Serial.print(short_name); Serial.println(F(" added!"));
}


void AnalogSensor::sense_all() {
  for (uint8_t i=0; i < num_sensors; i++) {
    this->sense(i);
  }
}

// Sense the line and do averaging
uint16_t AnalogSensor::sense(uint8_t id) {
  this->state[id].last_value = analogRead(this->config[id].analog_pin);
  this->state[id].avg_value =
    this->state[id].avg_value*(1.0-this->config[id].accum_rate)
    + this->state[id].last_value*this->config[id].accum_rate;
  this->state[id].last_value;

  if ( this->lcd != NULL)
    this->update_lcd();
}

void AnalogSensor::log_all(File * log_file) {
  for (uint8_t i=0; i < num_sensors; i++) {
    this->log(log_file, i);
  }
}


void AnalogSensor::update_lcd() {
  for (uint8_t id=0; id < num_sensors; id++) {
    lcd->setCursor(this->config[id].display_column,
                    this->config[id].display_row);
    lcd->print(this->config[id].short_name);
    lcd->print(":");
    if (this->display_raw)
      lcd->print(this->state[id].last_value);
    else
      lcd->print(this->state[id].avg_value, 0);
  }
}


void AnalogSensor::log(File * log_file, uint8_t id) {
  // Serial console output
  this->log_serial(id);

  // Logfile output (if configured)
  if (log_file != NULL) {
    log_file->print(this->state[id].avg_value);
    log_file->print(",");
    log_file->print(this->state[id].last_value);
    log_file->print(",");
  }
}

void AnalogSensor::log_all_serial_only() {
  for (uint8_t i = 0; i < num_sensors; i++) {
    this->log_serial(i);
  }
}

void AnalogSensor::log_serial(uint8_t id) {
  Serial.print(this->config[id].analog_pin);
  Serial.print(F("| "));
  Serial.print(this->config[id].short_name);
  Serial.print(F("- avg: "));
  Serial.print(this->state[id].avg_value, 1);
  Serial.print(F(" | last: "));
  Serial.println(this->state[id].last_value);
}


void AnalogSensor::set_zero(uint8_t sensor_id, uint16_t zero_adjust) {
  if (this->config[sensor_id].zero_adjust != zero_adjust) {
    Serial.print(F("setting sensor ")); Serial.print(sensor_id);
    Serial.print(F(" zero adjust to ")); Serial.println(zero_adjust);
    this->config[sensor_id].zero_adjust = zero_adjust;
  }
}

uint16_t AnalogSensor::get_sensor_avg(uint8_t sensor_id) {
  return this->state[sensor_id].avg_value;
}
