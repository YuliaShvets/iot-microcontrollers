#include <LiquidCrystal.h>

#define DDR_KEYPAD  DDRL
#define PORT_KEYPAD PORTL
#define PIN_KEYPAD  PINL
#include "keypad4x4.h"

const unsigned char buttonBuzzerPin = 0;
const unsigned char timer1BuzzerPin = 36;
const unsigned char timer2BuzzerPin = 35;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 19, rw = 20, enable = 21, a0 = 22, a1 = 23, a2 = 24, a3 = 25, a4 = 26, a5 = 27, a6 = 28, a7 = 29;
LiquidCrystal lcd(rs, rw, enable,  a0, a1, a2, a3, a4, a5, a6, a7);

const PROGMEM  char timeStrings[100][3] = {
  {"00"}, {"01"}, {"02"}, {"03"}, {"04"}, {"05"}, {"06"}, {"07"}, {"08"}, {"09"},
  {"10"}, {"11"}, {"12"}, {"13"}, {"14"}, {"15"}, {"16"}, {"17"}, {"18"}, {"19"},
  {"20"}, {"21"}, {"22"}, {"23"}, {"24"}, {"25"}, {"26"}, {"27"}, {"28"}, {"29"},
  {"30"}, {"31"}, {"32"}, {"33"}, {"34"}, {"35"}, {"36"}, {"37"}, {"38"}, {"39"},
  {"40"}, {"41"}, {"42"}, {"43"}, {"44"}, {"45"}, {"46"}, {"47"}, {"48"}, {"49"},
  {"50"}, {"51"}, {"52"}, {"53"}, {"54"}, {"55"}, {"56"}, {"57"}, {"58"}, {"59"},
  {"60"}, {"61"}, {"62"}, {"63"}, {"64"}, {"65"}, {"66"}, {"67"}, {"68"}, {"69"},
  {"70"}, {"71"}, {"72"}, {"73"}, {"74"}, {"75"}, {"76"}, {"77"}, {"78"}, {"79"},
  {"80"}, {"81"}, {"82"}, {"83"}, {"84"}, {"85"}, {"86"}, {"87"}, {"88"}, {"89"},
  {"90"}, {"91"}, {"92"}, {"93"}, {"94"}, {"95"}, {"96"}, {"97"}, {"98"}, {"99"},
};

const unsigned char BUZZING_TIME_IN_SEC = 30;

struct Time {
  unsigned char seconds, minutes, hours;
};

struct Timer {
  Time mainTime;
  unsigned char buzzingTimeInSec;
  bool isActive;
};

Timer timer1 = {{0,0,0}, 0, false};
Timer timer2 = {{0,0,0}, 0, false};

bool isSymbolInString(char * str, int n, char symb) {
  for (int i = 0; i < n; i++) {
    if (str[i] == symb)
      return true;
  }
  return false;
}

void buzzShort() {
  digitalWrite(buttonBuzzerPin, HIGH);
  delay(15);
  digitalWrite(buttonBuzzerPin, LOW);
}

void buzzLong() {
  digitalWrite(buttonBuzzerPin, HIGH);
  delay(200);
  digitalWrite(buttonBuzzerPin, LOW);
}

void LCDPrintString(char * str, int n, bool isPGM = false) {
  if (isPGM) {
    for (int i = 0; i < n; i++)
      lcd.write((char) pgm_read_byte( & (str[i])));
  } else {
    for (int i = 0; i < n; i++)
      lcd.write(str[i]);
  }
}

void LCDPrintTimerTime(Timer timer, unsigned char rowNum) {
  lcd.setCursor(4, rowNum);
  LCDPrintString(timeStrings[timer.mainTime.hours], 2, true);
  lcd.write(':');
  LCDPrintString(timeStrings[timer.mainTime.minutes], 2, true);
  lcd.write(':');
  LCDPrintString(timeStrings[timer.mainTime.seconds], 2, true);
}

