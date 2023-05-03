const unsigned char BuzzerPin = 21;
const unsigned char SetHoursButtonPin = 51;
const unsigned char SetMinutesButtonPin = 50;
const unsigned char ChangeTimerButtonPin = 10;
const unsigned char ControlTimerButtonPin = 11;
const unsigned char SevenSegmentsControlPins[8] = {37, 36, 35, 34, 33, 32, 31, 30};
const unsigned char SevenSegmentsPowerPins[6] = {22, 23, 24, 25, 26, 27};

const unsigned char BUZZING_TIME_IN_SEC = 30;

const bool SevenSegmentsNumsArray[10][7] = {
  { 1, 1, 1, 1, 1, 1, 0 }, // 0
  { 0, 1, 1, 0, 0, 0, 0 }, // 1
  { 1, 1, 0, 1, 1, 0, 1 }, // 2
  { 1, 1, 1, 1, 0, 0, 1 }, // 3
  { 0, 1, 1, 0, 0, 1, 1 }, // 4
  { 1, 0, 1, 1, 0, 1, 1 }, // 5
  { 1, 0, 1, 1, 1, 1, 1 }, // 6
  { 1, 1, 1, 0, 0, 0, 0 }, // 7
  { 1, 1, 1, 1, 1, 1, 1 }, // 8
  { 1, 1, 1, 1, 0, 1, 1 }  // 9
};
struct Time {
  unsigned char seconds, minutes, hours;
};

struct Timer {
  Time mainTime;
  unsigned char buzzingTimeInSec;
  bool isActive;
};

Timer timer1 = {{0, 0, 0}, 0, false};
Timer timer2 = {{0, 0, 0}, 0, false};
Timer* currentTimer = &timer1;
unsigned char current7SegmentsPowerPinIndex = 0;

void buzz() {
  digitalWrite(BuzzerPin, HIGH);
  delay(15);
  digitalWrite(BuzzerPin, LOW);
}

void manageTimer(Timer & timer) {
  if (timer.isActive) {
    if (timer.mainTime.seconds == 0) {
      if (timer.mainTime.minutes == 0) {

        timer.mainTime.seconds = 59;
        timer.mainTime.minutes = 59;
        timer.mainTime.hours--;

      } else {
        timer.mainTime.seconds = 59;
        timer.mainTime.minutes--;
      }
    } else {
      timer.mainTime.seconds--;
    }

    if (timer.mainTime.seconds == 0 && timer.mainTime.minutes == 0 && timer.mainTime.hours == 0) {
      timer.isActive = false;
      timer.buzzingTimeInSec = BUZZING_TIME_IN_SEC;
      digitalWrite(BuzzerPin, HIGH);
    }
  } else if (timer.buzzingTimeInSec != 0) {
    timer.buzzingTimeInSec--;
    if ((BUZZING_TIME_IN_SEC - timer.buzzingTimeInSec) % 2 == 0)
      digitalWrite(BuzzerPin, LOW);
    else
      digitalWrite(BuzzerPin, HIGH);
  }
}

void printNumber(unsigned char number) {
  if (number < 10) {
    for (unsigned char i = 0; i < 7; i++)
      digitalWrite(SevenSegmentsControlPins[i], SevenSegmentsNumsArray[number][i]);
  }
}

