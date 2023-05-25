#define Y 0x01
#define U 0x02
#define L 0x03
#define I 0x04
#define A 0x05
#define S 0x06

#define H 0x07
#define V 0x08
#define E 0x09
#define T 0x0A
#define R 0x0B
#define I 0x0C
#define N 0x0D
#define _ 0x00

const unsigned char SLAVE_ADDRESS = 0x3B;
const unsigned char MESSAGE_LENGTH = 24;

const uint16_t CRC_POLY = 0x1021;
const uint16_t INIT_CRC = 0xFFFF;
// for each byte (0 to 255) we predefined a CRC table to speed up computations
uint16_t CRCTable[256];

unsigned char message[MESSAGE_LENGTH] = {
    Y, U, L, I, A, _, 
    S, H, V, E, T, S, _,
    T, A, R, A, S, I, V, N, A,
    0x00, 0x00
};

void setWriteModeRS485() {
  PORTD |= 1 << PD2;
  delay(1);
}

// переривання при завершенні передачі
ISR(USART_TX_vect) { 
  PORTD &= ~(1 << PD2); // встановлюю режим прийому
}

int writeDataToMaster() {
  uint16_t calculatedCrcValue = getCRC(message, MESSAGE_LENGTH - 2);
  // calculated crc value contains from 16 bits, so it has to be separated into 2 bytes
  unsigned char firstByteOfCrcValue = (calculatedCrcValue >> 8) & 0xFF;
  unsigned char secondByteOfCrcValue = calculatedCrcValue & 0xFF;
  message[MESSAGE_LENGTH - 2] = firstByteOfCrcValue;
  message[MESSAGE_LENGTH - 1] = secondByteOfCrcValue;


  for(int messageNumber = 0; messageNumber < 5; messageNumber++){
    for (int byteOfDataNumber = 0; byteOfDataNumber < MESSAGE_LENGTH; byteOfDataNumber++) {
        unsigned char byteToSend = message[byteOfDataNumber];
        if (messageNumber == 1 && byteOfDataNumber == 0) {
          byteToSend ^= (1 << 6);
        } else if (messageNumber == 4 && byteOfDataNumber == 6) {
          byteToSend ^= ((1 << 3) | (1 << 4) | (1 << 5));
        }
        Serial.write(byteToSend);
    }
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
  delay(100);
  // En_slave - на вихід + низький рівень
  DDRD = 0b00000111;
  PORTD = 0b11111000;

  buildCRCTable();

  // initialize UART0
  Serial.begin(9600, SERIAL_8N1);
  UCSR0B |= (1 << UCSZ02) | (1 << TXCIE0);
  UCSR0A |= (1 << MPCM0); // мультипроцесорний режим
}

void loop() {
  if (Serial.available()) {
    unsigned char inByte = Serial.read();
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