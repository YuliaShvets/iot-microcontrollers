void setWriteModeRS485() {
  unsigned char port = PORTD;
  PORTD |= 1 << PD1; 
  delay(1);
}

// переривання при завершенні передачі
ISR(USART1_TX_vect)
{ 
  PORTD &= ~(1 << PD1); // режим прийому
}

void setup() {
  delay(100);
  // En_m - на вихід + низький рівень
  DDRD |= 1 << PD1;
  PORTD &= ~(1 << PD1);

  // RS-232 microcontroller <=> Windows Form
  Serial.begin(9600);

  // RS-485
  Serial1.begin(14400, SERIAL_8N1);
  UCSR1B |= (1 << UCSZ12) | (1 << TXCIE1);
}

void loop() {
  if (Serial.available()) { // if data from form has come
    // inByte contains address of Slave1 or Slave2. After specific 
    // slave receives the address, it begins to send the message
    unsigned char inByte = Serial.read();
    setWriteModeRS485();
    UCSR1B |= 1 << TXB81;
    Serial1.write(inByte);
  }
  if (Serial1.available()) { // if we receive something from slaves, we also process it.
    unsigned char inByte1 = Serial1.read();
    Serial.write(inByte1);
  }
}
