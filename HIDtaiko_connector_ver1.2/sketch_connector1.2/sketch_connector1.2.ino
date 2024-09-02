#include <Keyboard.h>
#include <SwitchControlLibrary.h>
int de = 220;
const int A1pin = A1;
const int A2pin = A2;
const int A0pin = A0; 
const int A3pin = A3; 

int se0 = 36;
int se1 = 33;
int se2 = 33;
int se3 = 36;     

char left = 'd';
char middleleft = 'f';  
char middletight = 'j';
char right = 'k';       

char A = 12; 
char B = 14;
char C = 27;
char p1 = 26;

char aa = 17;
char cc = 20;
char swA = 1;
char swB = 3;

long int sv1 =  0;
long int sv2 =  0;
long int sv3 =  0;
long int sv0 =  0;
long int ti1 =  0;
long int ti2 =  0;
long int ti3 =  0;
long int ti0 =  0;
long int time =  0;
long int timec =  0;
long int ti = 0;
bool swswitching = false;

void setup() {
  Serial.begin(9600);
  Keyboard.begin();
  pinMode(15, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  delay(100);
  if (digitalRead(15) == LOW) {
    swswitching = true;
    delay(de);
  }
  if (swswitching == true) {
    for (int i = 0; i < 6; i++) {
      SwitchControlLibrary().pressButton(Button::LCLICK);
      SwitchControlLibrary().sendReport();
      delay(400);
      SwitchControlLibrary().releaseButton(Button::LCLICK); 
      SwitchControlLibrary().sendReport();
      delay(400);
    }
  }
}

void loop() {
  if (swswitching == false) {
    long int a3 = analogRead(A3pin);
    long int a0 = analogRead(A0pin); 
    long int a1 = analogRead(A1pin);
    long int a2 = analogRead(A2pin);
    time = millis();

    if (a3 - sv3 >= se3 && time - ti3 > A && time - ti > C) {
      Keyboard.write(left);
      ti3 = millis();
      ti = millis();
    }
    if (a0 - sv0 >= se0 && time - ti0 > A && time - ti > C) {
      Keyboard.write(right);
      ti0 = millis();
      ti = millis();
    }
    if (a1 - sv1 >= se1 && time - ti1 > A && time - ti > B && time - ti0 > p1 && time - ti3 > p1) { 
      Keyboard.write(middletight);
      ti1 = millis();
      ti = millis();
    }
    if (a2 - sv2 >= se2 && time - ti2 > A && time - ti > B && time - ti0 > p1 && time - ti3 > p1) {
      Keyboard.write(middleleft);
      ti2 = millis();
      ti = millis();
    }
    sv3 = a3;
    sv0 = a0;
    sv1 = a1;
    sv2 = a2;
    if (digitalRead(2) == LOW) {
      Keyboard.write(KEY_UP_ARROW);
      delay(de);
    }
    if (digitalRead(3) == LOW) {
      Keyboard.write(KEY_RETURN); 
      delay(de);
    }
    if (digitalRead(4) == LOW) {
      Keyboard.write(KEY_F1); 
      delay(de);
    }
    if (digitalRead(5) == LOW) {
      Keyboard.write(KEY_INSERT); 
      delay(de);
    }
    if (digitalRead(6) == LOW) {
      Keyboard.write(KEY_ESC); 
      delay(de);
    }
    if (digitalRead(7) == LOW) {
      Keyboard.write(KEY_DOWN_ARROW); 
      delay(de);
    }
  } else {
    long int a3 = analogRead(A3pin);
    long int a0 = analogRead(A0pin); 
    long int a1 = analogRead(A1pin);
    long int a2 = analogRead(A2pin);
    time = millis();

    if (a3 - sv3 >= se3 && time - ti3 > swA && time - ti > swB) {
      SwitchControlLibrary().pressButton(Button::ZL);
      SwitchControlLibrary().sendReport();
      delay(cc);
      SwitchControlLibrary().releaseButton(Button::ZL); 
      SwitchControlLibrary().sendReport(); 
      delay(aa);
      ti3 = millis();
      ti = millis();
    }
    if (a0 - sv0 >= se0 && time - ti0 > swA && time - ti > swB) {
      SwitchControlLibrary().pressButton(Button::ZR);
      SwitchControlLibrary().sendReport(); 
      delay(cc);
      SwitchControlLibrary().releaseButton(Button::ZR); 
      SwitchControlLibrary().sendReport(); 
      delay(aa);
      ti0 = millis();
      ti = millis();
    }
    if (a1 - sv1 >= se1 && time - ti1 > swA && time - ti > swB) { 
      SwitchControlLibrary().pressButton(Button::RCLICK);
      SwitchControlLibrary().sendReport();
      delay(cc);
      SwitchControlLibrary().releaseButton(Button::RCLICK); 
      SwitchControlLibrary().sendReport(); 
      delay(aa);
      ti1 = millis();
      ti = millis();
    }
    if (a2 - sv2 >= se2 && time - ti2 > swA && time - ti > swB) {
      SwitchControlLibrary().pressButton(Button::LCLICK);
      SwitchControlLibrary().sendReport();
      delay(cc);
      SwitchControlLibrary().releaseButton(Button::LCLICK); 
      SwitchControlLibrary().sendReport(); 
      delay(aa);
      ti2 = millis();
      ti = millis();
    }
    sv3 = a3;
    sv0 = a0;
    sv1 = a1;
    sv2 = a2;
    if (digitalRead(2) == LOW) {
      SwitchControlLibrary().pressButton(Button::R);
      SwitchControlLibrary().sendReport();
      delay(de);
      SwitchControlLibrary().releaseButton(Button::R); 
      SwitchControlLibrary().sendReport(); 
      delay(de);
    }
    if (digitalRead(3) == LOW) {
      SwitchControlLibrary().pressButton(Button::PLUS);
      SwitchControlLibrary().sendReport();
      delay(de);
      SwitchControlLibrary().releaseButton(Button::PLUS); 
      SwitchControlLibrary().sendReport(); 
      delay(de);
    }
    if (digitalRead(4) == LOW) {
      SwitchControlLibrary().pressButton(Button::A);
      SwitchControlLibrary().sendReport();
      delay(de);
      SwitchControlLibrary().releaseButton(Button::A); 
      SwitchControlLibrary().sendReport(); 
      delay(de);
    }
    if (digitalRead(5) == LOW) {
      SwitchControlLibrary().pressButton(Button::B);
      SwitchControlLibrary().sendReport();
      delay(de);
      SwitchControlLibrary().releaseButton(Button::B); 
      SwitchControlLibrary().sendReport(); 
      delay(de);
    }
    if (digitalRead(6) == LOW) {
      SwitchControlLibrary().pressButton(Button::PLUS);
      SwitchControlLibrary().sendReport();
      delay(de);
      SwitchControlLibrary().releaseButton(Button::PLUS); 
      SwitchControlLibrary().sendReport(); 
      delay(de);
    }
    if (digitalRead(7) == LOW) {
      SwitchControlLibrary().pressButton(Button::L);
      SwitchControlLibrary().sendReport();
      delay(de);
      SwitchControlLibrary().releaseButton(Button::L); 
      SwitchControlLibrary().sendReport(); 
      delay(de);
    }
  }
}
