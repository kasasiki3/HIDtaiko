#include <Keyboard.h>
#include <SwitchControlLibrary.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
void pulse_counter(); // 関数の前方宣言
void clearArea(int16_t x, int16_t y, int16_t w, int16_t h); // 関数の前
const int A_pin = 0; // エンコーダーのpin
const int B_pin = 1;
int count = 0;
int lastcount = 0;




char key0 = 6;//左から順に
char key1 = 4;
char key2 = 7;
char key3 = 5;
char swpc = 9;
int keyl = 8;//レイヤー用
char ct = 10;//ディスプレイ操作のdelay
char ctt= 20;

int valuec = 0; 
bool delayc = false;
bool delayall = false;
bool delayallc = false;
bool layer = false;  // 現在のフラグ状態  keyL
bool lastkeyl = HIGH;  // 前回のスイッチの状態
bool key0f = false;  // 前回のスイッチの状態 key0
bool lastkey0 = HIGH;  // 前回のスイッチの状態
bool key1f = false;  // 前回のスイッチの状態 key1
bool lastkey1 = HIGH;  // 前回のスイッチの状態
bool key2f = false;  // 前回のスイッチの状態 key2
bool lastkey2 = HIGH;  // 前回のスイッチの状態
bool key3f = false;  // 前回のスイッチの状態 key3
bool lastkey3 = HIGH;  // 前回のスイッチの状態
Adafruit_SSD1306 display(-1);
int se0 = 0;
int se1 = 0;
int se2 = 0;
int se3 = 0;
int A1pin = A1;
int A2pin = A2;
int A0pin = A0;
int A3pin = A3;
char left = 'd';        //左カッ
char middleleft = 'f';  //左ドン
char middletight = 'j'; //右ドン
char right = 'k';       //右カッ
//SW(調節はお勧めしません)
char aa = 17; //入力のdelay
char cc = 20;
char swA = 1;
char swB = 3;
/*(PC)Aはどれかのキーが入力されてから、そのキーの次の入力を受け付けない時dk間です。
(PC)Bはどれかのキーが入力されてから、4キーすべての入力を受け付けない時間です。
*/
char A = 1; //キー単体のdelay
char B = 12; //キー全体のdelay
char C = 32;
char p1 = 35;//かっが入力された後のドンのディレイ


