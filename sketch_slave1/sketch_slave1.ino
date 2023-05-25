#include <string.h>
#include "HYT-271.h"

const byte SLAVE_ADDRESS = 0x3B;
const byte MESSAGE_LENGTH = 11;
byte message[MESSAGE_LENGTH];

HYT271 hyt = HYT271();


const uint16_t CRC_POLY = 0x1021;
const uint16_t INIT_CRC = 0xFFFF;
for each byte (0 to 255) we predefined a CRC table to speed up computations
uint16_t CRCTable[256];

void setWriteModeRS485() {
  PORTD |= 1 << PD2;
  delay(1);
}

// переривання при завершенні передачі
ISR(USART_TX_vect) { 
  PORTD &= ~(1 << PD2); // встановлюю режим прийому
}

int writeDataToMaster() {
  message[0] = MESSAGE_LENGTH;
  hyt.read();
  float_to_byte_array(hyt.getTemperature(), message, 1);
  float_to_byte_array(hyt.getHumidity(), message, 5);

  unsigned short checkSumCRC = getCRC(message, MESSAGE_LENGTH - 2);
  byte firstByteOfCheckSum = (checkSumCRC >> 8) & 0xFF;
  byte secondByteOfCheckSum = checkSumCRC & 0xFF;
  message[MESSAGE_LENGTH - 2] = firstByteOfCheckSum;
  message[MESSAGE_LENGTH - 1] = secondByteOfCheckSum;
  
    for (int i = 0; i < MESSAGE_LENGTH; i++) {
        byte byteToSend = message[i];
        Serial.write(byteToSend);
    }
  
}


uint16_t getCRCForByte(unsigned char val)
{
  uint16_t crc = (uint16_t)val << 8;
  for (unsigned char j = 0; j < 8; j++)
  {
    if (crc & 0x8000) {
        crc = (crc << 1) ^ CRC_POLY;
    } else {
        crc <<= 1;
    }
  }
 
  return crc;
}
 
void buildCRCTable() {
  // fill an array with CRC values of all 256 possible bytes
  for (int i = 0; i < 256; i++)
  {
    CRCTable[i] = getCRCForByte(i);
  }
}

void setup() {
  delay(500);
  // En_slave - на вихід + низький рівень
  DDRD = 0b00000111;
  PORTD = 0b11111000;

  // initialize UART0
  Serial.begin(7200, SERIAL_8N1);
  UCSR0B |= (1 << UCSZ02) | (1 << TXCIE0);
  UCSR0A |= (1 << MPCM0); // мультипроцесорний режим
  
  hyt.begin();

  delay(1);
}

void loop() {
  if (Serial.available()) {
    byte inByte = Serial.read();
    if (SLAVE_ADDRESS == inByte) {
      UCSR0A &= ~(1 << MPCM0);
      setWriteModeRS485();
      writeDataToMaster();
      delay(200);
    } 
  }
}

uint16_t getCRC(unsigned char data[], unsigned char length) {
  uint16_t crc = INIT_CRC;
 
  for (unsigned char i = 0; i < length; i++)
    crc = (crc << 8) ^ CRCTable[((crc >> 8) ^ message[i])];
  return crc ^ 0x0000;
}

void float_to_byte_array(float num, byte *byte_array, byte array_start_position) {
    memcpy(byte_array + array_start_position, &num, sizeof(float));
}
