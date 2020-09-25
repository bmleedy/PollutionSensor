/*-------------------------------------------------------------
  smoke_sensor

  This code drives a Sharp GP2Y1010AU0F sensor with control of 
  a fan and LCD display.  

  For calibration, achart of pm10 vs pm2.5:https://tinyurl.com/y3fkjgo7
  Best note for sample code: https://tinyurl.com/y6xhshw4
  US AQI standard: https://tinyurl.com/yxa48kty
 -------------------------------------------------------------*/

// Define if we're using I2C module (rather than 4-bit GPIO) to control
//   the display
#define I2C_DISPLAY

// Both displays use the same API, so we only need to include the
//   correct library and initialize properly
#ifdef I2C_DISPLAY
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,16,2);
#else
  #include <LiquidCrystal.h>
  // LCD Display configuration
  const int rs = 12, en = 11, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
  LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
#endif

// Dust Sensor Physical Configuration
int dustPin=A0;
int ledPower=13;  // Pin D13
int delayTime=280;
int delayTime2=40;
float offTime=9680;

// Dust Sensor Calibration Constants
#define DUST_ZERO 0.48      // Volts, 0.5 typical
#define DUST_OFFSET_CAL 0   // offset to pm2.5 value n 
#define DUST_SLOPE_CAL 1.65 // multiplier for pm2.5 value

// LCD Display content constants and variables
#define DESC_LEN 30         // max length for AQI Description
#define ACCUM_FRACTION 0.05  // lower = more smooth filtering
char first_line[DESC_LEN];  // holds the first display line
char second_line[DESC_LEN]; // holds the second display line
char dust_desc[DESC_LEN];
int display_scroll = 0;

// Control pin for NPN transistor for fan power
#define FAN_CONTROL_PIN 5

// Button to re-start fast sampling (optional to hook up)
#define RESAMPLE_BUTTON 8  // digital 8 button

// Loop speed configuration
#define SLOW_SAMPLING_DELAY_MS 59000  // sample every 60 seconds
#define SLOW_SAMPLING_FAN_LEAD 10000   // fan on 10 seconds before taking sample
#define NUM_FAST_ITERATIONS 60 // take 60 samples fast when powered up

//////////////////////////////////////////////////////////
void setup(){ 
  // Communication with the host computer
  Serial.begin(115200);

  // Sensor pins
  pinMode(ledPower,OUTPUT);  // Turns sensor LED on/off
  pinMode(dustPin, INPUT);   // Analog input


  // set npn transistor high (fan on) to start
  pinMode(FAN_CONTROL_PIN,OUTPUT);
  digitalWrite(FAN_CONTROL_PIN,HIGH);
  
  // LCD Initialization
#ifdef I2C_DISPLAY
  lcd.init();
  lcd.backlight();
#else
  lcd.begin(16, 2);  // columns, rows
#endif
  lcd.print("STARTUP");
  lcd.setCursor(0,1);  // column, row
  lcd.print("...  ...  ...  ");
  lcd.home();  // reposition cursor


  // Init the resample button (does nothing if not hooked up)
  pinMode(RESAMPLE_BUTTON, INPUT_PULLUP);
}


// slow_sample_delay()
//   turns the fan and lcd off and waits for sampline_delay ms. 
//   then turns both on, waits a bit, and then exits

void slow_sample_delay(uint16_t sampling_delay, uint16_t fan_lead){
  uint16_t timer_iterations = 0;
  
  while(digitalRead(RESAMPLE_BUTTON) && (timer_iterations < sampling_delay)){
    delay(1); // Run more
    if(timer_iterations > (sampling_delay - fan_lead) ) {
      digitalWrite(FAN_CONTROL_PIN,HIGH); // fan on
      lcd.backlight();
    }
    else {
      digitalWrite(FAN_CONTROL_PIN,LOW);  // fan off
      lcd.noBacklight();
    }
      
    timer_iterations++;
      
    if(timer_iterations % 500 == 0)
      Serial.print('.');
  }
  Serial.println("");
}


