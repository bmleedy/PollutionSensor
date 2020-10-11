#include<Arduino.h>
#include<Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include "AnalogSensor.h"
#include "LogFile.h"
#include "smoke_sensor.h"

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

// Menu Buttons
#define MENU_SELECT_BUTTON 7
#define MENU_UP_BUTTON     6
#define MENU_DN_BUTTON     5


LiquidCrystal_I2C * lcd = new LiquidCrystal_I2C(0x27,20,4);
LogFile * logfile;
AnalogSensor * sensors;  // container for all the sensors I configure
SmokeSensor * dust;

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

  Serial.println(F("Init done."));
}

uint32_t loop_number = 0;
uint32_t loop_start_millis = 0;


bool exit_callback(){
  return true;
}


void display_file_menu(){
  Serial.println(F("Entered file Callback"));
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(F("Current File: "));
  lcd->setCursor(0, 1);
  lcd->print(F("   "));
  lcd->print(logfile->get_file_name_ptr());
  lcd->setCursor(0, 2);
  lcd->print(F("Use arrows to set."));
  lcd->setCursor(0, 3);
  lcd->print(F("Blue to exit."));
}

bool file_callback(){
    delay(1000);
    display_file_menu();
    while(true){
    if(digitalRead(MENU_SELECT_BUTTON)==LOW){
      return false;
    } else if(digitalRead(MENU_UP_BUTTON)==LOW || digitalRead(MENU_DN_BUTTON)==LOW){
      logfile->rotate_file();
      display_file_menu();
      delay(500); // wait a  moment to check again
    }
  }
}


bool display_raw = false;
bool disp_callback(){
  Serial.println(F("Entered Display Callback"));
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(F("Press up or down to "));
  lcd->setCursor(0, 1);
  lcd->print(F("   set display type."));
  lcd->setCursor(0, 2);
  lcd->print(F("Blue button to exit."));
  lcd->setCursor(7, 3);
  if(display_raw)
    lcd->print(F("Raw    "));
  else
    lcd->print(F("Average"));


  while(true){
    if(digitalRead(MENU_SELECT_BUTTON)==LOW){
      dust->set_display_raw(display_raw);
      sensors->set_display_raw(display_raw);
      return false;
    } else if(digitalRead(MENU_UP_BUTTON)==LOW || digitalRead(MENU_DN_BUTTON)==LOW){
      display_raw = !display_raw;
      lcd->setCursor(7, 3);
      if(display_raw)
        lcd->print(F("Raw    "));
      else
        lcd->print(F("Average"));
      delay(500); // wait a  moment to check again
    }
  }
}

#define MENU_LENGTH 8
const char menu_0[] PROGMEM = "Raw / Avg"; // "String 0" etc are strings to store - change to suit.
const char menu_1[] PROGMEM = "Curr File";
const char menu_2[] PROGMEM = "Time Set ";
const char menu_3[] PROGMEM = "Rset File";
const char menu_4[] PROGMEM = "Samp Rate";
const char menu_5[] PROGMEM = "Log  Rate";
const char menu_6[] PROGMEM = "Init SD  ";
const char menu_e[] PROGMEM = "EXIT     ";
const char *const menu_line[] PROGMEM = {menu_e,         menu_0,         menu_1,         menu_2,         menu_3,         menu_4,         menu_5,         menu_6};


// Render the menue, starting with a specific line
void render_menu(uint8_t line){
  lcd->clear();
  for(int d_row=line; d_row < line + 4 && d_row < MENU_LENGTH; d_row++){
    char buffer[21];  // make sure this is large enough for the largest string it must hold
    strcpy_P(buffer, (char *)pgm_read_word(&(menu_line[d_row])));
    lcd->setCursor(COL1, d_row-line);
    lcd->print(buffer);
    Serial.print("render line "); Serial.println(d_row);
  }
  lcd->setCursor(0,0);
  lcd->cursor();
}

bool enter_menu_item(uint8_t id){
  delay(1000);
  Serial.print(F("entering menu item ")); Serial.println(id);
  bool rv = false;
  switch(id){
    case 0:
      rv = exit_callback();
      break;
    case 1:
      rv = disp_callback();
      break;
    case 2:
      rv = file_callback();
      break;
    default:
      Serial.println(F("No function exists for this menu item"));
      return false;
  }
  delay(1000);
  return rv;
}

void enter_menu(){
    uint8_t menu_pos = 0;
    render_menu(menu_pos);  // render the menu at the start
    // now
    while(true){
      // look for the up, down, or select buttons
      if(digitalRead(MENU_UP_BUTTON)==LOW && menu_pos > 0){
        menu_pos--;
        render_menu(menu_pos);
        delay(200);
      } else if(digitalRead(MENU_DN_BUTTON)==LOW && menu_pos < MENU_LENGTH-1){
        menu_pos++;
        render_menu(menu_pos);
        delay(200);
      } else if(digitalRead(MENU_SELECT_BUTTON)==LOW){
        if(enter_menu_item(menu_pos)){
          break;
        }
        else{
          lcd->clear();
          render_menu(menu_pos);
        }
      }
    }

    lcd->clear();
}










void loop() {
  loop_start_millis = millis();
  
  // Serial log start
  Serial.print(loop_number++); Serial.print(F(" ----------- ")); Serial.println(loop_start_millis);

  // Collect and print sensor data to screen
  sensors->sense_all();
  sensors->log_all_serial_only();
  dust->sense();
  dust->log_serial();


  // Wrap the file every day
  if(loop_number % (SECONDS_PER_DAY / (LOOP_PERIOD_MILLIS / 1000)) == 0){
    Serial.println(F("Wrapping the log File"));
  }

  // Log to file every N loops
  if(loop_number % LOG_EVERY_N_LOOPS == 0) {
    logfile->open_line(loop_number, loop_start_millis);
    sensors->log_all(&logfile->file);
    dust->log(&logfile->file);
    logfile->close_line();
  }
  
  // Burn remainder of the loop period
  while(millis() < loop_start_millis + LOOP_PERIOD_MILLIS) {
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
      enter_menu();
    }
  }// while(millis)
  
}
