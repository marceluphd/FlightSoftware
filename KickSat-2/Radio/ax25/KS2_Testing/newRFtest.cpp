#include <SPI.h>
#include <RH_RF22.h>
//#include <RHReliableDatagram.h>

#define KICKSAT_RADIO_SDN 5
#define RADIO_INTERRUPT 2
#define RADIO_SS 10

//#define CLIENT_ADDRESS 1
//#define SERVER_ADDRESS 2

RH_RF22 radio(RADIO_SS, RADIO_INTERRUPT);
//RHReliableDatagram radio(driver, CLIENT_ADDRESS);

uint8_t packet[] = {
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111, 0b01111111,
  0b01110101, 0b10010110, 0b01010100, 0b10101011, 0b01010100, 0b10101011, 0b01010111, 0b01110010,
  0b01011010, 0b01001000, 0b10110101, 0b10101101, 0b10001010, 0b00101000, 0b11101010, 0b10101111,
  0b10110011, 0b00110111, 0b00010000, 0b10110000, 0b11011000, 0b10000111, 0b00001000, 0b10101001,
  0b00101110, 0b10010001, 0b00010001, 0b00000011, 0b01111101, 0b11000000, 0b01111111
};

void setup() {
  SerialUSB.begin(9600);
  //SerialUSB.println("Serial Started");
  //RH_RF22::initSPI():

  pinMode(KICKSAT_RADIO_SDN, OUTPUT);
  digitalWrite(KICKSAT_RADIO_SDN, HIGH);
  delay(2000);
  digitalWrite(KICKSAT_RADIO_SDN, LOW);
  delay(500);
  
  if (!radio.init()) {
    SerialUSB.println("Init failed.");
  } else {
    SerialUSB.println("Init Success.");
  }
  
  
}

int n = 0;
void loop() {
  SerialUSB.print("Status byte: 0x");
  SerialUSB.print(radio.statusRead(), HEX);
  SerialUSB.print("\t Packet ");
  uint8_t data = 4;
  radio.send(packet, 1);
  radio.waitPacketSent();
  SerialUSB.println(n++);
  delay(100);
}