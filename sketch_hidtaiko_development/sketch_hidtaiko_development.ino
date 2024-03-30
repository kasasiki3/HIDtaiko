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
const int A1pin = A1;
const int A2pin = A2;
const int A0pin = A0; 
const int A3pin = A3; 


//(PC)それぞれの数値を変更することで感度を調節ができます。　例 int lefts = 75; 
int lefts = 65;           //左カッ　
int middlelefts = 55;     //左ドン  
int middlerights = 45;    //右ドン　
int rights = 65;          //右カッ　

//(PC)叩いた時に入力されるキーを変更できます。例 left = 'y';
char left = 'd';        //左カッ
char middleleft = 'j';  //左ドン
char middletight = 'f'; //右ドン
char right = 'k';       //右カッ

/*(PC)Aはどれかのキーが入力されてから、そのキーの次の入力を受け付けない時間です。
(PC)Bはどれかのキーが入力されてから、4キーすべての入力を受け付けない時間です。
*/
char A = 12; //キー単体のdelay
char B = 20; //キー全体のdelay
char C = 27;
char K = 23;
char K2 = 15;


bool daru0 = false;
bool daru1 = false;
bool daru2 = false;
bool daru3 = false;

int tia = 0;
int sagarichi = 20;
int agari = 15;
int sv32 = 0;
int sv22 = 0;
int sv12 = 0;
int sv02 = 0;
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
  daru3 = true;
  daru1 = true;
  daru2 = true;
  ti3 = millis();
  ti = millis();
  }
    if (a0 - sv0 >= lefts && time - ti0 > A && time- ti > C) {
  Keyboard.write(right);
  daru0 = true;
  daru1 = true;
  daru2 = true;
  ti0 = millis();
  ti = millis();
  }
    if (a1 - sv1 >= middlelefts && time - ti1 > A && time- ti > B) { 
  Keyboard.write(middletight);
  daru1 = true;
  ti1 = millis();
  ti = millis();
  }
    if (a2 - sv2 >= middlerights && time - ti2 > A && time- ti > B) {
  Keyboard.write(middleleft);
  daru2 = true;
  ti2 = millis();
  ti = millis();
  }
  if(sv32 - sv3 > sagarichi && a3 - sv3 > agari && time - ti3 > K && time - tia >K2 && daru3 == false){
    ti3 = millis();
    tia = millis();
 
  Keyboard.write(left);
    }
      if(sv22 - sv32 > sagarichi && a2 - sv2 > agari && time - ti2 > K && time - tia >K2 && daru2 == false && time - ti >15){
        ti2 = millis();
        tia = millis();
  Keyboard.write(middleleft);
    }
      if(sv12 - sv1 > sagarichi && a1 - sv1 > agari && time - ti1 > K && time - tia >K2 && daru1 == false && time - ti >15){
        ti1 = millis();
        tia = millis();
  Keyboard.write(middletight);
    }
      if(sv02 - sv0 > sagarichi && a0 - sv0 > agari && time - ti0 > K && time - tia >K2 && daru0 == false){
        ti0 = millis();
        tia = millis();
  Keyboard.write(right);
    }
  sv3 = a3;
  sv32 = sv3;
  
  sv0 = a0;
  sv02 = sv0;
  
  sv1 = a1;
  sv12 = a1;
  
  sv2 = a2;
  sv22 = sv2;





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
 daru0 = false;
 daru1 = false;
 daru2 = false;
 daru3 = false;
  }
}
  
  
  
