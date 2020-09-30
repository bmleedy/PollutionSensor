#include "AnalogSensor.h"

void AnalogSensor::add_sensor(const char short_name[SHORT_NAME_LEN], 
              uint8_t column,
              uint8_t row,
              uint8_t analog_pin,
              float accum_rate){

  // Init the config
  strncpy(this->config[this->num_sensors].short_name, short_name, SHORT_NAME_LEN);
  this->config[this->num_sensors].accum_rate = accum_rate;
  this->config[this->num_sensors].display_column = column;
  this->config[this->num_sensors].display_row = row;  // todo: add sanity check
  this->config[this->num_sensors].analog_pin = analog_pin;

  // Init the data
  this->state[this->num_sensors].last_value = 0;
  this->state[this->num_sensors].avg_value = 0.0;

  // Init the analog input pin
  pinMode(analog_pin, INPUT);

  this->num_sensors++; // mark this sensor as inited
}


void AnalogSensor::sense_all(){
  for(uint8_t i=0; i<num_sensors; i++){
    this->sense(i);
  }
}

// Sense the line and do averaging
uint16_t AnalogSensor::sense(uint8_t id){
  this->state[id].last_value = analogRead(this->config[id].analog_pin);
  this->state[id].avg_value = this->state[id].avg_value*(1.0-this->config[id].accum_rate) 
                                      + this->state[id].last_value*this->config[id].accum_rate;
  this->state[id].last_value;
}

void AnalogSensor::log_all(LiquidCrystal_I2C* lcd, File * log_file){
  for(uint8_t i=0; i<num_sensors; i++){
    this->log(lcd, log_file, i);
  }
}
  
void AnalogSensor::log(LiquidCrystal_I2C* lcd, File * log_file, uint8_t id){

  // Serial console output
  Serial.print(this->config[id].analog_pin);
  Serial.print(F("| "));
  Serial.print(this->config[id].short_name);
  Serial.print(F("- avg: "));
  Serial.print(this->state[id].avg_value, 1);
  Serial.print(F(" | last: "));
  Serial.println(this->state[id].last_value);

  // LCD output
  lcd->setCursor(this->config[id].display_column,this->config[id].display_row);
  lcd->print(this->config[id].short_name);
  lcd->print(":");
  lcd->print(this->state[id].avg_value, 0);

  // Logfile output (if configured)
//  if(log_file != NULL){
    //log_file->print(this->config[id].short_name); space saving
    //log_file->print(":,");
    log_file->print(this->state[id].avg_value);
    log_file->print(",");
    log_file->print(this->state[id].last_value);
    log_file->print(",");
//  }
}