void setTimer(Timer & timer, unsigned char rowNum) {
  unsigned char timerString[] = "  :  :  ";
  lcd.setCursor(4, rowNum);
  LCDPrintString(timerString, 8);
  unsigned char i = 0;

  while (i < 9) {
    if (isButtonPressed()) {
      buzzShort();
      unsigned char keyFromPad = readKeyFromPad4x4();

      if (keyFromPad == 'F') {
        lcd.setCursor(4, rowNum);
        LCDPrintString("Stopping", 8);
        buzzLong();
        return;
      }
        

      if (i == 8) {
        if (keyFromPad == 'E'){
          buzzShort();
          delay(100);
          buzzShort();
          i++;
        }
        else {
          lcd.setCursor(4, rowNum);
          LCDPrintString("#-confirm", 9);
          buzzLong();
          lcd.setCursor(4, rowNum);
          LCDPrintString(timerString, 8);
          lcd.write(' ');
        }
        continue;
      }

      if (isSymbolInString("0123456789", 10, keyFromPad)) {
        if (i == 3 or i == 6) {
          if (isSymbolInString("6789", 4, keyFromPad)) {
            lcd.setCursor(4, rowNum);
            LCDPrintString("Bad time", 8);
            buzzLong();
            lcd.setCursor(4, rowNum);
            LCDPrintString(timerString, 8);
            continue;
          }
        }
        timerString[i] = keyFromPad;
        i++;
        if (i == 2 or i == 5)
          i++;
      } else {
        lcd.setCursor(4, rowNum);
        LCDPrintString("Bad key", 7);
        buzzLong();
      }

      lcd.setCursor(4, rowNum);
      LCDPrintString(timerString, 8);
    }
  }

  timer.mainTime.hours = (timerString[0] - '0') * 10 + timerString[1] - '0';
  timer.mainTime.minutes = (timerString[3] - '0') * 10 + timerString[4] - '0';
  timer.mainTime.seconds = (timerString[6] - '0') * 10 + timerString[7] - '0';
}

void manageTimer(Timer & timer, unsigned char buzzerPin, unsigned char rowNum) {
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
      digitalWrite(buzzerPin, HIGH);
      lcd.setCursor(4, rowNum);
      LCDPrintString("00:00:00", 8);
    }
  } else if (timer.buzzingTimeInSec != 0) {
    timer.buzzingTimeInSec--;
    if ((BUZZING_TIME_IN_SEC-timer.buzzingTimeInSec) % 2 == 0)
      digitalWrite(buzzerPin, LOW);
    else
      digitalWrite(buzzerPin, HIGH);
  }
}

ISR(TIMER1_COMPA_vect) {
  manageTimer(timer1, timer1BuzzerPin, 0);
  manageTimer(timer2, timer2BuzzerPin, 1);
  
  if (timer1.isActive) {
    LCDPrintTimerTime(timer1, 0);
  }
  if (timer2.isActive) {
    LCDPrintTimerTime(timer2, 1);
  }
}

void setup() {
  noInterrupts();

  TCCR1A = 0x00;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); //CTC mode & Prescaler @ 1024
  TIMSK1 = (1 << OCIE1A); // дозвіл на переривання по співпадінню
  OCR1A = 0x3D08;

  initKeyPad();

  lcd.begin(16, 2);
  interrupts(); // Enable global interrupts

  pinMode(buttonBuzzerPin, OUTPUT);
  digitalWrite(buttonBuzzerPin, LOW);
  pinMode(timer1BuzzerPin, OUTPUT);
  digitalWrite(timer1BuzzerPin, LOW);
  pinMode(timer2BuzzerPin, OUTPUT);
  digitalWrite(timer2BuzzerPin, LOW);
  
  LCDPrintTimerTime(timer1, 0);
  LCDPrintTimerTime(timer2, 1);
}

void loop() {
  if (isButtonPressed()) {
    buzzShort();
    unsigned char keyFromPad = readKeyFromPad4x4();
    switch (keyFromPad) {
    case 'A':
      timer1.isActive = false;
      timer1.buzzingTimeInSec = 0;
      digitalWrite(timer1BuzzerPin, LOW);
      setTimer(timer1, 0);
      LCDPrintTimerTime(timer1, 0);
      break;
    case 'B':
      timer2.isActive = false;
      timer2.buzzingTimeInSec = 0;
      digitalWrite(timer2BuzzerPin, LOW);
      setTimer(timer2, 1);
      LCDPrintTimerTime(timer2, 1);
      break;
    case 'C':
      if (timer1.isActive) {
        timer1.isActive = false;
        timer1.mainTime.hours = 0;
        timer1.mainTime.minutes = 0;
        timer1.mainTime.seconds = 0;
        LCDPrintTimerTime(timer1, 0);
      }
      else if (!(timer1.mainTime.hours == 0 && timer1.mainTime.minutes == 0 && timer1.mainTime.seconds == 0))
        timer1.isActive = true;
      else {
        timer1.buzzingTimeInSec = 0;
        digitalWrite(timer1BuzzerPin, LOW);
      }
      break;
    case 'D':
      if (timer2.isActive) {
        timer2.isActive = false;
        timer2.mainTime.hours = 0;
        timer2.mainTime.minutes = 0;
        timer2.mainTime.seconds = 0;
        LCDPrintTimerTime(timer2, 1);
      }
      else if (!(timer2.mainTime.hours == 0 && timer2.mainTime.minutes == 0 && timer2.mainTime.seconds == 0))
        timer2.isActive = true;
      else {
        timer2.buzzingTimeInSec = 0;
        digitalWrite(timer2BuzzerPin, LOW);
      }
      break;
    }
  }
}
