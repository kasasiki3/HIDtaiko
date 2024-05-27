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
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
int de = 220;
const int A1pin = A1;
const int A2pin = A2;
const int A0pin = A0; 
const int A3pin = A3; 


//(PC)それぞれの数値を変更することで感度を調節ができます。　例 int lefts = 75; 
int lefts = 230;           //左カッ　
int middlelefts = 150;     //左ドン  
int middlerights = 150;    //右ドン　
int rights = 230;          //右カッ　

//(PC)叩いた時に入力されるキーを変更できます。例 left = 'y';
char left = 'd';        //左カッ
char middleleft = 'j';  //左ドン
char middletight = 'f'; //右ドン
char right = 'k';       //右カッ

/*(PC)Aはどれかのキーが入力されてから、そのキーの次の入力を受け付けない時間です。
(PC)Bはどれかのキーが入力されてから、4キーすべての入力を受け付けない時間です。
*/
char A = 3; //キー単体のdelay
char B = 22; //キー全体のdelay
char C = 25;

//SW(調節はお勧めしません)
char aa = 17; //入力のdelay
char cc = 20;
char swA = 1;
char swB = 3;
int keyb = 0;
bool key1 = false;
bool key2 = false;
bool key3 = false;
bool key4 = false;
int ct = 3;
bool nobu = false;
bool nobu0 = false;
int count = 0;
//OLED設定
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
void pulse_counter1();

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

char sw1 = 5;
char sw2 = 7;
char sw3 = 6;
char sw4 = 4;
char swr = 8;

int count1 = 1000;
int count2 = 1000;
int count3 = 1000;
int count4 = 1000;
const int Apin= 0; 
const int Bpin = 1;
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
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));

    if (digitalRead(9) == LOW) {
  swswitching = true;
  delay(de); 
  }
  count1 = EEPROM.read(1);
  count2 = EEPROM.read(2);
  count3 = EEPROM.read(3);
  count4 = EEPROM.read(4);
  Serial.begin(9600);
  Keyboard.begin();
  int count1 = EEPROM.read(1);
  int count2 = EEPROM.read(2);
  int count3 = EEPROM.read(3);
  int count4 = EEPROM.read(4);
  pinMode(15, INPUT_PULLUP);//sw切り替え

  pinMode(sw1, INPUT_PULLUP); //keyboard
  pinMode(sw2, INPUT_PULLUP); //keyboard
  pinMode(sw3, INPUT_PULLUP); //keyboard 
  pinMode(sw4, INPUT_PULLUP); //keyboard
  pinMode(swr, INPUT_PULLUP); //感度調節切り替え
  //

  pinMode(Apin, INPUT_PULLUP);//ロータリーエンコーダー
  pinMode(Bpin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Apin), pulse_counter1, CHANGE);

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
   lefts = EEPROM.read(1);         //左カッ　
   middlelefts = EEPROM.read(2); 
   middlerights = EEPROM.read(3); 
   rights = EEPROM.read(4);    

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


  if (digitalRead(8) == LOW) { //上右
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
//ここから下が新しいコード

  if(digitalRead(swr) == LOW ){
  nobu = true;
  delay(500);
  }

  while(nobu == true){
   // Serial.println("righfts");

    if(digitalRead(swr) == LOW){
        display.clearDisplay();
    nobu = false;
    key2 = false;
    key3 = false;
    key4 = false;
    key1 = false;
      delay(500);
    }
    /*
    if(digitalRead(Apin) ^ digitalRead(Bpin)) {
    count++;
    delay(10);
  } else {
    count--;
        delay(10);
  }
  */
    display.clearDisplay();
  Serial.println(count);

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("select"));

  if(key1 == true){
  display.setTextSize(4);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(60,30);   
  display.print(count1); //display.println(55, HEX);
  }
    if(key2 == true){
  display.setTextSize(4);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(60,30);   
  display.print(count2); //display.println(55, HEX);
  }
    if(key3 == true){
  display.setTextSize(4);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(60,30);   
  display.print(count3); //display.println(55, HEX);
  }
    if(key4 == true){
  display.setTextSize(4);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(60,30);   
  display.print(count4); //display.println(55, HEX);
  }

  display.setTextSize(3);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(10,30);   
  display.print(keyb); //display.println(55, HEX);

  display.display();
  delay(20);
  
  if(digitalRead(sw1) == LOW ){
  count = count1;
  key1 = true; 
  key2 = false;
  key3 = false;
  key4 = false;
//  delay(ct);
  }
    if(digitalRead(sw2) == LOW ){
        count = count2;
  key2 = true; 
  key1 = false;
  key3 = false;
  key4 = false;
 //   delay(ct);
  }
    if(digitalRead(sw3) == LOW ){
        count = count3;
  key3 = true; 
  key2 = false;
  key1 = false;
  key4 = false;
 //   delay(ct);
  }
    if(digitalRead(sw4) == LOW ){
        count = count4;
  key4 = true; 
  key2 = false;
  key3 = false;
  key1 = false;
 //   delay(ct);
  }
  if(key1 == true){
    count1 = count;
    EEPROM.write (1, count1);
   keyb = 1;
  }
    if(key2 == true){
          count2 = count;
    EEPROM.write (2, count2);
   keyb = 2;
  }
    if(key3 == true){
          count3 = count;
    EEPROM.write (3, count3);
   keyb = 3;
  }
    if(key4 == true){
          count4 = count;
    EEPROM.write (4, count4);
   keyb = 4;


  }
   //Serial.println(count);
   // Serial.println(middlerights);
   // Serial.println(middlelefts);
   // Serial.println(lefts);
    Serial.println(rights);

  }
  }

  void pulse_counter1() {
  if(digitalRead(Apin) ^ digitalRead(Bpin) ) {
    count++;
   // delay(ct);
  } else {
    count--;
   // delay(ct);
  }
}

#define XPOS   0 // Indexes into the 'icons' array in function below
#define YPOS   1
#define DELTAY 2

void testanimate(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  int8_t f, icons[NUMFLAKES][3];

  // Initialize 'snowflake' positions
  for(f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
    icons[f][YPOS]   = -LOGO_HEIGHT;
    icons[f][DELTAY] = random(1, 6);
    Serial.print(F("x: "));
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(F(" y: "));
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(F(" dy: "));
    Serial.println(icons[f][DELTAY], DEC);
  }
  }
 