void printTime() {
  digitalWrite(SevenSegmentsPowerPins[current7SegmentsPowerPinIndex], LOW);
  if (current7SegmentsPowerPinIndex == 5)
    current7SegmentsPowerPinIndex = 0;
  else
    current7SegmentsPowerPinIndex++;
  digitalWrite(SevenSegmentsPowerPins[current7SegmentsPowerPinIndex], HIGH);

  for (unsigned char i = 0; i < 7; i++)
    digitalWrite(SevenSegmentsControlPins[i], LOW);
  digitalWrite(SevenSegmentsControlPins[7], LOW);
  switch (current7SegmentsPowerPinIndex) {
    case 0:
      printNumber((unsigned char)(currentTimer->mainTime.hours / 10));
      break;
    case 1:
      printNumber(currentTimer->mainTime.hours % 10);
      digitalWrite(SevenSegmentsControlPins[7], HIGH);
      break;
    case 2:
      printNumber((unsigned char)(currentTimer->mainTime.minutes / 10));
      break;
    case 3:
      printNumber(currentTimer->mainTime.minutes % 10);
      digitalWrite(SevenSegmentsControlPins[7], HIGH);
      break;
    case 4:
      printNumber((unsigned char)(currentTimer->mainTime.seconds / 10));
      break;
    case 5:
      printNumber(currentTimer->mainTime.seconds % 10);
      break;
  }
}

ISR(TIMER1_COMPA_vect) {
  manageTimer(timer1);
  manageTimer(timer2);
}

ISR(TIMER0_COMPA_vect) {
  printTime();
}

void setup() {
  noInterrupts();

  TCCR1A = 0x00;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); //CTC mode & Prescaler @ 1024
  TIMSK1 = (1 << OCIE1A);
  OCR1A = 0x2DC6; // compare value = 1 sec (12MHz AVR)

  TCCR2A = 0x00;
  TCCR2B = (1 << WGM22) | (1 << CS22) | (1 << CS20); //CTC mode & Prescaler @ 1024
  TIMSK0 = (1 << OCIE2A);
  OCR0A = 0x0014; // compare value for 500 Hz (12MHz AVR)

  pinMode(BuzzerPin, OUTPUT);
  digitalWrite(BuzzerPin, LOW);
  pinMode(SetHoursButtonPin, INPUT);
  digitalWrite(SetHoursButtonPin, HIGH);
  pinMode(SetMinutesButtonPin, INPUT);
  digitalWrite(SetMinutesButtonPin, HIGH);
  pinMode(ChangeTimerButtonPin, INPUT);
  digitalWrite(ChangeTimerButtonPin, HIGH);
  pinMode(ControlTimerButtonPin, INPUT);
  digitalWrite(ControlTimerButtonPin, HIGH);

  DDRA = 0xFF;  		// порт працює на вихід 
	PORTA = 1;    

   // Порт C -- сегменти (катод)
	DDRC = 0xFF;  		// порт працює на вихід 
	PORTC = 0xFF; 	  // +5V (семисегм. не світять)  

  interrupts(); // Enable global interrupts
}

void loop() {
  if (digitalRead(SetHoursButtonPin) == LOW) {
    if (currentTimer->mainTime.hours == 99) 
      currentTimer->mainTime.hours = 0;
    else
      currentTimer->mainTime.hours++;
    delay(21000);
  }
  
  if (digitalRead(SetMinutesButtonPin) == LOW) {
    if (currentTimer->mainTime.minutes == 59) 
      currentTimer->mainTime.minutes = 0;
    else
      currentTimer->mainTime.minutes++;
    delay(21000);
  }
  
  if (digitalRead(ChangeTimerButtonPin) == LOW) {
    currentTimer->isActive = false;
    if (currentTimer == &timer1)
      currentTimer = &timer2;
    else
      currentTimer = &timer1;
    delay(21000);
  }

  if (digitalRead(ControlTimerButtonPin) == LOW) {
    if (currentTimer->isActive) {
        currentTimer->isActive = false;
        currentTimer->mainTime.hours = 0;
        currentTimer->mainTime.minutes = 0;
        currentTimer->mainTime.seconds = 0;
      }
      else if (!(currentTimer->mainTime.hours == 0 && currentTimer->mainTime.minutes == 0 && currentTimer->mainTime.seconds == 0))
        currentTimer->isActive = true;
      else {
        currentTimer->buzzingTimeInSec = 0;
        digitalWrite(BuzzerPin, LOW);
      }
    delay(21000);
  }
}

