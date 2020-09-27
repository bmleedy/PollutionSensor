#include<Arduino.h>
#include<Wire.h>
#include <LiquidCrystal_I2C.h>


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
 ** ! MQ-131 - Ozone - https://www.sparkfun.com/products/17051
 * MQ-135 - Harmful Gasses https://components101.com/sensors/mq135-gas-sensor-for-air-quality
 ** ! MQ-136 - Hydrogen Sulfide - https://www.sparkfun.com/products/17052
 ** ! MQ-137 - Ammonia - https://www.sparkfun.com/products/17053 
 * ! Multi-channel gas sensor: https://wiki.seeedstudio.com/Grove-Multichannel_Gas_Sensor/
 */

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,16,2);

void setup() {
  // put your setup code here, to run once:
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);

  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.clear();

}

#define COL1 0
#define COL2 9

uint16_t v0, v1, v2, v3, v6, v7 = 0;

void loop() {
  // put your main code here, to run repeatedly:
  v0 = analogRead(A0);
  Serial.print(F("A0 (MQ3 -     Alcohol): ")); Serial.println(v0);
  lcd.setCursor(COL1,0);  // column, row
  lcd.print(F("Alc.:")); lcd.print(v0);
  
  v1 = analogRead(A1);
  Serial.print(F("A1 (MQ5 -         LPG): ")); Serial.println(v1);
  lcd.setCursor(COL1,1);  // column, row
  lcd.print(F("LPG: ")); lcd.print(v1);
  
  v2 = analogRead(A2);
  Serial.print(F("A2 (MQ7 -          CO): ")); Serial.println(v2);
  lcd.setCursor(COL1,2);  // column, row
  lcd.print(F("CO:  ")); lcd.print(v2);
  
  v3 = analogRead(A3);
  Serial.print(F("A3 (MQ9 -    Gas Leak): ")); Serial.println(v3);
  lcd.setCursor(COL2,1);  // column, row
  lcd.print(F("Gas:   ")); lcd.print(v3);
  
  v6 = analogRead(A6);
  Serial.print(F("A6 (MQ135 - Hazardous): ")); Serial.println(v6);
  lcd.setCursor(COL2,0);  // column, row
  lcd.print(F("Poison:")); lcd.print(v6);
  
  //v7 = analogRead(A7);
  //Serial.print(F("A7 (MQ8 -    Hydrogen): ")); Serial.println(v7);
  Serial.println(F("-----------"));
  

  delay(2000);

}
