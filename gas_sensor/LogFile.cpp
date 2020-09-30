#include "LogFile.h"


LogFile::LogFile(){
  // Init the SD card
  pinMode(10, OUTPUT);
  Serial.println(F("LogFile: Init SD card..."));
  SD.begin(10);

//    // Open or create the directory todo: fix directory creation
//    Serial.println(F("LogFile: Init SD directory..."));
//    if(!SD.mkdir(LOG_DIRECTORY)){
//      Serial.println(F("SD directory creation failed!"));
//      failure = SD_DIR_FAILURE;
//      return;
//    }

  // Init the file name
  current_id = this->get_highest_used_id();
  snprintf(current_name, MAX_FILENAME_LEN, "%s-%d.%s", log_file_name_base, ++current_id, LOGFILE_EXTENSION);
  Serial.print(F("Logfile: File name is ")); Serial.print(current_name); Serial.println("|");
  
}

void LogFile::open_line(uint16_t id, uint16_t timestamp){
  this->file.close();
  this->file = SD.open(this->current_name, FILE_WRITE);
  if (this->file) {
    // success
  } else {
    Serial.println(F("error opening log file:"));
    Serial.println(this->current_name);
    Serial.println(this->file);
  }

  this->file.print(id);
  this->file.print(F(","));
  this->file.print(timestamp);
  this->file.print(F(","));
}

uint16_t LogFile::get_highest_used_id(){
  uint16_t highest_number_found = 0;
  File dir;
  dir = SD.open("/", FILE_READ);
  Serial.println(dir);
  Serial.println(dir.name());
  //check that it opened and is a directory
  if(!dir) {
    //could not open directory
    Serial.println(F("Could not open directory"));
    return 0;
  } else if (!dir.isDirectory()){
    //not a directory
    Serial.println(F("Not a directory"));
    return 0;
  }
  
  //check that it is a dir
  char temp_filename[MAX_FILENAME_LEN];
  uint16_t temp_filename_len = MAX_FILENAME_LEN;
  while(true){
    File entry =  dir.openNextFile();
    if (entry == 0)
      break; // no more files
      
    // if entry.name() matches our file prefix, check the highest number
    // copy the string to tokenize
    uint16_t temp_filename_len = MAX_FILENAME_LEN;
    char * token;
    strncpy(temp_filename, entry.name(), MAX_FILENAME_LEN);
    Serial.println(temp_filename);
    // string is of the format "LOGFILE_NAME_BASE.number.csv"
    token = strtok(temp_filename, "-");

    // compare the first token to the filename
    if(strncmp(token, log_file_name_base, temp_filename_len) != 0){
      Serial.print(F("invalid base: ")); Serial.println(token);
      // continue;  //different base name - ignore
    } else {
      Serial.print(F("valid base: ")); Serial.println(token);
    }

    
    token = strtok(NULL, ".");  // todo: can this just be next_token?
    // convert and compare the number token (if found)
    Serial.print(F("number token is: ")); Serial.println(token);
    uint16_t file_number = atoi(token);
    if(file_number > highest_number_found){
      Serial.print(F("integer is: ")); Serial.println(file_number);
      highest_number_found = file_number;
    }
    entry.close();
  }//while(true)

  dir.close();
  Serial.print(F("Highest file number found: ")); Serial.println(highest_number_found);
  return highest_number_found;
}

  void LogFile::close_line(){  //todo:check that the file is  open
    this->file.println("");
    this->file.close();
  }

  void LogFile::rotate_file(){
    this->override_file_number(this->current_id+1);
  }

  void LogFile::override_file_number(uint16_t new_id){
    this->current_id = new_id;
  }
