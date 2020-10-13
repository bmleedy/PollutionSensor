#ifndef SENSOR_MENU_H
#define SENSOR_MENU_H



// Menu Buttons
#define MENU_SELECT_BUTTON 7
#define MENU_UP_BUTTON     6
#define MENU_DN_BUTTON     5

#define MENU_LENGTH 5
// todo: this whole thing is hokey, and I should be defining these as menu item classes 
//   and instantiating them with a factory pattern.
const char menu_e[]      PROGMEM = "EXIT     ";              // 0
const char menu_raw[]    PROGMEM = "Disp. Raw / Average";    // 1
const char menu_file[]   PROGMEM = "File";                   // 2
const char menu_sam[]    PROGMEM = "Sampling Rate";          // 3
const char menu_log[]    PROGMEM = "File logging Rate";      // 4
const char menu_back[]   PROGMEM = "Toggle LCD Backlight";   // 5
const char menu_logon[]  PROGMEM = "Toggle file log";        // 6
const char menu_lpg[]    PROGMEM = "LPG Threshold";          // 7
const char menu_co[]     PROGMEM = "CO Threshold";           // 8
const char menu_ozone[]  PROGMEM = "Ozone Threshold";        // 9
const char menu_gas[]    PROGMEM = "Gas Threshold";          // 10
const char menu_poison[] PROGMEM = "Poison Threshold";       // 11
const char menu_dust_z[] PROGMEM = "Dust Zero";              // 12

const char *const menu_line[] PROGMEM = {menu_e,      //  0: Exit
                                         menu_raw,    //  1: Raw or averaged data
                                         menu_file,   //  2: Increment the file name 
                                         menu_sam,    //  3: Set sampling frequency
                                         menu_log,    //  4: Set logging rate (every n samples)};
                                         menu_back,   //  5: Toggle LCD backlight
                                         menu_logon,  //  6: Toggle logging to file
                                         menu_lpg,    //  7: LPG Gas Sensor
                                         menu_co,     //  8: Carbon Monoxide Sensor
                                         menu_ozone,  //  9: Ozone Sensor
                                         menu_gas,    // 10: Gas Sensor
                                         menu_poison, // 11: Hazardous Gas Sensor
                                         menu_dust_z, // 12: Dust sensor adjustment
                                         };

#define DEFAULT_LOOP_PERIOD_MILLIS 2000  // every one second
#define DEFAULT_LOG_EVERY_N_LOOPS    10  // every ten seconds

struct settings_type{
  uint16_t sampling_period_ms = DEFAULT_LOOP_PERIOD_MILLIS;
  uint16_t log_every_n_loops = DEFAULT_LOG_EVERY_N_LOOPS;
  uint16_t file_number = 0;

  uint16_t dust_zero = 0;
  uint16_t lpg_threshold = 800;
  uint16_t co_threshold = 800;
  uint16_t ozone_threshold = 800;
  uint16_t gas_threshold = 800;
  uint16_t hazard_threshold = 800;

  bool log_raw = false;
  bool backlight = true;
  bool logging = true;

  uint16_t checksum = 0;

  uint16_t calc_checksum(){
    return sampling_period_ms +
               log_every_n_loops +
               log_raw +
               file_number + 
               + backlight +
               198; // salt value to avoid all-zeros hazard 
  }
  void store_checksum(){
    checksum = calc_checksum();
  }
  bool check(){
    return (checksum == calc_checksum());
  }
};



class SensorMenu{
  LiquidCrystal_I2C * lcd;
  LogFile * logfile;
  AnalogSensor * sensors;
  SmokeSensor * dust;
  uint8_t col1, col2;
  settings_type config;

 public:
  SensorMenu(LiquidCrystal_I2C * lcd,
                LogFile * logfile,
                AnalogSensor * sensors,
                SmokeSensor * dust,
                uint8_t col1_idx,
                uint8_t col2_idx){
    this->lcd = lcd;
    this->logfile = logfile;
    this->sensors = sensors;
    this->dust = dust;
    this->col1 = col1_idx;
    this->col2 = col2_idx;

    // retrieve config values from the EEPROM
    EEPROM.get(0, this->config);
    if(this->config.check()){
      Serial.println(F("Configuration Loaded:"));
      Serial.print(F("  sampling_period_ms: ")); Serial.println(this->config.sampling_period_ms);
      Serial.print(F("  log_every_n_loops: ")); Serial.println(this->config.log_every_n_loops);
      Serial.print(F("  log_raw (boolean): ")); Serial.println(this->config.log_raw);
      Serial.print(F("  file_number: ")); Serial.println(this->config.file_number);
      Serial.print(F("  backlight: ")); Serial.println(this->config.backlight);
      Serial.print(F("  logging: ")); Serial.println(this->config.logging);
      Serial.print(F("  checksum: ")); Serial.println(this->config.checksum);
      Serial.print(F("  calculated checksum: ")); Serial.println(this->config.calc_checksum());
    } else {
      Serial.println(F("Checksum failed! Writing default config."));
      this->config.sampling_period_ms = DEFAULT_LOOP_PERIOD_MILLIS;
      this->config.log_every_n_loops = DEFAULT_LOG_EVERY_N_LOOPS;
      this->config.log_raw = false;
      this->config.file_number = 0;
      this->config.backlight = true;
      this->config.logging = true;
      commit_config();  //write to EEPROM with valid checksum
    }
  }



  bool display_sensor_setting(const char * name, uint16_t * setting){
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(name);
    lcd->setCursor(0, 1);
    lcd->print(F("Setting : "));
    lcd->print(*setting);
    lcd->setCursor(0, 3);
    lcd->print(F("Blue to exit."));
  }


