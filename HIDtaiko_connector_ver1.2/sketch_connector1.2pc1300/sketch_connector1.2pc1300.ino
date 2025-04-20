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
int de = 220;
const int A1pin = A0;
const int A2pin = A2;
const int A0pin = A1; 
const int A3pin = A3; 

//(PC)それぞれの数値を変更することで感度を調k節ができます。　例 int lefts = 75; 
int se0 = 40;//左カッ　
int se1 = 25;//右ドン
int se2 = 25;//左ドン　
int se3 = 40;//右カッ　     

//int se0 = 160;//左カッ　
//int se1 = 220;//左ドン  
//int se2 = 220;//右ドン　
//int se3 = 160;//右カッ　  

//(PC)叩いた時に入力されるキーを変更できます。例 left = 'y';
char left = 'd';        //左カッ
char middleleft = 'f';  //左ドン
char middletight = 'k'; //右ドン
char right = 'j';       //右カッ

/*(PC)Aはどれかのキーが入力されてから、そのキーの次の入力を受け付けない時間です。
(PC)Bはどれかのキーが入力されてから、4キーすべての入力を受け付けない時間です。
*/
char A = 2; 
char B = 12; //何かkeyが押されてからドンの入力を受け付けない時間(ミリ秒)
char C = 30;//何かkeyが押されてからカッの入力を受け付けない時間(ミリ秒)
char p1 = 33;//面を叩いた時縁の入力を受け付けない時間　兼　縁を叩いた時面の入力を受け付けない時間(ミリ秒)
char ha = 20;//公式は20くらい　シュミレーターは0
long int sv1 =  0;
long int sv2 =  0;
long int sv3 =  0;
long int sv0 =  0;
long int ti1 =  0;
long int ti2 =  0;
long int ti3 =  0;
long int ti0 =  0;
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

}

void loop() {

  if(swswitching == false){
  long int a3 = analogRead(A3pin);
  long int a0 = analogRead(A0pin); 
  long int a1 = analogRead(A1pin);
  long int a2 = analogRead(A2pin);
  timec = millis();

  if (a3 - sv3 >= se3 && timec - ti3 > A && timec- ti > C && timec - ti1 > p1 && timec - ti2 > p1) {
  Keyboard.press(left);
  delay(ha);
  Keyboard.release(left);
  ti3 = millis();
  ti = millis();
  }
    if (a0 - sv0 >= se0 && timec - ti0 > A && timec- ti > C && timec - ti1 > p1 && timec - ti2 > p1) {
  Keyboard.press(right);
  delay(ha);
  Keyboard.release(right);
  ti0 = millis();
  ti = millis();
  }
    if (a1 - sv1 >= se1 && timec - ti1 > A && timec- ti > B && timec - ti0 > p1 && timec - ti3 > p1) { 
  Keyboard.press(middletight);
  delay(ha);
  Keyboard.release(middletight);
  ti1 = millis();
  ti = millis();
  }
    if (a2 - sv2 >= se2 && timec - ti2 > A && timec- ti > B && timec - ti0 > p1 && timec - ti3 > p1) {
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

  if (digitalRead(6) == LOW) {//上中　 
  Keyboard.write(KEY_RETURN); 
  delay(de);
  }

  if (digitalRead(9) == LOW) { //上右
  Keyboard.write(KEY_F1); 
  delay(de);
  }

  if (digitalRead(8) == LOW) { //下右
  Keyboard.write(KEY_INSERT); 
  delay(de);
  }
    if (digitalRead(7) == LOW) {//下中
  Keyboard.write(KEY_ESC); 
  delay(de);
  }
  }
}
  
  
  
 
