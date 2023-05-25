bool isAddress = true;
bool isCommand = false;
byte command;

void setWriteModeRS485() {
  byte port = PORTD;
  PORTD |= 1 << PD1; 
  delay(1);
}

// переривання при завершенні передачі
ISR(USART1_TX_vect)
{ 
  PORTD &= ~(1 << PD1); // режим прийому
}

void setup() {
  delay(500);
  // usually pin 13 has built in LED
  pinMode(LED_BUILTIN, OUTPUT);
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);   
  
  // En_m - на вихід + низький рівень
  DDRD |= 1 << PD1;
  PORTD &= ~(1 << PD1);

  // initialize UART0 (RS-232: microcontroller to Windows Form communication)
  // baud rate of 9600 bps with a data frame of 8 bits, no parity, and 1 stop bit
  Serial.begin(9600);

  // initialize RS-485
  Serial1.begin(7200, SERIAL_8N1);
  UCSR1B |= (1 << UCSZ12) | (1 << TXCIE1);
  /*
   The first bit is UCSZ12, which is responsible for setting the character size of the data frame to 8 bits. 
   The second bit is TXCIE1, which is the Transmission Complete Interrupt Enable bit. 
   This bit allows the microcontroller to generate an interrupt when the transmission of a byte is complete.
  */
}

void loop() {
  if (Serial.available()) { // if data from form has come
    byte inByte = Serial.read();
    if (isAddress) {
      setWriteModeRS485();
      UCSR1B |= 1 << TXB81;
      Serial1.write(inByte);
      isAddress = false;
      isCommand = true;
    } else if (isCommand) {
      command = inByte;
      isCommand = false;
      setWriteModeRS485();
      // після запису даних у буфер передачі необхідно
      // надати дозвіл на переривання для події спорожнення буфера UDR.
      UCSR1B &= ~(1 << TXB81); 
      Serial1.write(inByte);
      if (command == 0xB1) isAddress = true; // Команда читання 1 байту
    } else { // data byte
      isAddress = true;
      setWriteModeRS485();
      UCSR1B &= ~(1 << TXB81);
      Serial1.write(inByte);
    }
  }

  if (Serial1.available()) { // if we receive something from slaves, we also process it.
    byte inByte1 = Serial1.read();
    Serial.write(inByte1);
  }
}