  bool sensor_settings_callback(const char * name, uint16_t * setting){
    Serial.print(F("Entered settings callback for ")); Serial.println(name);

    this->display_sensor_setting(name, setting);

    delay(1000);
    while(true){
      if(digitalRead(MENU_SELECT_BUTTON)==LOW){
        commit_config();  //write to EEPROM before exiting
        return false;
      } else if(digitalRead(MENU_UP_BUTTON)==LOW){
        *setting += 20;
        this->display_sensor_setting(name, setting);
        delay(200);
      } else if(digitalRead(MENU_DN_BUTTON)==LOW){
        if( *setting >= 20)
          *setting -= 20;
        this->display_sensor_setting(name, setting);
        delay(200);
      }
    }
  
  }

  bool exit_callback(){
    return true;
  }

  void commit_config(){
    this->config.store_checksum();
    EEPROM.put(0, this->config);
  }

  uint16_t get_log_every_n_loops(){
    return this->config.log_every_n_loops;
  }

  bool get_backlight_config(){
    return this->config.backlight;
  }

  bool backlight_callback(){
    this->config.backlight = !this->config.backlight;
    return true;  // exit back out to main display
  }

  bool get_logon_config(){
    return this->config.logging;
  }

  bool logon_callback(){
    this->config.logging = !this->config.logging;
    return true;  // exit back out to main display
  }
  
  void display_lograte_menu(){
    Serial.println(F("Entered log rate Callback"));
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(F("Logging every N "));
    lcd->setCursor(0, 1);
    lcd->print(F("samples:  "));
    lcd->print(this->config.log_every_n_loops);
    lcd->setCursor(0, 2);
    lcd->print(F("Use arrows to set."));
    lcd->setCursor(0, 3);
    lcd->print(F("Blue to exit."));
  }
  
  bool lograte_callback(){
      display_lograte_menu();
      delay(1000);
      while(true){
        if(digitalRead(MENU_SELECT_BUTTON)==LOW){
          commit_config();  //write to EEPROM before exiting
          return false;
        } else if(digitalRead(MENU_UP_BUTTON)==LOW){
          this->config.log_every_n_loops++;
          this->display_lograte_menu();
          delay(500);
        } else if(digitalRead(MENU_DN_BUTTON)==LOW){
          if( this->config.sampling_period_ms > 1)
            this->config.log_every_n_loops--;
          this->display_lograte_menu();
          delay(500);
        }
      }
  }


  
  void display_sampling_menu(){
    Serial.println(F("Entered sample rate Callback"));
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(F("Current sample rate:"));
    lcd->setCursor(0, 1);
    lcd->print(F("Period, ms: "));
    lcd->print(this->config.sampling_period_ms);
    lcd->setCursor(0, 2);
    lcd->print(F("Use arrows to set."));
    lcd->setCursor(0, 3);
    lcd->print(F("Blue to exit."));
  }
  
  bool sampling_callback(){
      display_sampling_menu();
      delay(1000);
      while(true){
        if(digitalRead(MENU_SELECT_BUTTON)==LOW){
          commit_config();  //write to EEPROM before exiting
          return false;
        } else if(digitalRead(MENU_UP_BUTTON)==LOW){
          this->config.sampling_period_ms += 500;
          this->display_sampling_menu();
          delay(500);
        } else if(digitalRead(MENU_DN_BUTTON)==LOW){
          if( this->config.sampling_period_ms >= 1000)
            this->config.sampling_period_ms -= 500;
          this->display_sampling_menu();
          delay(500);
        }
      }
  }


  uint16_t get_sampling_period_ms(){
    return this->config.sampling_period_ms;
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
      display_file_menu();
      delay(1000);
      while(true){
      if(digitalRead(MENU_SELECT_BUTTON)==LOW){
        // commit_config();  //write to EEPROM before exiting
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
  
  
  // Render the menue, starting with a specific line
  void render_menu(uint8_t line){
    lcd->clear();
    for(int d_row=line; d_row < line + 4 && d_row < MENU_LENGTH; d_row++){
      char buffer[21];  // make sure this is large enough for the largest string it must hold
      strcpy_P(buffer, (char *)pgm_read_word(&(menu_line[d_row])));
      lcd->setCursor(col1, d_row-line);
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
      case 0:  rv = exit_callback(); break;
      case 1:  rv = disp_callback(); break;
      case 2:  rv = file_callback(); break;
      case 3:  rv = sampling_callback(); break;
      case 4:  rv = lograte_callback(); break;
      case 5:  rv = backlight_callback(); break;
      case 6:  rv = logon_callback(); break;
      case 7:  rv = sensor_settings_callback("LPG", &this->config.lpg_threshold);    break;//  7: LPG Gas Sensor
      case 8:  rv = sensor_settings_callback("CO",  &this->config.co_threshold);     break;//  8: Carbon Monoxide Sensor
      case 9:  rv = sensor_settings_callback("O3",  &this->config.ozone_threshold);  break;//  9: Ozone Sensor
      case 10: rv = sensor_settings_callback("GAS", &this->config.gas_threshold);    break;// 10: Gas Sensor
      case 11: rv = sensor_settings_callback("HAZ", &this->config.hazard_threshold); break;// 11: Hazardous Gas Sensor
      case 12: rv = sensor_settings_callback("PM",  &this->config.dust_zero);        break;// 12: Dust sensor adjustment
      default:
        Serial.println(F("No function exists for this menu item"));
        return false;
    }
    delay(1000);
    return rv;
  }
  
  void enter_menu(){
    lcd->backlight();
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
    
    if(!this->config.backlight)
      lcd->noBacklight();  // turn off the backlight if it's configed off.
    
    lcd->clear();
  }
  
};




#endif
