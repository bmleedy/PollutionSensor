// Copyright 2014 http://switchdevice.com
// This example code is in the public domain.

#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "arduino-mock/Serial.h"
//#include "arduino-mock/SD.h"
#include "arduino-mock/SPI.h"
#include "arduino-mock/EEPROM.h"
#include "arduino-mock/Wire.h"

//#define File file
#include "File_mock.h"

SDClass SD;

#include "../LogFile.h"
#include "../LogFile.cpp"

using ::testing::Return;
TEST(testsuite, testname) {
  ArduinoMock* arduinoMock = arduinoMockInstance();
  SerialMock* serialMock = serialMockInstance();
  //EXPECT_CALL(*arduinoMock, digitalRead(2))
  //  .WillOnce(Return(1));
  //EXPECT_CALL(*serialMock, println(1, 10));
  EXPECT_CALL(*arduinoMock, delay(1));
  //loop();
  releaseSerialMock();
  releaseArduinoMock();
}