char de = 20;
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
  pinMode(A_pin, INPUT_PULLUP);//ロータリーエンコーダの設定
  pinMode(B_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(A_pin), pulse_counter, CHANGE);


  Keyboard.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  pinMode(swpc, INPUT_PULLUP);
  pinMode(keyl, INPUT_PULLUP);
  pinMode(key0, INPUT_PULLUP);
  pinMode(key1, INPUT_PULLUP);
  pinMode(key2, INPUT_PULLUP);
  pinMode(key3, INPUT_PULLUP);
 Serial.begin(9600);
 se0 = EEPROM.read(0);
    se1 = EEPROM.read(1);
      se2 = EEPROM.read(2);
        se3 = EEPROM.read(3);
        B = EEPROM.read(4);
        C = EEPROM.read(5);
        p1 = EEPROM.read(6);
  if (digitalRead(9) == LOW) {
  swswitching = true;
  delay(100);
  }
  if(swswitching == false){
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
 if(swswitching == true){
  long int a3 = analogRead(A3pin);
  long int a0 = analogRead(A0pin);
  long int a1 = analogRead(A1pin);
  long int a2 = analogRead(A2pin);
  time = millis();
    if (a3 - sv3 >= se3 && time - ti3 > A && time- ti > C) {
  Keyboard.write(right);
  ti3 = millis();
  ti = millis();
  }
    if (a0 - sv0 >= se0 && time - ti0 > A && time- ti > C) {
  Keyboard.write(left);
  ti0 = millis();
  ti = millis();
  }
 if (a1 - sv1 >= se1 && time - ti1 > A && time- ti > B && time - ti0 > p1 && time - ti3 > p1) {
  Keyboard.write(middleleft);
  ti1 = millis();
  ti = millis();
  }
    if (a2 - sv2 >= se2 && time - ti2 > A && time- ti > B && time - ti0 > p1 && time - ti3 > p1) {
  Keyboard.write(middletight);
  ti2 = millis();
  ti = millis();
  }
  sv3 = a3;
  sv0 = a0;
  sv1 = a1;
  sv2 = a2;
  /*
      if (digitalRead(key0) == LOW && lastkey0 == HIGH && layer == false) {//下中
  Keyboard.write(KEY_ESC); 
 delay(de);
  }
      if (digitalRead(key1) == LOW && lastkey1 == HIGH && layer == false) {//下左　 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  Keyboard.write(KEY_DOWN_ARROW); 
 delay(de);
  }
      if (digitalRead(key2) == LOW && lastkey2 == HIGH && layer == false) {
  Keyboard.write(KEY_UP_ARROW);  //上左
 delay(de);
  }
    if (digitalRead(key3) == LOW && lastkey3 == HIGH && layer == false) { //上右
  Keyboard.write(KEY_F1); 
 delay(de);
  }
  */
 }


   if(swswitching == false){
  long int a3 = analogRead(A3pin);
  long int a0 = analogRead(A0pin);
  long int a1 = analogRead(A1pin);
  long int a2 = analogRead(A2pin);
  time = millis();


  if (a3 - sv3 >= se3 && time - ti3 > swA && time- ti > swB) {
  SwitchControlLibrary().pressButton(Button::ZL);
  SwitchControlLibrary().sendReport();
  delay(cc);
  SwitchControlLibrary().releaseButton(Button::ZL);
  SwitchControlLibrary().sendReport();
  delay(aa);
  ti3 = millis();
  ti = millis();
  }
    if (a0 - sv0 >= se0 && time - ti0 > swA && time- ti > swB) {
  SwitchControlLibrary().pressButton(Button::ZR);
  SwitchControlLibrary().sendReport();
  delay(cc);
  SwitchControlLibrary().releaseButton(Button::ZR);
  SwitchControlLibrary().sendReport();
  delay(aa);
  ti0 = millis();
  ti = millis();
  }
    if (a1 - sv1 >= se1 && time - ti1 > swA && time- ti > swB) {
  SwitchControlLibrary().pressButton(Button::RCLICK);
  SwitchControlLibrary().sendReport();
  delay(cc);
  SwitchControlLibrary().releaseButton(Button::RCLICK);
  SwitchControlLibrary().sendReport();
  delay(aa);


  ti1 = millis();
  ti = millis();
  }
    if (a2 - sv2 >= se2 && time - ti2 > swA && time- ti > swB) {
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
    if (digitalRead(key0) == LOW) {//上左　 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  SwitchControlLibrary().pressButton(Button::R);
  SwitchControlLibrary().sendReport();
  delay(de);
  SwitchControlLibrary().releaseButton(Button::R);
  SwitchControlLibrary().sendReport();
  delay(de);
  }
  if (digitalRead(key1) == LOW) {//上中　 KEY_ESC) KEY_DOWN_ARROW KEY_RETURN
  SwitchControlLibrary().pressButton(Button::PLUS);
  SwitchControlLibrary().sendReport();
  delay(de);
  SwitchControlLibrary().releaseButton(Button::PLUS);
  SwitchControlLibrary().sendReport();
  delay(de);
  }


  if (digitalRead(key2) == LOW) { //上右
  SwitchControlLibrary().pressButton(Button::A);
  SwitchControlLibrary().sendReport();
  delay(de);
  SwitchControlLibrary().releaseButton(Button::A);
  SwitchControlLibrary().sendReport();
  delay(de);
  }
  if (digitalRead(key3) == LOW) { //下右
  SwitchControlLibrary().pressButton(Button::B);
  SwitchControlLibrary().sendReport();
  delay(de);
  SwitchControlLibrary().releaseButton(Button::B);
  SwitchControlLibrary().sendReport();
  delay(de);
  }
  }


//カウント変数を用意して、その数に応じて数値を変更するパーツを切り替える
//チャタは許されない
//変数の範囲を0から６に



    if (digitalRead(keyl) == LOW && lastkeyl == HIGH && layer == false) {//レイヤーの切り替え
    layer = true;
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    //display.println("edit");
      display.println(F("edit mode"));
      display.display();
    }
        else if(digitalRead(keyl) == LOW && lastkeyl == HIGH && layer == true){
      //HIDtaiko ver.2.0と表示させたりしたいね
      layer = false;
    display.clearDisplay();
         display.display();
      EEPROM.write (0,se0);
      EEPROM.write (1,se1);
      EEPROM.write (2,se2);
      EEPROM.write (3,se3);
      EEPROM.write (3,se3);
      EEPROM.write (4,B);
      EEPROM.write (5,C);
      EEPROM.write (6,p1);
    }


 
//ここまでがレイヤーの切り替え


if(layer == true){

   if (digitalRead(keyl) == LOW &&lastkeyl == HIGH) {
        display.clearDisplay();
         display.display();
          EEPROM.write (0,se0);
          EEPROM.write (1,se1);
          EEPROM.write (2,se2);
          EEPROM.write (3,se3);
          EEPROM.write (4,B);
          EEPROM.write (5,C);
          EEPROM.write (6,p1);
          layer = false;
          Serial.println("nannde?");
   }
          if (digitalRead(key0) == LOW && lastkey0 == HIGH){
    valuec++;
   // Serial.println("dasd");
    delay(5);
  }
  if (digitalRead(key1) == LOW && lastkey1 == HIGH){
    valuec--;
  //  Serial.println("dasd");
        delay(5);
  }
  if(valuec > 6){ //変数の値を0～６に制限する。
  valuec = 6;
}
else if(valuec < 0){
 valuec = 0;
}
Serial.println(valuec);

      if (valuec == 0 && key0f == false) {
      delay(ctt);
          delayall = false;
    delayallc = false;
    delayc = false;
    key0f = true;
    key1f = false;
    key2f = false;
    key3f = false;
count = se0;
      clearArea2(5,10,30,40);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5 ,10);
    display.println("0");
      display.display();
  //    delay(ct);
      }
          if (valuec == 1 && key1f == false) {
          delay(ctt);
              delayall = false;
    delayallc = false;
    delayc = false;
    key1f = true;
    key0f = false;
    key2f = false;
    key3f = false;
      count = se1;
    clearArea2(5,10,30,40);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5 ,10);
    display.println("1");
      display.display();
   //   delay(ct);
          }
          if (valuec == 2 && key2f == false) {
          delay(ctt);
              delayall = false;
    delayallc = false;
    delayc = false;
    key2f = true;
    key1f = false;
    key0f = false;
    key3f = false;
count = se2;
    clearArea2(5,10,30,40);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5 ,10);
    display.println("2");
      display.display();
   //   delay(ct);
          }
          if (valuec == 3 && key3f == false) {
          delay(ctt);
    delayall = false;
    delayallc = false;
    delayc = false;
    key3f = true;
    key1f = false;
    key2f = false;
    key0f = false;
      count = se3;
    clearArea2(5,10,30,40);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5 ,10);
    display.println("3");
      display.display();
 // delay(ct);
  }
  //こっからdelay系になる
  if(valuec == 4 && delayall == false){//Bのdelay
    delayall = true;
    delayallc = false;
    delayc = false;
    key1f = false;
    key2f = false;
    key0f = false;
    key3f = false;
    count = B;//全体delayの数値
    clearArea2(5,10,30,40);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5 ,10);
    display.println("4");
    display.display();
 // delay(ct);
  }
    if(valuec == 5 && delayallc == false){//Bのdelay
    delayall = false;
    delayallc = true;
    delayc = false;
    key1f = false;
    key2f = false;
    key0f = false;
    key3f = false;
    count = C;//全体delayの数値
    clearArea2(5,10,30,40);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5 ,10);
    display.println("5");
    display.display();
  //delay(ct);
  }
      if(valuec == 6 && delayc == false){//p1のdelay
    delayall = false;
    delayallc = false;
    delayc = true;
    key1f = false;
    key2f = false;
    key0f = false;
    key3f = false;
    count = p1;//
    clearArea2(5,10,30,40);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5 ,10);
    display.println("6");
    display.display();
  //delay(ct);
  }
   
   
   
   
 
 
  if(count != lastcount){//えんこ代入
 // Serial.println("AAD");
if(key0f == true){
    se0 = count;
}
else if(key2f == true){
  se2 = count;
}
else if(key3f == true){
  se3 = count;
}
else if(key1f == true){
  se1 = count;
}
else if(delayallc == true){
  C = count;
}
else if(delayall == true){
  B = count;
}
else if(delayc == true){
  p1 = count;
}
    clearArea(74, 11, 55, 23);
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(74 ,11);//(x.y)
    display.println(count);
    display.display();
   // delay(50);
  }
//display.clearDisplay();
 //Serial.println(digitalRead(key3));
//Serial.println(layer);
//Serial.println("a");



}
 lastkey0 = digitalRead(key0);
 lastkey1 = digitalRead(key1);
 lastkey2 = digitalRead(key2);
 lastkey3 = digitalRead(key3);
 lastkeyl = digitalRead(keyl);//レイヤー
 //lastkeyl = digitalRead(keyl);//レイヤー
 Serial.println(count); 
  lastcount = count;
}
         
void pulse_counter() {
 // count = 0;
  if(digitalRead(A_pin) ^ digitalRead(B_pin)) { //XOR　AとBが異なっていた時にfalseそれっていたらtrue
    count++;
  } else {
    count--;
  }
  }

void clearArea(int16_t x, int16_t y, int16_t w, int16_t h) {//塗りつぶし
    display.fillRect(x, y, w, h, SSD1306_BLACK);
}




void clearArea2(int16_t x, int16_t y, int16_t w, int16_t h) {//keyの塗りつぶし
    display.fillRect(x, y, w, h, SSD1306_BLACK);
}
     

