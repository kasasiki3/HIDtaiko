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
#include <WebUSB.h>
#include <EEPROM.h>
#include <Keyboard.h>
   bool key6 = false;
   bool key7 = false;
   bool key8 = false;
   bool key9 = false;
    int pin6 = 6;
    int pin7 = 7;
    int pin8 = 8;
    int pin9 = 9;
    int pin2 = 2;
   //keyboard関係
WebUSB WebUSBSerial(1 /* https:// */, "kasasiki3.github.io/hidtaiko_webapp/ver1.3/");
#define  Serial WebUSBSerial
int color[9];
int colorIndex;
bool isConnected = false;
bool isConnectedLOG = false;
int de = 20;
const int A1pin = A1;
const int A2pin = A2;
const int A0pin = A0; 
const int A3pin = A3; 

//(PC)それぞれの数値を変更することで感度を調節ができます。　例 int lefts = 75; 
int se0 = 40;//左カッ　
int se1 = 33;//右ドン
int se2 = 40;//左ドン　
int se3 = 40;//右カッ　     

//(PC)叩いた時に入力されるキーを変更できます。例 left = 'y';
char left = 'd';        //左カッ
char middleleft = 'f';  //左ドン
char middletight = 'j'; //右ドン
char right = 'k';       //右カッ

/*(PC)Aはどれかのキーが入力されてから、そのキーの次の入力を受け付けない時間です。
(PC)Bはどれかのキーが入力されてから、4キーすべての入力を受け付けない時間です。
*/
char A = 16; 
char B = 10; //何かkeyが押されてからドンの入力を受け付けない時間(ミリ秒)
char C = 30;//何かkeyが押されてからカッの入力を受け付けない時間(ミリ秒)
char p1 = 26;//カッが入力されてからドンの入力を受け付けない時間(ミリ秒)
char ha = 0;

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
long int timec =  0;
long int ti = 0;
void setup() {
    pinMode(pin2, INPUT_PULLUP);
    Serial.begin(9600);
    if(digitalRead(pin2) == HIGH){
    while (!Serial) {
    ;
  }
  }
  Serial.write("Sketch begins.\r\n");
  Serial.flush();
  colorIndex = 0;
  



  int se0 = EEPROM.read(0);
  int se1 = EEPROM.read(1);
  int se2 = EEPROM.read(2);
  int se3 = EEPROM.read(3);
  int A = EEPROM.read(4);
  int B = EEPROM.read(5);
  int C = EEPROM.read(6);
  int p1 = EEPROM.read(7);
  int ha = EEPROM.read(8);
  color[0] = se0;
  color[1] = se1;
  color[2] = se2;
  color[3] = se3;
  color[4] = A;
  color[5] = B;
  color[6] = C;
  color[7] = p1;
  color[8] = ha;
  
//Serial.print(", ");
  Serial.print(se0);
  Serial.print("\n");
//        Serial.print(", ");
  Serial.print(se1);
  Serial.print("\n");
 //       Serial.print(", ");
   Serial.print(se2);
  Serial.print("\n");
  Serial.print(se3);
  Serial.print("\n");
 //       Serial.print(", ");
  Serial.print(A);
  Serial.print("\n");
 //       Serial.print(", ");
  Serial.print(B);
  Serial.print("\n");
 //       Serial.print(", ");
  Serial.print(C);
  Serial.print("\n");
//        Serial.print(", ");
  Serial.print(p1);
  Serial.print("\n");
  //Serial.print(", ");
    Serial.print(ha);
  Serial.print("\n");
  //Serial.print(", ");
 // Serial.print(color[8]);
  Serial.flush();
  delay(10);
  

  Keyboard.begin();
  pinMode(13,OUTPUT);
  pinMode(2, INPUT_PULLUP);//pc-sw
  pinMode(pin6, INPUT_PULLUP);  
  pinMode(pin7, INPUT_PULLUP);
  pinMode(pin8, INPUT_PULLUP);
  pinMode(pin9, INPUT_PULLUP);

 
}

void loop() {
  if (Serial && Serial.available()) {//入力があった時
  int seba = Serial.read();
  if(seba == 240){
      EEPROM.write (0,se0);
      EEPROM.write (1,se1);
      EEPROM.write (2,se2);
      EEPROM.write (3,se3);
      EEPROM.write (4,A);
      EEPROM.write (5,B);
      EEPROM.write (6,C);
      EEPROM.write (7,p1);
      EEPROM.write (8,ha);
  }
  else{
    color[colorIndex++] = seba;//代入
    if (colorIndex == 9) {
         se0 = color[0];
         se1 = color[1];
         se2 = color[2];
         se3 = color[3];
         A = color[4];
         B = color[5];
         C = color[6];
         p1 = color[7];
         ha = color[8];
    colorIndex = 0;
    }
  }
 }


  long int a3 = analogRead(A3pin);
  long int a0 = analogRead(A0pin); 
  long int a1 = analogRead(A1pin);
  long int a2 = analogRead(A2pin);
  time = millis();

  if (a3 - sv3 >= se3 && time - ti3 > A && time- ti > C) {
 // Keyboard.write(left);
  Keyboard.press(left);
  delay(ha);
  Keyboard.release(left);
  ti3 = millis();
  ti = millis();
  }
    if (a0 - sv0 >= se0 && time - ti0 > A && time- ti > C) {
 // Keyboard.write(right);
  Keyboard.press(right);
  delay(ha);
  Keyboard.release(right);
  ti0 = millis();
  ti = millis();
  }
    if (a1 - sv1 >= se1 && time - ti1 > A && time- ti > B && time - ti0 > p1 && time - ti3 > p1) { 
  //Keyboard.write(middletight);
  Keyboard.press(middletight);
  delay(ha);
  Keyboard.release(middletight);
  ti1 = millis();
  ti = millis();
  }
    if (a2 - sv2 >= se2 && time - ti2 > A && time- ti > B && time - ti0 > p1 && time - ti3 > p1) {
  //Keyboard.write(middleleft);
  Keyboard.press(middleleft);
  delay(ha);
  Keyboard.release(middleleft);
  ti2 = millis();
  ti = millis();
  }
  sv3 = a3;
  sv0 = a0;
  sv1 = a1;
  sv2 = a2;
  int p6 = digitalRead(pin6);
  int p7 = digitalRead(pin7);
  int p8 = digitalRead(pin8);
  int p9 = digitalRead(pin9);
  
    if (p6 == LOW && key6 == false) {//下中
  Keyboard.write(KEY_RETURN); 
  delay(de);
  }
    if (p7 == LOW && key7 == false) {//下左　 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  Keyboard.write(KEY_ESC); 
  delay(de);
  }
      if (p8 == LOW && key8 == false) {//下左　 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  Keyboard.write(KEY_INSERT); 
  delay(de);
  }
      if (p9 == LOW && key9 == false) {//下左　 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  Keyboard.write(KEY_F1); 
  delay(de);
  }

  if(p6 == LOW){
    key6 = true;
  }
  else{
    key6 = false;
  }
  if(p7 == LOW){
    key7 = true;
  }
  else{
    key7 = false;
  }
  if(p9 == LOW){
    key9 = true;
  }
  else{
    key9 = false;
  }
  if(p8 == LOW){
    key8 = true;
  }
      else{
    key8 = false;
   }



  
  

  }
 
