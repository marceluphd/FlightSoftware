//**********************************************************
// KickSat_Sensor.cpp
//**********************************************************
//TODO: 


#include <KickSat_Sensor.h>
#include <SPI.h>

#define Serial SerialUSB
// #define DEBUG

//constructor, sets up this sensor object with the corresponding config file
KickSat_Sensor::KickSat_Sensor(uint8_t adc_rst) {
  pinMode(SPI_CS_XTB1, OUTPUT);
  pinMode(SPI_CS_XTB2, OUTPUT);
  pinMode(SPI_CS_XTB3, OUTPUT);
  pinMode(XTB_RESET, OUTPUT);
  digitalWrite(SPI_CS_XTB1, HIGH);
  digitalWrite(SPI_CS_XTB2, HIGH);
  digitalWrite(SPI_CS_XTB3, HIGH);
  digitalWrite(XTB_RESET, HIGH);
}

//this is the main function for using the sensor. this function will execute commands on the sensor board's ADC based on the config writeable.
void KickSat_Sensor::operate(String board, float* dataBuffer) {
  if (board== "xtb1"){
    _ADCchipSelect = SPI_CS_XTB1;
  } else if (board== "xtb2"){
    _ADCchipSelect = SPI_CS_XTB2;
  } else if (board== "xtb3"){
    _ADCchipSelect = SPI_CS_XTB3;
  }
   
  wakeADC();
  delay(500);
  resetADC();
  delay(100);
  startADC();
  delay(300);
  if (board== "xtb1"){ //H.ALPERT devices
    float dataOut[sensor1_BUF_LEN];
    float dataTemp[12];
    //DEVICE B
    dataOut[0] =      readTemp();
    GPIO(0x00,0x02);
    dataTemp[0] =    hallGen(8, 4, 0x04, 5, 9, 50);
    GPIO(0x00,0x00);
    dataTemp[1] =    hallGen(9, 5, 0x04, 8, 4, 50);
    dataTemp[2] = -1*hallGen(8, 4, 0x04, 9, 5, 50);
    GPIO(0x00,0x01);
    dataTemp[3] = -1*hallGen(9, 5, 0x04, 4, 8, 50); 
    GPIO(0x00,0x00);  
    dataOut[1] = ((dataTemp[0]+dataTemp[1]+dataTemp[2]+dataTemp[3])/4);
    dataOut[2] = (voltageApplied/4);
    voltageApplied = 0;
    //DEVICE A
    dataTemp[4] =    hallGen(0, 2, 0x04, 3, 1, 50);
    dataTemp[5] =    hallGen(1, 3, 0x04, 0, 2, 50);
    dataTemp[6] = -1*hallGen(0, 2, 0x04, 1, 3, 50);
    dataTemp[7] = -1*hallGen(1, 3, 0x04, 2, 0, 50);   
    dataOut[3] = ((dataTemp[4]+dataTemp[5]+dataTemp[6]+dataTemp[7])/4);
    dataOut[4] = (voltageApplied/4);
    voltageApplied = 0;
    //DEVICE C
    dataTemp[8] =    hallGen(7, 12, 0x04, 6, 12, 50);
    dataTemp[9] =    hallGen(12, 6, 0x04, 7, 12, 50);
    dataOut[5] = ((dataTemp[8]+dataTemp[9])/2);
    dataOut[6] = (voltageApplied/4);
    voltageApplied = 0;
    //DEVICE D
    dataTemp[10] =    hallGen(10, 12, 0x04, 11, 12, 50);
    dataTemp[11] =    hallGen(11, 12, 0x04, 10, 12, 50);
    dataOut[7] = ((dataTemp[10]+dataTemp[11])/2);
    dataOut[8] = (voltageApplied/4);
    voltageApplied = 0;
    memcpy(dataBuffer, dataOut, sizeof(dataOut)); 
    #ifdef DEBUG
      for (uint8_t i=0; i<sensor1_BUF_LEN; i++){      
        Serial.println(dataOut[i],8);
      }
      Serial.println("Phases...");
      for (uint8_t i=0; i<12; i++){      
        Serial.println(dataTemp[i],8);
      }
    #endif    
  }
  else if (board=="xtb2"){ //T.HEUSER devices
    float dataOut[sensor2_BUF_LEN];
    dataOut[0] = readPins(0x6C, 0xF6, 0x80, 200, 100, 0x03);
    dataOut[1] = readPins(0x3C, 0xF3, 0x80, 200, 100, 0x03);
    dataOut[2] = readPins(0x2C, 0xF6, 0x80, 200, 100, 0x03);
    GPIO(0x00, 0x04);
    dataOut[3] = readPins(0x1A, 0xF1, 0x80, 200, 100, 0x03);
    GPIO(0x00, 0x00);
    delay(50);
    GPIO(0x00, 0x02);
    dataOut[4] = readPins(0x49, 0xF4, 0x80, 200, 100, 0x01);
    GPIO(0x00, 0x00);
    dataOut[5] = readPins(0x5C, 0xF5, 0x80, 200, 100, 0x01);
    GPIO(0x00, 0x01);
    dataOut[6] = readPins(0x78, 0xF7, 0x80, 200, 100, 0x01);
    GPIO(0x00, 0x00);
    memcpy(dataBuffer, dataOut, sizeof(dataOut)); 
    // datafile.write((const uint8_t *)&dataOut, sizeof(dataOut)); //save data to SD card as bytes (4 bytes per float);
    #ifdef DEBUG
      for (uint8_t i=0; i<sensor2_BUF_LEN; i++){      
        Serial.println(dataOut[i],8);
      }
    #endif    
  }
  else if (board=="xtb3"){ //M.HOLLIDAY devices
    float dataOut[sensor3_BUF_LEN];
    Serial.println(board);
    dataOut[0] = readTemp();     
    dataOut[1] = readPins(0x7C, 0xF7, 0x80, 200, 100, 0x03);
    dataOut[2] = readPins(0x4C, 0xF4, 0x80, 200, 100, 0x03);
    dataOut[3] = readPins(0x3C, 0xF3, 0x80, 200, 100, 0x03);
    dataOut[4] = readPins(0x2C, 0xF2, 0x80, 200, 100, 0x03);
    readPins(0xCC, 0xFF, 0x80, 20, 1, 0x01);
    GPIO(0x00, 0x01);
    dataOut[5] = readPins(0x58, 0xF5, 0x80, 200, 100, 0x01);
    GPIO(0x00, 0x00);
    delay(50);
    GPIO(0x00, 0x02);
    dataOut[6] = readPins(0x59, 0xF5, 0x80, 200, 100, 0x01);
    GPIO(0x00, 0x00);
    delay(50);
    GPIO(0x00, 0x04);
    dataOut[7] = readPins(0x0A, 0xF0, 0x80, 200, 100, 0x01);
    GPIO(0x00, 0x00);
    delay(50);
    GPIO(0x00, 0x08);
    dataOut[8] = readPins(0x0B, 0xF0, 0x80, 200, 100, 0x01);
    GPIO(0x00, 0x00);
    memcpy(dataBuffer, dataOut, sizeof(dataOut));  
    // datafile.write((const uint8_t *)&dataOut, sizeof(dataOut)); //save data to SD card as bytes (4 bytes per float);    
    #ifdef DEBUG
     for (uint8_t i=0; i<sensor3_BUF_LEN; i++){      
       Serial.println(dataOut[i],8);
     }
    #endif    
  }  
  shutdownADC(); 
  
  // datafile.close();  
  delay(2000);
}

