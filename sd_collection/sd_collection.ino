#include <SPI.h>
#include <SD.h>
File myFile;
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  
  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(10)) {
    Serial.println(F("initialization failed!"));
    while (1);
  }
  Serial.println("initialization done.");

  myFile = SD.open(F("test.txt"), FILE_READ);
  if(myFile) {
    myFile.close(); // file found, skip this step
  } else {
    myFile.close(); // file not found, but close cleanly
    myFile = SD.open(F("test.txt"), FILE_WRITE);
    for(int i=0; i < 15; i++) {
      myFile.println(i);  // write some data on lines
    }
    myFile.close();
  }


  
  // open the file for reading:
  myFile = SD.open(F("test.txt"));
  if (myFile) {
    Serial.println(F("test.txt:"));
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
      // close the file:
      myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening test.txt"));
  }
}
void loop() {
// nothing happens after setup
}
