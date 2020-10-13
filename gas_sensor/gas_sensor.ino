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
 ** ! MQ-136 - Hydrogen Sulfide - https://www.sparkfun.com/products/17052
 ** ! MQ-137 - Ammonia - https://www.sparkfun.com/products/17053 
 * ! Multi-channel gas sensor: https://wiki.seeedstudio.com/Grove-Multichannel_Gas_Sensor/
 */
/*
 * Current todo list:
 *   - Refactor messy parts and clear out todo's
 *   - Display warnings and AQI
 *   - Set thresholds for warnings in menu
 *   - Collect baselines for values
 *   - Calibrate particle sensor
 *   - expose programming cable to outside 
 *   - buy additional buttons(running low)
 *   - zero the dust sensor based on menu
 */



// Indices of LCD columns
#define COL1 0
#define COL2 10


#define SECONDS_PER_DAY 86400




LiquidCrystal_I2C * lcd = new LiquidCrystal_I2C(0x27,20,4);
LogFile * logfile;
AnalogSensor * sensors;  // container for all the sensors I configure
SmokeSensor * dust;
SensorMenu * menu;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("Init menu Pins..."));
  pinMode(MENU_SELECT_BUTTON, INPUT_PULLUP);
  pinMode(MENU_UP_BUTTON, INPUT_PULLUP);
  pinMode(MENU_DN_BUTTON, INPUT_PULLUP);
  

  Serial.println(F("Init LCD..."));
  lcd->init(); 
  lcd->backlight();
  lcd->clear();
  lcd->createChar(0, file_ok_glyph);
  lcd->createChar(1, file_bad_glyph);

  Serial.println(F("Init gas Sensors..."));
  //Args are: (Shortname, LCD_column, LCD_row, Ain_pin, averaging_rate) //
  sensors = new AnalogSensor(lcd);
  //sensors->add_sensor("Dust", COL1, 0, A0, 0.1);  // Particle Sensor todo: change this to particle sensor class
  sensors->add_sensor(" Gas", COL1, 1, A1, 0.1);  // MQ5 - LPG, City Gas Leak
  sensors->add_sensor("  CO", COL1, 2, A6, 0.1);  // MQ7 - Carbon Monoxide
  sensors->add_sensor("Ozon", COL2, 0, A7, 0.1);  // MQ131 - Ozone
  sensors->add_sensor(" Gas", COL2, 1, A3, 0.1);  // Gas leaks
  sensors->add_sensor(" Haz", COL2, 2, A2, 0.1);  // MQ135Poison Gasses (organic)
 
  Serial.println(F("Init Particle Sensor..."));
  dust = new SmokeSensor(A0, 4, lcd, COL1, 0);

  Serial.println(F("Init Logfile..."));
  logfile = new LogFile();

  Serial.println(F("Init sensor menu..."));
  menu = new SensorMenu(lcd, logfile, sensors, dust, COL1, COL2);

  lcd->setCursor(0,0);
  lcd->write(byte(0));
  lcd->write(byte(1));
  if(menu->get_backlight_config())
    lcd->backlight();
  else
    lcd->noBacklight();

  Serial.println(F("Init done."));
}

uint32_t loop_number = 0;
uint32_t loop_start_millis = 0;

void loop() {
  loop_start_millis = millis();
  
  // Serial log start
  Serial.print(loop_number++); Serial.print(F(" ----------- ")); Serial.println(loop_start_millis);

  // Clear the LCD before sensing, which updates all the fields
  lcd->clear(); //todo: make sure this doesn't make it too "flashy"

  // Collect and print sensor data to screen
  sensors->sense_all();
  sensors->log_all_serial_only();
  dust->sense();
  dust->log_serial();


  // Wrap the file every day
  if(loop_number % (SECONDS_PER_DAY / (menu->get_sampling_period_ms() / 1000)) == 0){
    Serial.println(F("Wrapping the log File"));
  }

  // Log to file every N loops, if file logging is configured on
  if(menu->get_logon_config()){
    if(loop_number % menu->get_log_every_n_loops() == 0 &&
        menu->get_logon_config()) {
      logfile->open_line(loop_number, loop_start_millis);
      sensors->log_all(&logfile->file);
      dust->log(&logfile->file);
      logfile->close_line();
    }
  
    // Print glyph overlay for file status
    lcd->setCursor(19,3);
    if(logfile->is_sd_failed()){
      lcd->write(byte(1));  // Dead File
    } else {
      lcd->write(byte(0));  // OK File
    }
  }

  // todo: create warnings, or display AQI on last line, blink if bad?

  
  // Burn remainder of the loop period
  while(millis() < loop_start_millis + menu->get_sampling_period_ms()) {
    // Check that millis() hasn't wrapped
    if(loop_start_millis > millis()){
      //millis have wrapped - Should happen every 50 days, give or take
      loop_start_millis = millis(); //hacky
      break;
    }

    // Run the menu.  Stop logging and sampling if the menu is activated
    if(digitalRead(MENU_SELECT_BUTTON) == LOW || 
       digitalRead(MENU_UP_BUTTON) == LOW ||
       digitalRead(MENU_DN_BUTTON) == LOW){
   
      Serial.println(F("Enter Menu"));
      delay(1000);  // debounce
      menu->enter_menu();
    }
  }// while(millis)

  
  
}
