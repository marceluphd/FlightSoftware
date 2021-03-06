//SD_DataFile.cpp

#include "SD_DataFile.h"
#include <SD.h>

//constructor, sets up file type and name
SD_DataFile::SD_DataFile(int cs_pin, uint16_t dw, String fn) {
  _chipSelectPin = cs_pin;
  _dataWidth = dw;
  _fileName = fn;
  _numEntries = 0;
  
  pinMode(_chipSelectPin, OUTPUT);
  digitalWrite(_chipSelectPin, HIGH);
}

//refreshes the numEntries variable
//TODO: do a better job of initializing the DataFile object so this isn't necessary
bool SD_DataFile::refresh() {
  SD.begin(_chipSelectPin);
  digitalWrite(_chipSelectPin, LOW);
  _dataFile = SD.open(_fileName);
  if(_dataFile){
    _numEntries = _dataFile.size() / _dataWidth;
    _dataFile.close();
    digitalWrite(_chipSelectPin, HIGH);
    return true;
  }
  digitalWrite(_chipSelectPin, HIGH);
  return false;
}

//writes from the passed array into the next data entry slot on the data file
//make sure the passed array is the right size
//returns true if success, false otherwise
bool SD_DataFile::writeDataEntry(byte* data) {
  SD.begin(_chipSelectPin);
  digitalWrite(_chipSelectPin, LOW);
  _dataFile = SD.open(_fileName, FILE_WRITE);
  if (_dataFile) {
    _dataFile.write(data, _dataWidth);
    _dataFile.close();
    _numEntries++;
    digitalWrite(_chipSelectPin, HIGH);
    return true;
  }
  digitalWrite(_chipSelectPin, HIGH);
  return false;
}

//reads the data file entry specified by the int index into the buffer buf
//returns true if success, false otherwise
bool SD_DataFile::readDataEntry(int index, byte* buf) {
  SD.begin(_chipSelectPin);
  digitalWrite(_chipSelectPin, LOW);
  if (index < _numEntries) {
    _dataFile = SD.open(_fileName, FILE_READ);
    if (_dataFile) {
      _dataFile.seek(index * _dataWidth);
      _dataFile.read(buf, _dataWidth);
      _dataFile.close();
      digitalWrite(_chipSelectPin, HIGH);
      return true;
    }
  }
  digitalWrite(_chipSelectPin, HIGH);
  return false;
}

bool SD_DataFile::readLineIndex(int lineNum, int index, int len, byte* buf){
  if (lineNum < _numEntries) {
    SD.begin(_chipSelectPin);
    digitalWrite(_chipSelectPin, LOW);
    _dataFile = SD.open(_fileName, FILE_READ);
    if (_dataFile) {
      _dataFile.seek((lineNum * _dataWidth) + index);
      _dataFile.read(buf, len);
      _dataFile.close();
      digitalWrite(_chipSelectPin, HIGH);
      return true;
    }
    digitalWrite(_chipSelectPin, HIGH);
  }
  return false;
}
