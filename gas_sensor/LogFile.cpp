// Copyright 2020 Brett M. Leedy

#include "LogFile.h"

// todo: create unit tests, especially for string manipulation, which scares me.

LogFile::LogFile() {
  this->re_init_sd();
}

bool LogFile::re_init_sd() {
  // don't try to re-init if cooldown has not expired
  if ((millis() > this->cooldown_start_millis) &&  // millis hasn't wrapped
      (millis() - this->cooldown_start_millis <
                          SD_COOLDOWN_LENGTH) &&   // waited for the cooldn
      (this->cooldown_start_millis != 0)           // we've run this once
    ) {
    return this->sd_failure;
  } else {
    this->cooldown_start_millis = millis();  // we  ran, so reset
  }

  // Init the SD card
  pinMode(10, OUTPUT);
  Serial.println(F("LogFile: Init SD card..."));
  this->sd_failure = !SD.begin(10);  // begin returns false on failure

  if (!sd_failure) {
    // Init the file name
    current_id = this->get_highest_used_id();
    snprintf(current_name, MAX_FILENAME_LEN, "%s-%d.%s",
               log_file_name_base, ++current_id, LOGFILE_EXTENSION);
    Serial.print(F("Logfile: File name is "));
    Serial.print(current_name); Serial.println("|");
  } else {
    // Failed to init SD, print error and fail out
    Serial.println(F("LogFile: ERROR - Failed to init SD card!"));
  }
  return this->sd_failure;
}

void LogFile::get_file_name(char * buffer, uint8_t max_size) {
  uint8_t maxlen = MAX_FILENAME_LEN;
  if (max_size < maxlen)
    maxlen = max_size;   // take the smallest max size

  strncpy(buffer, this->current_name, max_size);
}

char * LogFile::get_file_name_ptr() {
  return this->current_name;
}

bool LogFile::is_sd_failed() {
  if (sd_failure)      // check whether we're marked for failure
    if (re_init_sd())  // attempt to re-init
      return true;    // return true of re-init fails

  return false;  // otherwise, return false
}

void LogFile::open_line(uint16_t id, uint16_t timestamp) {
  if (is_sd_failed())
    return;
  this->file.close();
  this->file = SD.open(this->current_name, FILE_WRITE);
  if (this->file) {
    // success
  } else {
    Serial.println(F("error opening log file:"));
    Serial.println(this->current_name);
    Serial.println(this->file);
    this->sd_failure = true;
  }

  this->file.print(id);
  this->file.print(F(","));
  this->file.print(timestamp);
  this->file.print(F(","));
}

uint16_t LogFile::get_highest_used_id() {
  if (is_sd_failed())
    return 0;

  uint16_t highest_number_found = 0;
  File dir;
  dir = SD.open("/", FILE_READ);
  // check that it opened and is a directory
  if (!dir) {
    // could not open directory
    Serial.println(F("Could not open directory"));
    return 0;
  } else if (!dir.isDirectory()) {
    // not a directory
    Serial.println(F("Not a directory"));
    return 0;
  }

  // check that it is a dir
  char temp_filename[MAX_FILENAME_LEN];
  uint16_t temp_filename_len = MAX_FILENAME_LEN;
  while (true) {
    File entry =  dir.openNextFile();
    if (entry == 0)
      break;  // no more files

    // if entry.name() matches our file prefix, check the highest number
    // copy the string to tokenize
    uint16_t temp_filename_len = MAX_FILENAME_LEN;
    char * token;
    strncpy(temp_filename, entry.name(), MAX_FILENAME_LEN);
    // string is of the format "LOGFILE_NAME_BASE.number.csv"
    token = strtok(temp_filename, "-");

    // compare the first token to the filename
    if (strncmp(token, log_file_name_base, temp_filename_len) != 0) {
      continue;  // different base name - ignore
    }

    token = strtok(NULL, ".");
    // convert and compare the number token (if found)
    uint16_t file_number = atoi(token);
    if (file_number > highest_number_found) {
      highest_number_found = file_number;
    }
    entry.close();
  }  // while(true)

  dir.close();
  Serial.print(F("Highest file number found: "));
  Serial.println(highest_number_found);
  return highest_number_found;
}

void LogFile::close_line() {
  if (is_sd_failed())
    return;
  this->file.println("");
  this->file.close();
}

void LogFile::rotate_file() {
  this->override_file_number(this->current_id+1);
}

void LogFile::override_file_number(uint16_t new_id) {
  this->current_id = new_id;

  snprintf(current_name, MAX_FILENAME_LEN, "%s-%d.%s",
           log_file_name_base, current_id,
           LOGFILE_EXTENSION);

  Serial.print(F("Logfile: File name is now")); Serial.println(current_name);
}

File * LogFile::get_file_ptr() {
  // todo: make this safer
  return &this->file;
}
