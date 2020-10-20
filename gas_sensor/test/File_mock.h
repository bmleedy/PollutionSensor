#define FILE_WRITE 0
#define FILE_READ 1

// Fake class to mock and make assertions about
// this is necessary because we're not actually able 
// to build the SD library for this platform,
// or rather won't get value out of spending the time.
//
class File {
 public:				
  File(){}
  char * name(){return NULL;}
	File openNextFile(){return File();}
	int open(){return 0;}
	int close(){return 0;}
	void print(int t){}
	void print(const char[]){}
	void println(const char[]){}
	bool isDirectory(){return false;}
	operator int() const {return 0;}
};


// https://github.com/arduino-libraries/SD/blob/master/src/SD.h
class SDClass {
 public:				
  bool begin(uint8_t csPin){return true;}
	File open(char * name, int mode){return File();} //todo: make not broken
};

