/*
   Copyright [2023] [kasashiki]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <Keyboard.h>

const int A1pin = A1;
const int A2pin = A2;
const int A0pin = A0; 
const int A3pin = A3; 

//主にここの数値を変更して調整をしてください。
int k = 90; //kの感度
int d = 90; //dの感度
int f = 45; //fの感度
int j = 45; //jの感度
long int A = 11; //キー単体のdelay
long int B = 15; //キー全体のdelay
//感度調節


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

  if (a3 - sv3 >= k && time - ti3 > A && time- ti > B) {
  Keyboard.write('K');
  ti3 = millis();
  ti = millis();
  }
    if (a0 - sv0 >= d && time - ti0 > A && time- ti > B) {
  Keyboard.write('D');
  ti0 = millis();
  ti = millis();
  }
    if (a1 - sv1 >= f && time - ti1 > A && time- ti > B) { 
  Keyboard.write('F');
  ti1 = millis();
  ti = millis();
  }
    if (a2 - sv2 >= j && time - ti2 > A && time- ti > B) {
  Keyboard.write('J');
  ti2 = millis();
  ti = millis();
  }
  sv3 = a3;
  sv0 = a0;
  sv1 = a1;
  sv2 = a2;
 
}
