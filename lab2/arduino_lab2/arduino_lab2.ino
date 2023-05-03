const int buttonPin1 =  64;
const int buttonPin2 =  65;
int inByte;

void setup() {
  DDRC = 0xFF;
  PORTC = 0;
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  Serial.begin(9600);  //UART0
}

void loop() {
  if (Serial.available()) {
    inByte = Serial.read();
    if (inByte == 0xA1) {
      PORTC = 1;
      while (PORTC) {
        delay(1200);
        PORTC = PORTC << 1;
      }
    } else if (inByte == 0xA2) {
      PORTC = B00011000;
      delay(1200);
      PORTC = B00100100;
      delay(1200);
      PORTC = B01000010;
      delay(1200);
      PORTC = B10000001;
      delay(1200);
      PORTC = B00000000;
    }
  }

  if (digitalRead(buttonPin1) == LOW) {
    Serial.write(0xB1);
    delay(500);
  }
  if (digitalRead(buttonPin2) == LOW) {
    Serial.write(0xB2);
    delay(500);
  }
}

