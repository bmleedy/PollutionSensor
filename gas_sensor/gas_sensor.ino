#include<Arduino.h>
#include<Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include "AnalogSensor.h"
#include "LogFile.h"


/*------------------------------------
 * Gas sensor test harness
 * 
 * MQ-2 - Smoke - 
 ** MQ-3 - Alcohol Gas - https://www.sparkfun.com/products/8880
 * MQ-4 - Mehane (CNG) - https://www.sparkfun.com/products/9404
 ** MQ-5 - LPG, natural gas, town gas - https://tinyurl.com/y464ghh8
 * MQ-6 - LPG, iso-butane, propane - https://tinyurl.com/y4pnkfsl
 ** MQ-7 - Carbon Monoxide (CO) - https://tinyurl.com/zr2uqtn
 * MQ-8 - Hydrogen - https://www.sparkfun.com/products/10916
 ** MQ-9 - Gas leaks - https://wiki.seeedstudio.com/Grove-Gas_Sensor-MQ9/
 ** MQ-131 - Ozone - https://www.sparkfun.com/products/17051
 * MQ-135 - Harmful Gasses https://components101.com/sensors/mq135-gas-sensor-for-air-quality
 ** ! MQ-136 - Hydrogen Sulfide - https://www.sparkfun.com/products/17052
 ** ! MQ-137 - Ammonia - https://www.sparkfun.com/products/17053 
 * ! Multi-channel gas sensor: https://wiki.seeedstudio.com/Grove-Multichannel_Gas_Sensor/
 */

// Indices of LCD columns
#define COL1 0
#define COL2 10

#define LOOP_PERIOD_MILLIS 1000  // every one second
#define LOG_EVERY_N_LOOPS    10  // every ten seconds
#define SECONDS_PER_DAY 86400

LiquidCrystal_I2C * lcd = new LiquidCrystal_I2C(0x27,16,2);
LogFile * logfile;
AnalogSensor * sensors;  // container for all the sensors I configure

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("Init LCD..."));
  lcd->init(); 
  lcd->backlight();
  lcd->clear();

  Serial.println(F("Init Sensors..."));
  //Args are: (Shortname, LCD_column, LCD_row, Ain_pin, averaging_rate) //
  sensors = new AnalogSensor(lcd);
  sensors->add_sensor(" Alc", COL1, 0, A0, 0.1);  // Alcohol
  sensors->add_sensor(" LPG", COL1, 1, A1, 0.1);  // LPG
  sensors->add_sensor("  CO", COL1, 2, A2, 0.1);  // Carbon Monoxide
  sensors->add_sensor("Ozon", COL2, 0, A7, 0.1);  // Ozone
  sensors->add_sensor(" Gas", COL2, 1, A3, 0.1);  // Gas leaks
  sensors->add_sensor(" Haz", COL2, 2, A6, 0.1);  // Poison Gasses (organic)
 


  Serial.println(F("Init Logfile..."));
  logfile = new LogFile();

  Serial.println(F("Init done."));
}

uint32_t loop_number = 0;
uint32_t loop_start_millis = 0;

void loop() {
  loop_start_millis = millis();
  
  // Serial log start
  Serial.print(loop_number++); Serial.print(F(" ----------- ")); Serial.println(loop_start_millis);

  // Collect and print sensor data to screen
  sensors->sense_all();
  sensors->log_all_serial_only();



  // Wrap the file every day
  if(loop_number % (SECONDS_PER_DAY / (LOOP_PERIOD_MILLIS / 1000)) == 0){
    Serial.println(F("Wrapping the log File"));
  }

  // Log to file every N loops
  if(loop_number % LOG_EVERY_N_LOOPS == 0) {
    logfile->open_line(loop_number, loop_start_millis);
    sensors->log_all(&logfile->file);
    logfile->close_line();
  }
  
  // Burn remainder of the loop period
  while(millis() < loop_start_millis + LOOP_PERIOD_MILLIS) {
    if(loop_start_millis > millis()){
      //millis have wrapped - Should happen every 50 days, give or take
      loop_start_millis = millis(); //hacky
      break;
    }
    delayMicroseconds(20);
  }

}