uint8_t loop_iterations = 0; // only needs to hold max value of 61
float dust_accum = 20.0;  // current dust sensor value, averaged

void loop(){ //////////////////////////////////////////////////////////
  digitalWrite(FAN_CONTROL_PIN,HIGH); // fan on
  digitalWrite(ledPower,LOW); // power on sensor LED
  delayMicroseconds(delayTime); // wait for data collection
  int dustVal=analogRead(dustPin); // read sensor value
  delayMicroseconds(delayTime2);  // wait for a settling time
  digitalWrite(ledPower,HIGH);  // power off the sensor LED
  delayMicroseconds(offTime); // delay for the remainder of a second
  delay(500);  // base delay time

  
  // Slow ourselves down to prevent sensor wear, unless button is pressed
  uint16_t timer_iterations = 0;
  if(loop_iterations > NUM_FAST_ITERATIONS) {
    slow_sample_delay(SLOW_SAMPLING_DELAY_MS, SLOW_SAMPLING_FAN_LEAD);
  }
  else {
    // I'm in the initial loop, make sure the fan and backlight is on
    loop_iterations++; // only count up to 61 then stop
    digitalWrite(FAN_CONTROL_PIN,HIGH); // fan on
    lcd.backlight();
  }

  // Check the resample button
  if(!digitalRead(RESAMPLE_BUTTON)){
    digitalWrite(FAN_CONTROL_PIN,HIGH);  // Fan on
    Serial.println(F("Fast Resampling"));
    loop_iterations = 0;
  }
  
  // Compute voltage of the dust sensor
  float dust_volts = float(dustVal/1024.0*5.0);
  float dust_relative_volts = dust_volts - DUST_ZERO;
  // Sensitivity: 0.5V/(0.1mg/m3) = ( 0.5v/100ugmc) = 1v / 200ugmc
  float dust_pm25 = dust_relative_volts * 200 * DUST_SLOPE_CAL + DUST_OFFSET_CAL;
  dust_accum = dust_accum*(1.0 - ACCUM_FRACTION) + dust_pm25 * ACCUM_FRACTION ;
  
  // Select the AQI Classification for PM2.5
  float dust_rating = dust_accum;
  if(dust_rating > 250.0){
    snprintf(dust_desc, DESC_LEN, "Hazardous (Death)");
  } else if(dust_rating > 150) {
    snprintf(dust_desc, DESC_LEN, "V. Unhealthy (Purple)");
  } else if(dust_rating > 55){
    snprintf(dust_desc, DESC_LEN, "Unhealthy (Red)");
  } else if(dust_rating > 35) {
    snprintf(dust_desc, DESC_LEN, "USG (Orange)");
  } else if(dust_rating > 12) {
    snprintf(dust_desc, DESC_LEN, "Moderate (Yellow)");
  } else {
    snprintf(dust_desc, DESC_LEN, "Good (Green)");
  }
  
  // Report back via serial port
  Serial.print(F("Quality is ")); 
  Serial.print(dust_desc);
  Serial.print(" ");
  Serial.print(dust_volts);
  Serial.print("v -- ");
  Serial.print(dust_accum);
  Serial.print(" (PM2.5 AVG) -- ");
  Serial.print(dust_pm25);
  Serial.print("(PM2.5) -- ");
  Serial.println("");

  // Display to the LCD display
  if(display_scroll > 9)
    display_scroll = 0;
  else
    display_scroll++;  

  snprintf(first_line, DESC_LEN, "AQI %s", dust_desc);
  snprintf(second_line, DESC_LEN, "PM %dav, %dnow", (int)dust_accum, (int)dust_pm25);
  lcd.home();
  lcd.clear();
  lcd.print(first_line+display_scroll);
  lcd.setCursor(0,1);
  lcd.print(second_line);
}
