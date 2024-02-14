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
#include <SwitchControlLibrary.h>
int de = 220;
const int A1pin = A1;
const int A2pin = A2;
const int A0pin = A0; 
const int A3pin = A3; 


//(PC)それぞれの数値を変更することで感度を調節ができます。　例 int lefts = 75; 
int lefts = 50;           //左カッ　
int middlelefts = 55;     //左ドン  
int middlerights = 55;    //右ドン　
int rights = 65;          //右カッ　

//(PC)叩いた時に入力されるキーを変更できます。例 left = 'y';
char left = 'd';        //左カッ
char middleleft = 'f';  //左ドン
char middletight = 'j'; //右ドン
char right = 'k';       //右カッ

/*(PC)Aはどれかのキーが入力されてから、そのキーの次の入力を受け付けない時間です。
(PC)Bはどれかのキーが入力されてから、4キーすべての入力を受け付けない時間です。
*/
char A = 2; //キー単体のdelay
char B = 19; //キー全体のdelay
char C = 26;

//SW(調節はお勧めしません)
char aa = 17; //入力のdelay
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
  if(swswitching == true){
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
}

void loop() {

  if(swswitching == false){
  long int a3 = analogRead(A3pin);
  long int a0 = analogRead(A0pin); 
  long int a1 = analogRead(A1pin);
  long int a2 = analogRead(A2pin);
  time = millis();

  if (a3 - sv3 >= rights && time - ti3 > A && time- ti > C) {
  Keyboard.write(left);
  ti3 = millis();
  ti = millis();
  }
    if (a0 - sv0 >= lefts && time - ti0 > A && time- ti > C) {
  Keyboard.write(right);
  ti0 = millis();
  ti = millis();
  }
    if (a1 - sv1 >= middlelefts && time - ti1 > A && time- ti > B) { 
  Keyboard.write(middletight);
  ti1 = millis();
  ti = millis();
  }
    if (a2 - sv2 >= middlerights && time - ti2 > A && time- ti > B) {
  Keyboard.write(middleleft);
  ti2 = millis();
  ti = millis();
  }
  sv3 = a3;
  sv0 = a0;
  sv1 = a1;
  sv2 = a2;
    if (digitalRead(2) == LOW) {
  Keyboard.write(KEY_UP_ARROW);  //上左
  delay(de);
  }

  if (digitalRead(3) == LOW) {//上中　 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  Keyboard.write(KEY_RETURN); 
  delay(de);
  }

  if (digitalRead(4) == LOW) { //上右
  Keyboard.write(KEY_F1); 
  delay(de);
  }

  if (digitalRead(5) == LOW) { //下右
  Keyboard.write(KEY_INSERT); 
  delay(de);
  }
    if (digitalRead(6) == LOW) {//下中
  Keyboard.write(KEY_ESC); 
  delay(de);
  }
    if (digitalRead(7) == LOW) {//下左　 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  Keyboard.write(KEY_DOWN_ARROW); 
  delay(de);
  }
  }
  if(swswitching == true){
  long int a3 = analogRead(A3pin);
  long int a0 = analogRead(A0pin); 
  long int a1 = analogRead(A1pin);
  long int a2 = analogRead(A2pin);
  time = millis();

  if (a3 - sv3 >= lefts && time - ti3 > swA && time- ti > swB) {
  SwitchControlLibrary().pressButton(Button::ZL);
  SwitchControlLibrary().sendReport();
  delay(cc);
  SwitchControlLibrary().releaseButton(Button::ZL); 
  SwitchControlLibrary().sendReport(); 
  delay(aa);
  ti3 = millis();
  ti = millis();
  }
    if (a0 - sv0 >= rights && time - ti0 > swA && time- ti > swB) {
  SwitchControlLibrary().pressButton(Button::ZR);
  SwitchControlLibrary().sendReport(); 
  delay(cc);
  SwitchControlLibrary().releaseButton(Button::ZR); 
  SwitchControlLibrary().sendReport(); 
  delay(aa);
  ti0 = millis();
  ti = millis();
  }
    if (a1 - sv1 >= middlerights && time - ti1 > swA && time- ti > swB) { 
  SwitchControlLibrary().pressButton(Button::RCLICK);
  SwitchControlLibrary().sendReport();
  delay(cc);
  SwitchControlLibrary().releaseButton(Button::RCLICK); 
  SwitchControlLibrary().sendReport(); 
  delay(aa);

  ti1 = millis();
  ti = millis();
  }
    if (a2 - sv2 >= middlelefts && time - ti2 > swA && time- ti > swB) {
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
    if (digitalRead(2) == LOW) {//上左　 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  SwitchControlLibrary().pressButton(Button::R);
  SwitchControlLibrary().sendReport();
  delay(de);
  SwitchControlLibrary().releaseButton(Button::R); 
  SwitchControlLibrary().sendReport(); 
  delay(de);
  }
  if (digitalRead(3) == LOW) {//上中　 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  SwitchControlLibrary().pressButton(Button::PLUS);
  SwitchControlLibrary().sendReport();
  delay(de);
  SwitchControlLibrary().releaseButton(Button::PLUS); 
  SwitchControlLibrary().sendReport(); 
  delay(de);
  }

  if (digitalRead(4) == LOW) { //上右
  SwitchControlLibrary().pressButton(Button::A);
  SwitchControlLibrary().sendReport();
  delay(de);
  SwitchControlLibrary().releaseButton(Button::A); 
  SwitchControlLibrary().sendReport(); 
  delay(de);
  }


  if (digitalRead(5) == LOW) { //下右
  SwitchControlLibrary().pressButton(Button::B);
  SwitchControlLibrary().sendReport();
  delay(de);
  SwitchControlLibrary().releaseButton(Button::B); 
  SwitchControlLibrary().sendReport(); 
  delay(de);
  }

if (digitalRead(6) == LOW) { //下中
  SwitchControlLibrary().pressButton(Button::PLUS);
  SwitchControlLibrary().sendReport();
  delay(de);
  SwitchControlLibrary().releaseButton(Button::PLUS); 
  SwitchControlLibrary().sendReport(); 
  delay(de);
}
  if (digitalRead(7) == LOW) {//下左 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  SwitchControlLibrary().pressButton(Button::L);
  SwitchControlLibrary().sendReport();
  delay(de);
  SwitchControlLibrary().releaseButton(Button::L); 
  SwitchControlLibrary().sendReport(); 
  delay(de);
  }

  }
  }
 
