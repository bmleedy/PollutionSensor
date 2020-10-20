// Copyright 2020 Brett M. Leedy

#include<Arduino.h>
#include<Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include "AnalogSensor.h"
#include "LogFile.h"
#include "smoke_sensor.h"
#include "SensorMenu.h"
#include "custom_lcd_glyphs.h"

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
 ** ! MQ-136 - Hydrogen Sulfide - https://www.sparkfun.com/products/17052 (spare one in yellow box)
 ** ! MQ-137 - Ammonia - https://www.sparkfun.com/products/17053 
 * ! Multi-channel gas sensor: https://wiki.seeedstudio.com/Grove-Multichannel_Gas_Sensor/
 */
/*
 * Current todo list:
 *   - Collect baselines for values
 *   - Calibrate particle sensor
 */

// Indices of LCD columns
#define COL1  0
#define COL2 10

// LCD Glyph ID's for this sketch
#define FILE_OK_GLYPH  0
#define FILE_BAD_GLYPH 1
#define SKULL_GLYPH    2

// Constants
#define SECONDS_PER_DAY 86400

// Classes for main components
LiquidCrystal_I2C * lcd;
LogFile           * logfile;
AnalogSensor      * sensors;  // container for all the sensors I configure
SmokeSensor       * dust;
SensorMenu        * menu;

void setup() {
  
  // Init Serial port
  Serial.begin(115200);
  while (!Serial); // Wait for it to be initialized.

  Serial.println(F("Init Pins..."));
  pinMode(MENU_SELECT_BUTTON, INPUT_PULLUP);
  pinMode(MENU_UP_BUTTON,     INPUT_PULLUP);
  pinMode(MENU_DN_BUTTON,     INPUT_PULLUP);
  
  Serial.println(F("Init LCD..."));
  lcd = new LiquidCrystal_I2C(0x27,20,4);
  lcd->init(); 
  lcd->backlight();
  lcd->clear();
  lcd->createChar(FILE_OK_GLYPH,  file_ok_glyph);
  lcd->createChar(FILE_BAD_GLYPH, file_bad_glyph);
  lcd->createChar(SKULL_GLYPH,    skull_glyph);
  lcd->setCursor(0,0);
  if(menu->get_backlight_config())
    lcd->backlight();
  else
    lcd->noBacklight();

  Serial.println(F("Init sensor menu..."));
  menu = new SensorMenu(lcd, COL1, COL2);

  Serial.println(F("Init gas Sensors..."));
  sensors = new AnalogSensor(lcd);
  if(!menu->is_alternate_config()){
    //Args are: (Shortname, LCD_column, LCD_row, Ain_pin, averaging_rate) //
    sensors->add_sensor(" LPG", COL1, 1, A1, 0.1);  // MQ5 - LPG, City Gas Leak
    sensors->add_sensor("  CO", COL1, 2, A6, 0.1);  // MQ7 - Carbon Monoxide
    sensors->add_sensor("Ozon", COL2, 0, A7, 0.1);  // MQ131 - Ozone
    sensors->add_sensor(" Gas", COL2, 1, A3, 0.1);  // MP9 Gas leaks
    sensors->add_sensor(" Haz", COL2, 2, A2, 0.1);  // MQ135Poison Gasses (organic)
    Serial.println(F("Init Particle Sensor..."));
    dust = new SmokeSensor(A0, 4, lcd, COL1, 0);
    menu->attach_dust_sensor(dust);
  } else {
    sensors->add_sensor("MQ2 ", COL1, 1, A0, 0.1);  // MQ2 - smoke
    sensors->add_sensor("MQ7 ", COL1, 2, A1, 0.1);  // MQ7 - Carbon Monoxide
    sensors->add_sensor("MQ4 ", COL2, 0, A2, 0.1);  // MQ4 - Methane (CNG)
    sensors->add_sensor("MQ6 ", COL2, 1, A3, 0.1);  // MQ-6 - LPG, iso-butane, propane
    sensors->add_sensor("MQ8 ", COL2, 2, A6, 0.1);  // MQ-8 - Hydrogen
    sensors->add_sensor("M137", COL1, 0, A7, 0.1);  // MQ-137 - Ammonia
    Serial.println(F("No Particle Sensor in config!"));
    dust = NULL;
  }
  menu->attach_analog_sensors(sensors);

  Serial.println(F("Init Logfile..."));
  logfile = new LogFile();
  menu->attach_logfile(logfile);

  Serial.println(F("Init done."));
}

void check_menu(){
  // Run the menu.  Stop logging and sampling if the menu is activated
  if(digitalRead(MENU_SELECT_BUTTON) == LOW || 
     digitalRead(MENU_UP_BUTTON) == LOW ||
     digitalRead(MENU_DN_BUTTON) == LOW){
 
    Serial.println(F("Enter Menu"));
    delay(1000);  // debounce
    menu->enter_menu();
  }
}

uint32_t loop_number = 0;
uint32_t loop_start_millis = 0;

void loop() {
  loop_start_millis = millis();
  
  // Serial log start
  Serial.print(loop_number++);
  Serial.print(F(" ----------- "));
  Serial.println(loop_start_millis);

  // Clear the LCD before sensing, which updates all the fields
  lcd->clear();

  // Collect and print sensor data to screen
  sensors->sense_all();
  sensors->log_all_serial_only();
  if(dust != NULL){
    dust->sense();
    dust->log_serial();
  }
  


  // Wrap the file every day
  if(loop_number % (SECONDS_PER_DAY / (menu->get_sampling_period_ms() / 1000)) == 0){
    Serial.println(F("Wrapping the log File"));
  }

  // Log to file every N loops, if file logging is configured on
  if(menu->get_logon_config()){
    if(loop_number % menu->get_log_every_n_loops() == 0 &&
        menu->get_logon_config()) {
      logfile->open_line(loop_number, loop_start_millis);
      sensors->log_all(logfile->get_file_ptr());
      if(dust != NULL){dust->log(logfile->get_file_ptr());}
      logfile->close_line();
    }
  
    // Print glyph overlay for file status
    lcd->setCursor(19,3);
    if(logfile->is_sd_failed()){
      lcd->write(byte(FILE_BAD_GLYPH));  // Dead File
    } else {
      lcd->write(byte(FILE_OK_GLYPH));  // OK File
    }
  }

  // Display sensor threshod warnings
  for(uint8_t sensor_id = 0; sensor_id < sensors->get_num_sensors(); sensor_id++){
    if(sensors->get_sensor_avg(sensor_id) > menu->get_sensor_threshold(sensor_id)){
      lcd->setCursor(COL1, 3);
      lcd->print(sensors->get_short_name(sensor_id));
      lcd->print(F(" WARNING "));
      lcd->write(byte(SKULL_GLYPH));
      break;  // only have room to print one.
    }
  }

  check_menu();

  // Set sensor zeros, based on menu adjustments
  
  // Burn remainder of the loop period
  while(millis() < loop_start_millis + menu->get_sampling_period_ms()) {
    // Check that millis() hasn't wrapped
    if(loop_start_millis > millis()){
      //millis have wrapped - Should happen every 50 days, give or take
      loop_start_millis = millis(); //hacky
      break;
    }

    check_menu();
  }// while(millis)

  
  
}
