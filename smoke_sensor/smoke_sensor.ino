#include <LiquidCrystal.h>
// best note for sample code: https://tinyurl.com/y6xhshw4
// https://tinyurl.com/yxa48kty
// pms7003 sensor: https://www.espruino.com/PMS7003
// Easy chart of pm10 vs pm2.5:https://tinyurl.com/y3fkjgo7
//www.diyusthad.com
// US AQI standard: https://diyprojects.io/calculate-air-quality-index-iaq-iqa-dsm501-arduino-esp8266/

// LCD Display configuration
const int rs = 12, en = 11, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Dust Sensor Control Configuration
int dustPin=A0;
int ledPower=13;  // Pin D13
int delayTime=280;
int delayTime2=40;
float offTime=9680;

// Dust Sensor Calibration Constants
#define DUST_ZERO 0.48      // Volts, 0.5 typical
#define DUST_OFFSET_CAL 0   // offset to pm2.5 value n 
#define DUST_SLOPE_CAL 1.65 // multiplier for pm2.5 value


// Display content constants and variables
#define DESC_LEN 30         // max length for AQI Description
#define ACCUM_FRACTION 0.05  // lower = more smooth filtering
char first_line[DESC_LEN];  // holds the first display line
char second_line[DESC_LEN]; // holds the second display line
char dust_desc[DESC_LEN];
int display_scroll = 0;

// Button to re-start fast sampling
#define RESAMPLE_BUTTON 8  // digital 8 button

// Loop speed configuration
#define SLOW_SAMPLING_DELAY_MS 9000  // sample every 10 seconds
#define NUM_FAST_ITERATIONS 60 // take 60 samples fast

void setup(){ //////////////////////////////////////////////////////////
  // Communication with the host computer
  Serial.begin(115200);

  // Sensor pins
  pinMode(ledPower,OUTPUT);
  pinMode(dustPin, INPUT);

  // LCD
  lcd.begin(16, 2);  // columns, rows
  lcd.print("STARTUP");
  lcd.setCursor(0,1);  // column, row
  lcd.print("...  ...  ...  ");
  lcd.home();  // reposition cursor

  // Resample Button
  pinMode(RESAMPLE_BUTTON, INPUT_PULLUP);
}

uint8_t loop_iterations = 0; // only needs to hold max value of 61
float dust_accum = 20.0;  // current dust sensor value, averaged

void loop(){ //////////////////////////////////////////////////////////
  digitalWrite(ledPower,LOW); // power on sensor LED
  delayMicroseconds(delayTime); // wait for data collection
  float dustVal=analogRead(dustPin); // read sensor value
  delayMicroseconds(delayTime2);  // wait for a settling time
  digitalWrite(ledPower,HIGH);  // power off the sensor LED
  delayMicroseconds(offTime); // delay for the remainder of a second
  delay(500);  // base delay time

  
  // Slow ourselves down to prevent sensor wear, unless button is pressed
  int timer_iterations = 0;
  if(loop_iterations > NUM_FAST_ITERATIONS) {
    while(digitalRead(RESAMPLE_BUTTON) && (timer_iterations < SLOW_SAMPLING_DELAY_MS)){
      delay(1); // Run more
      timer_iterations++;
      if(timer_iterations % 500 == 0)
        Serial.print('.');
    }
    Serial.println("");
  }
  else {
    loop_iterations++; // only count up to 61 then stop
  }

  
  // Check the resample button
  if(!digitalRead(RESAMPLE_BUTTON)){
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