//==================== Register Commands ====================//

//this function wirtes [len] registers to the values specified in [data]
//use to initialize/reset the ADC
void KickSat_Sensor::burstWriteRegs(byte start, uint8_t len, byte* data) {
  Serial.println("------Writing ADC Config------");
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(_ADCchipSelect, LOW);
  SPI.transfer(start);   //Send register START location
  SPI.transfer(len - 1);   //how many registers to write to (must be len-1 as the ADC considers 0x00 to be 1, 0x01 is 2, ect)
  for (uint8_t i = 0; i < len; i++) {
    SPI.transfer(data[i]);
  }
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH);
}

//brings the ADC from STANDBY mode into CONVERSION mode
void KickSat_Sensor::startADC() {
  Serial.println("------Starting ADC------");
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x0A); //send stop byte
  SPI.transfer(0x08); //send start byte
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH);
}

//brings the ADC from CONVERSION mode to STANDBY mode
void KickSat_Sensor::stopADC() {
  Serial.println("------Stopping ADC------");
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x0A); //send stop byte
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH);
}

//resets the ADC
void KickSat_Sensor::resetADC() {
  byte commands[8]{0xCC, 0x08, 0x1C, 0x39, 0x00, 0xFF, 0x00, 0x10};
  Serial.println("------Resetting ADC------");
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x06); //send reset byte
  digitalWrite(XTB_RESET, LOW);
  delayMicroseconds(1);
  digitalWrite(XTB_RESET, HIGH);
  burstWriteRegs(0x42, 8, commands);
}


//brings the ADC from CONVERSION or STANDBY mode into SHUTDOWN mode
void KickSat_Sensor::shutdownADC() {
  Serial.println("------Shutting Down ADC------");
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x04); //send shutdown byte
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH);
}

