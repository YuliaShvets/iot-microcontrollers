#define ZERO 0x11
#define SIX 0x12
#define FOUR  0x13
#define TWO  0x14
#define DOT  0x15
#define _ 0x00

const unsigned char SLAVE_ADDRESS = 0x62;
const unsigned char MESSAGE_LENGTH = 12;

const uint16_t CRC_POLY = 0x1021;
const uint16_t INIT_CRC = 0xFFFF;
// for each byte (0 to 255) we predefined a CRC table to speed up computations
uint16_t CRCTable[256];

unsigned char message[MESSAGE_LENGTH] = { ZERO, SIX, DOT, ZERO, FOUR, DOT, TWO, ZERO, ZERO, FOUR, _, _ };

void setWriteModeRS485() {
  PORTD |= 1 << PD2;
  delay(1);
}

// переривання по завершенню передачі
ISR(USART_TX_vect)
{
  PORTD &= ~(1 << PD2); 
}

void writeDataToMaster() {
  uint16_t calculatedCrcValue = getCRC(message, MESSAGE_LENGTH - 2);
  unsigned char firstByteOfCrcValue = (calculatedCrcValue >> 8) & 0xFF;
  unsigned char secondByteOfCrcValue = calculatedCrcValue & 0xFF;
  message[MESSAGE_LENGTH - 2] = firstByteOfCrcValue;
  message[MESSAGE_LENGTH - 1] = secondByteOfCrcValue;


  for(int messageNumber = 0; messageNumber < 5; messageNumber++) {
    for (int byteOfDataNumber = 0; byteOfDataNumber < MESSAGE_LENGTH; byteOfDataNumber++) {
      unsigned char byteToSend = message[byteOfDataNumber];
      if (messageNumber == 2 && byteOfDataNumber == 9) {
        byteToSend ^= (1 << 5);
      } else if (messageNumber == 3 && byteOfDataNumber == 1) {
        byteToSend ^= ((1 << 5) | (1 << 6)) ;
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
  DDRD = 0b00000111;
  PORTD = 0b11111000;

  buildCRCTable();

  Serial.begin(9600, SERIAL_8N1);
  // задає біт UCSZ02, який описаний тут: "Біти UCSZn2, поєднані з бітом UCSZn1: 0 в UCSRnC, встановлюють
  // кількість бітів даних (Символ SiZe) у кадрі. використання приймача та передавача ". Це в основному
  // дозволяє вам вибрати 9-бітовий серійний номер (8 біт є більш поширеним)
  
  UCSR0B |= (1 << UCSZ02) | (1 << TXCIE0);
  UCSR0A |= (1 << MPCM0);
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

// just an example of an algorithm implementation. Not used for crc calculation
uint16_t calculate_crc16_ccitt_false(const uint8_t *data, size_t data_length) {
    uint16_t crc = 0xFFFF;
    uint16_t poly = 0x1021;

    for (size_t i = 0; i < data_length; i++) {
        crc ^= ((uint16_t)data[i] << 8);

        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ poly;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc ^ 0x0000;
}
