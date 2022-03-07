// Pillola - Smart pill dispenser
// © 2021 Pranav Ramesh. All rights reserved.
// This code may not be duplicated or distributed
// elsewhere in any manner by any party. All rights to
// the code are attributed to Pranav Ramesh.

#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>
LiquidCrystal lcd = LiquidCrystal(8,9,13,12,11,10);

char hexaKeys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[4] = {A0,A1,A2,A3};
byte colPins[4] = {4,5,6,7};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, 4, 4);

Servo dialServo;

SoftwareSerial mySerial(2,3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int getFingerprintID() {
  
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  dialServo.attach(A5, 300, 2500);
  //  dialServo.attach(A4);
  dialServo.write(4);
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor");
  } else {
    Serial.println("Did not find fingerprint sensor");
    lcd.print("! No fingerprint");
    lcd.setCursor(0, 1); lcd.print("sensor found.");
    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

int i = 0;
char passcode[4] = {'1', '3', '7', '9'};
char key;

boolean authenticateKeypad(String message) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(message);
  lcd.setCursor(0, 1); lcd.print("Passcode: ");
  short digit = 0;
  char givenCode[4];
  bool correct;
  do {
    key = customKeypad.getKey();
    if (key >= 48 && key < 58 && digit < 4) {
      givenCode[digit] = key;
      Serial.print("Digit "); Serial.print(digit);
      Serial.print(":"); Serial.println(key);
      correct = givenCode[digit] == passcode[digit++];
      lcd.print(key);
    } else if (key == 'B' && digit >= 0) {
      digit = max(0, digit - 1);
      lcd.setCursor(10 + digit, 1);
      lcd.print(" "); lcd.setCursor(10 + digit, 1);
      givenCode[digit] = '\0';
    } else if (key == 'C' && digit == 4) {
      if (correct) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Correct");
        lcd.setCursor(0, 1); lcd.print("passcode.");
        delay(1500); lcd.clear();
      } else {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Incorrect");
        lcd.setCursor(0, 1); lcd.print("passcode.");
        delay(1500); lcd.clear();
      }
      return correct;
    }
  } while (true);
}

void loop() {
  lcd.setCursor(0, 0);
  key = customKeypad.getKey();

  // No action
  if (!key) {
    lcd.print("Pillola");
  }

  // A
  if (key == 'A') {
    if (authenticateKeypad("Dispense pill")) {
      dialServo.write(180);
      delay(1500);
      dialServo.write(4);
    }
  }

  // B
  else if (key == 'B') {
    lcd.print("Looking for");
    lcd.setCursor(0, 1); lcd.print("fingerprint...");
    int id = getFingerprintID();
    while (id <= 0) {
      key = customKeypad.getKey();
      if (key == 'D') {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Canceled.");
        delay(1500); lcd.clear();
        break;
      }
      id = getFingerprintID();
    }
    if (id > 0) {
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Successfully");
      lcd.setCursor(0, 1); lcd.print("authenticated");
      delay(1500); dialServo.write(180);
      delay(1500); dialServo.write(4);
      lcd.clear();

    }
  }

  // C
  else if (key == 'C') {
    lcd.print("C");
    lcd.setCursor(0, 1); lcd.print("Not assigned.");
    delay(1000); lcd.clear();
  }

  // D
  else if (key == 'D') {
    lcd.print("D");
    lcd.setCursor(0, 1); lcd.print("Not assigned.");
    delay(1000); lcd.clear();
  }
}