//brings the ADC into STANDBY mode from SHUTDOWN mode
void KickSat_Sensor::wakeADC()  {
  Serial.println("------Waking ADC------");
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x02); //send wakeup byte
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH);
}

void KickSat_Sensor::GPIO(byte pins, byte state){
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1); 
  SPI.transfer(0x50);  
  SPI.transfer(0x01); 
  SPI.transfer(pins);
  SPI.transfer(state); 
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH); 
}

void KickSat_Sensor::writeReg(byte start, byte value){
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1); 
  SPI.transfer(start);  
  SPI.transfer(0x00); 
  SPI.transfer(value);
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH); 
}

void KickSat_Sensor::regReadout(){
  Serial.println("------Register Readout------");
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x20);   //Send register START location
  SPI.transfer(0x12);   //how many registers we want to read (0x12 = all 18)
  for (uint8_t i = 0; i <= 17; i++) {
    readOut[i] = SPI.transfer(0x00);
  }
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH);
  Serial.print("Register 0x00 (ID):        "), Serial.println(readOut[0], HEX);
  Serial.print("Register 0x01 (STATUS):    "), Serial.println(readOut[1], HEX);
  Serial.print("Register 0x02 (INPMUX):    "), Serial.println(readOut[2], HEX);
  Serial.print("Register 0x03 (PGA):       "), Serial.println(readOut[3], HEX);
  Serial.print("Register 0x04 (DATARATE):  "), Serial.println(readOut[4], HEX);
  Serial.print("Register 0x05 (REF):       "), Serial.println(readOut[5], HEX);
  Serial.print("Register 0x06 (IDACMAG):   "), Serial.println(readOut[6], HEX);
  Serial.print("Register 0x07 (IDACMUX):   "), Serial.println(readOut[7], HEX);
  Serial.print("Register 0x08 (VBIAS):     "), Serial.println(readOut[8], HEX);
  Serial.print("Register 0x09 (SYS):       "), Serial.println(readOut[9], HEX);
  Serial.print("Register 0x0A (OFCAL0):    "), Serial.println(readOut[10], HEX);
  Serial.print("Register 0x0B (OFCAL1):    "), Serial.println(readOut[11], HEX);
  Serial.print("Register 0x0C (OFCAL2):    "), Serial.println(readOut[12], HEX);
  Serial.print("Register 0x0D (FSCAL0):    "), Serial.println(readOut[13], HEX);
  Serial.print("Register 0x0E (FSCAL1):    "), Serial.println(readOut[14], HEX);
  Serial.print("Register 0x0F (FSCAL2):    "), Serial.println(readOut[15], HEX);
  Serial.print("Register 0x10 (GPIODAT):   "), Serial.println(readOut[16], HEX);
  Serial.print("Register 0x11 (GPIOCON):   "), Serial.println(readOut[17], HEX);
  Serial.println("-----------------------------");
}

float KickSat_Sensor::dataConvert( byte a, byte b, byte c){
  int rawDataIN = 0; //create an empty 24 bit integer for the data
  float dataOUT = 0;
  rawDataIN |= a; //shift the data in one byte at a time
  rawDataIN = (rawDataIN << 8) | b;
  rawDataIN = (rawDataIN << 8) | c;
  if (((1 << 23) & rawDataIN) != 0){ //If the rawData has bit 24 on, then it's a negative value
    int maskIN = (1 << 24) - 1;
    rawDataIN = ((~rawDataIN) & maskIN) + 1;
    dataOUT = rawDataIN * LSBsize * -1;}
  else{ //if it's not negative
    dataOUT = float(rawDataIN)*LSBsize;} //then just multiply by LSBsize
  return dataOUT;
}

