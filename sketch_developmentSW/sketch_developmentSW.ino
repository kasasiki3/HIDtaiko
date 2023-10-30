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

#include <SwitchControlLibrary.h>
int aa = 15;
int cc = 20;
const int A1pin = A1;
const int A2pin = A2;
const int A0pin = A0; 
const int A3pin = A3; 

//主にここの数値を変更して調整をしてください。
int k = 75; //kの感度
int d = 75; //dの感度
int f = 30; //fの感度
int j = 35; //jの感度
int A = 15; //キー単体のdelay
int B = 10; //ドンのdelay
int C = 10; //かdelay



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
    SwitchControlLibrary().pressButton(Button::LCLICK);
  SwitchControlLibrary().sendReport();
  delay(400);
  SwitchControlLibrary().releaseButton(Button::LCLICK); 
  SwitchControlLibrary().sendReport(); 
  delay(400);
    SwitchControlLibrary().pressButton(Button::LCLICK);
  SwitchControlLibrary().sendReport();
  delay(400);
  SwitchControlLibrary().releaseButton(Button::LCLICK); 
  SwitchControlLibrary().sendReport(); 
  delay(400);
    SwitchControlLibrary().pressButton(Button::LCLICK);
  SwitchControlLibrary().sendReport();
  delay(400);
  SwitchControlLibrary().releaseButton(Button::LCLICK); 
  SwitchControlLibrary().sendReport(); 
  delay(400);

}

void loop() {
  long int a3 = analogRead(A3pin);
  long int a0 = analogRead(A0pin); 
  long int a1 = analogRead(A1pin);
  long int a2 = analogRead(A2pin);
  time = millis();

  if (a3 - sv3 >= f && time - ti3 > A && time- ti > C) {//Button::LCLICK
  SwitchControlLibrary().pressButton(Button::LCLICK);
  SwitchControlLibrary().sendReport();
  delay(cc);
  SwitchControlLibrary().releaseButton(Button::LCLICK); 
  SwitchControlLibrary().sendReport(); 
  delay(aa);

  ti3 = millis();
  ti = millis();
  }
    if (a0 - sv0 >= d && time - ti0 > A && time- ti > C) {//Button::R
  SwitchControlLibrary().pressButton(Button::ZR);
  SwitchControlLibrary().sendReport();
  delay(cc);
  SwitchControlLibrary().releaseButton(Button::ZR); 
  SwitchControlLibrary().sendReport(); 
  delay(aa);
  ti0 = millis();
  ti = millis();
  }
    if (a1 - sv1 >= j && time - ti1 > A && time- ti > B) { //Button::RCLICK
  SwitchControlLibrary().pressButton(Button::RCLICK);
  SwitchControlLibrary().sendReport();
  delay(cc);
  SwitchControlLibrary().releaseButton(Button::RCLICK); 
  SwitchControlLibrary().sendReport(); 
  delay(aa);
  ti1 = millis();
  ti = millis();
  }
    if (a2 - sv2 >= d && time - ti2 > A && time- ti > B) {//Button::L
  SwitchControlLibrary().pressButton(Button::ZL);
  SwitchControlLibrary().sendReport(); 
  delay(cc);
  SwitchControlLibrary().releaseButton(Button::ZL); 
  SwitchControlLibrary().sendReport(); 
  delay(aa);
  ti2 = millis();
  ti = millis();
  }
  sv3 = a3;
  sv0 = a0;
  sv1 = a1;
  sv2 = a2;

}
