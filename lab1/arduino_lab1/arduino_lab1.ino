const int buttonPin = 64;

void setup() {
  DDRC = 0xFF;
  PORTC = 0;

pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  if(digitalRead(buttonPin) == LOW){
    PORTC = 1;
    while(PORTC){
      delay(1200);
      PORTC = PORTC << 1;
    }
  }
}

