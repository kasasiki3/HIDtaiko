#include <Keyboard.h>

const int A1pin = A1;
const int A2pin = A2;
const int A0pin = A0; 
const int A3pin = A3; 


long int sv1 =  0;
long int sv2 =  0;
long int sv3 =  0;
long int sv0 =  0;
long int ti1 =  0;
long int ti2 =  0;
long int ti3 =  0;
long int ti0 =  0;
long int time =  0;
long int ti = 0;
long int A = 25;//キーごと
long int B = 18; //全体
void setup() {
  Serial.begin(9600);
  Keyboard.begin();



}

void loop() {
  long int a3 = analogRead(A3pin);
  long int a0 = analogRead(A0pin); 
  long int a1 = analogRead(A1pin);
  long int a2 = analogRead(A2pin);
  time = millis();

  if (a3 - sv3 >= 190 && time - ti3 > A && time- ti > B) {
  Keyboard.write('K');
  ti3 = millis();
  ti = millis();
  }
    if (a0 - sv0 >= 200 && time - ti0 > A && time- ti > B) {
  Keyboard.write('D');
  ti0 = millis();
  ti = millis();
  }
    if (a1 - sv1 >= 57 && time - ti1 > A && time- ti > B) { 
  Keyboard.write('F');
  ti1 = millis();
  ti = millis();
  }
    if (a2 - sv2 >= 65&& time - ti2 > A && time- ti > B) {
  Keyboard.write('J');
  ti2 = millis();
  ti = millis();
  }
  sv3 = a3;
  sv0 = a0;
  sv1 = a1;
  sv2 = a2;
 
}