/*
  Read pin command
  in config file w/o quotes:
  "read [+ pin] [- pin] [IDAC pin] [VBias Pin] [Log Data t/f] [wait time (in ms)] [measurements to read] [IDAC mag] [data label text],"
  for example:
  "read 2 12 3 4 t 5,"
          measures a voltage between AIN2 and AINCOM (C hex = 12 DEC),
          drives a 100uA current on AIN3,
          applies a 1.6V bias on AIN2,
          logs the data to the SD card,
          and waits for 5 ms between measurements
*/
float KickSat_Sensor::readPins(byte pinNums, byte idacPin, byte vbPin, int wait, int bufflen, byte idacMag){
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  byte pinDat1, pinDat2, pinDat3 = 0;
  float pinData = 0; 
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1); 
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x06);   //how many registers to write to
  SPI.transfer(pinNums);//0x42  INPMUX 
  SPI.transfer(0x08);   //0x43  PGA
  SPI.transfer(0x1C);   //0x44  DATARATE
  SPI.transfer(0x39);   //0x45  REF
  SPI.transfer(idacMag);   //0x46  IDACMAG
  SPI.transfer(idacPin);   //0x47  IDACMUX
  SPI.transfer(vbPin);     //0x48  VBIAS -- BIASING AIN2
  delay(wait);
  for (uint8_t i = 0; i <= bufflen; i++) {    
    SPI.transfer(0x00);   //NOP to get rid of junk?
    SPI.transfer(0x00);   //NOP to get rid of junk?
    SPI.transfer(0x12);   //transfer read command  
    pinDat1 = SPI.transfer(0x00);
    pinDat2 = SPI.transfer(0x00);
    pinDat3 = SPI.transfer(0x00);    
    pinData += dataConvert(pinDat1,pinDat2,pinDat3);
    delay(2);
  }  
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH);
  pinData = pinData/bufflen;
  return pinData;
}

float KickSat_Sensor::readTemp() {
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  byte aa, bb, cc = 0;
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1);   
  SPI.transfer(0x49);   //Send register START location
  SPI.transfer(0x00);   //how many registers to write to
  SPI.transfer(0x50);   //0x49  SYS  
  delay(5);
  SPI.transfer(0x00);   //send NOPS
  SPI.transfer(0x00);
  SPI.transfer(0x12); //transfer read command  
  aa = SPI.transfer(0x00);
  bb = SPI.transfer(0x00);
  cc = SPI.transfer(0x00);
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH);  
  float temp = dataConvert(aa, bb, cc)*1000.0;
  float dd = 0;
  if (temp < 129){
    dd = ((-1*(129.00-temp)*0.403)+25);
  }
  else {
    dd = ((-1*(129.00-temp)*0.403)+25);
  }
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1);   
  SPI.transfer(0x49);   //Send register START location
  SPI.transfer(0x00);   //how many registers to write to
  SPI.transfer(0x10);   //0x49  SYS  
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH);
  return dd;
}

float KickSat_Sensor::hallGen(uint8_t inp, uint8_t inn, byte idacMag, uint8_t idacMux, uint8_t vb, int delayT) {
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  byte inByteA, inByteB, inByteC, vbPin, inByteD, inByteE, inByteF = 0;
  byte inpMux = ((inp << 4) | inn);
  byte vapp = (idacMux <<4) | vb;
  if (vb <= 1){
    vbPin = (0x80 | (vb+1));
  } else if (vb == 2){
    vbPin = 0x84;
  } else if (vb == 3){
    vbPin = 0x88;
  } else if (vb == 4){
    vbPin = 0x90;
  } else if (vb == 5){
    vbPin = 0xA0;
  } else if (vb > 5){
    vbPin = 0x80;
  }  
  digitalWrite(_ADCchipSelect, LOW);
  delayMicroseconds(1);
  SPI.transfer(0x42);     //Send register START location
  SPI.transfer(0x06);     //how many registers to write to
  SPI.transfer(inpMux);   //0x42  INPMUX 
  SPI.transfer(0x08);     //0x43  PGA
  SPI.transfer(0x0C);     //0x44  DATARATE
  SPI.transfer(0x39);     //0x45  REF
  SPI.transfer(idacMag);           //0x46  IDACMAG
  SPI.transfer((0xF0 | idacMux));  //0x47  IDACMUX
  SPI.transfer(vbPin);             //0x48  VBIAS
  delay(delayT);
  SPI.transfer(0x00);
  SPI.transfer(0x12);     //transfer read command  
  inByteA = SPI.transfer(0x00);
  inByteB = SPI.transfer(0x00);
  inByteC = SPI.transfer(0x00);
  regReadout();
  SPI.transfer(0x42);   //Send register START location
  SPI.transfer(0x00);   //0x42  INPMUX 
  SPI.transfer(vapp);   //how many registers to write to
  delay(10);
  SPI.transfer(0x00);
  SPI.transfer(0x12);     //transfer read command  
  inByteD = SPI.transfer(0x00);
  inByteE = SPI.transfer(0x00);
  inByteF = SPI.transfer(0x00);
  delay(1);
  digitalWrite(_ADCchipSelect, HIGH);
  float reading = dataConvert(inByteA,inByteB,inByteC);
  float voltageApp = dataConvert(inByteD,inByteE,inByteF);
  voltageApplied += voltageApp; 
  Serial.println(reading,8); 
  return reading;
}


float KickSat_Sensor::getFloat(byte packet[], uint8_t i){
    float out;
    memcpy(&out, &packet[i], sizeof(float));
    return out;
}