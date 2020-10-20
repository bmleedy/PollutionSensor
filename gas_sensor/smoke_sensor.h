// Copyright 2020 Brett M. Leedy

#ifndef GAS_SENSOR_SMOKE_SENSOR_H_
#define GAS_SENSOR_SMOKE_SENSOR_H_

/*-------------------------------------------------------------
  smoke_sensor

  This code drives a Sharp GP2Y1010AU0F sensor with control of 
  a fan and LCD display.  

  For calibration, achart of pm10 vs pm2.5:https://tinyurl.com/y3fkjgo7
  Best note for sample code: https://tinyurl.com/y6xhshw4
  US AQI standard: https://tinyurl.com/yxa48kty
 -------------------------------------------------------------*/
// LCD Display content constants and variables
#define DESC_LEN 30          // max length for AQI Description
#define ACCUM_FRACTION 0.05  // lower = more smooth filtering
// Dust Sensor Calibration Constants
#define DUST_ZERO 0.48       // Volts, 0.5 typical
#define DUST_OFFSET_CAL 0    // offset to pm2.5 value n
#define DUST_SLOPE_CAL 1.65  // multiplier for pm2.5 value


class SmokeSensor {
 private:
  // Dust Sensor Physical Configuration
  uint16_t dust_val = 0;    // most recent reading (raw)
  float dust_accum = 0.0;   // filtered value
  int dust_pin  = A0;  // Analog 0
  int led_power =  4;  // Pin D4
  const int delayTime = 280;
  const int delayTime2 = 40;
  const float offTime = 9680;
  LiquidCrystal_I2C* lcd = NULL;
  uint8_t col;
  uint8_t row;
  bool display_raw = false;

 public:
  SmokeSensor(int dustPin, int ledPower,
              LiquidCrystal_I2C* lcd, uint8_t col, uint8_t row) {
    this->col = col;
    this->row = row;
    this->dust_pin = dustPin;
    this->led_power = ledPower;

    // Sensor pins
    pinMode(this->led_power, OUTPUT);  // Turns sensor LED on/off
    pinMode(this->dust_pin, INPUT);   // Analog input

    Serial.println(F("Smoke sensor initialized on pins:"));
    Serial.print(F("pwr: ")); Serial.println(this->dust_pin);
    Serial.print(F("led: ")); Serial.println(this->led_power);

    this->lcd = lcd;
  }

  void update_lcd() {
    lcd->setCursor(col, row);
    lcd->print("dust");
    lcd->print(":");
    lcd->print(this->dust_accum, 0);
  }

  void sense() {  //////////////////////////////////////////////////////////
    // Note: this will block all other actions while I'm sensing
    digitalWrite(led_power, LOW);           // power on sensor LED
    delayMicroseconds(delayTime);           // wait for data collection
    this->dust_val = analogRead(dust_pin);  // read sensor value
    delayMicroseconds(delayTime2);          // wait for a settling time
    digitalWrite(led_power, HIGH);          // power off the sensor LED

    // Compute voltage of the dust sensor
    float dust_relative_volts =
            static_cast<float>(dust_val/1024.0*5.0) - DUST_ZERO;
    // Sensitivity: 0.5V/(0.1mg/m3) = ( 0.5v/100ugmc) = 1v / 200ugmc
    float dust_pm25 = dust_relative_volts * 200 * DUST_SLOPE_CAL +
                        DUST_OFFSET_CAL;
    dust_accum = dust_accum*(1.0 - ACCUM_FRACTION) +
                   dust_pm25 * ACCUM_FRACTION;

    if (this->lcd != NULL)
      this->update_lcd();
  }

  void log(File * log_file) {
    // Serial console output
    this->log_serial();

    // Logfile output (if configured)
    if (log_file != NULL) {
      log_file->print(this->dust_accum);
      log_file->print(",");
      log_file->print(this->dust_val);
      log_file->print(",");
    }
  }

  void set_display_raw(bool display_raw) {
    this->display_raw = display_raw;
  }

  void log_serial() {
    Serial.print(this->dust_pin);
    Serial.print(F("| dust"));
    Serial.print(F("- avg: "));
    Serial.print(this->dust_accum, 1);
    Serial.print(F(" | last: "));
    Serial.println(this->dust_val);
  }
};

#endif  // GAS_SENSOR_SMOKE_SENSOR_H_
