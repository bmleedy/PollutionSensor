#ifndef LOGFILE_H
#define LOGFILE_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#define MAX_FILENAME_LEN 12 //todo: check that max filename is larger than base name plus extensions
const char log_file_name_base[] = "LOG";
#define LOGFILE_EXTENSION "CSV"

class LogFile{
  char current_name[MAX_FILENAME_LEN]; 
  uint16_t current_id=0;

 public:
  File file;  //todo: make not public
  LogFile();
  void rotate_file();
  void open_line(uint16_t id, uint16_t timestamp);
  void close_line();
  void get_file_name(char * buffer, uint8_t max_size);
  char * get_file_name_ptr();

 private:
  uint16_t get_highest_used_id();
  void override_file_number(uint16_t new_id);
};

#endif
