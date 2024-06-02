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
int keyl = 8;//レイヤー用
char ct = 110;//ディスプレイ操作のdelay


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

void setup() {
  pinMode(A_pin, INPUT_PULLUP);//ロータリーエンコーダの設定
  pinMode(B_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(A_pin), pulse_counter, CHANGE);


  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  pinMode(keyl, INPUT_PULLUP);
  pinMode(key0, INPUT_PULLUP);
 Serial.begin(9600);


  se0 = EEPROM.read(0);
    se1 = EEPROM.read(1);
      se2 = EEPROM.read(2);
        se3 = EEPROM.read(3);
}






void loop() {

  lastcount = count;
 // count = 0;
  //Serial.println(digitalRead(keyl));
delay(2);//このdelayがないとswitchが反応しねぇ
    if (digitalRead(keyl) == LOW && lastkeyl == HIGH) {//レイヤーの切り替え
    layer = !layer;
    if(layer == true){
     
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    //display.println("edit");
      display.println(F("edit mode"));
      display.display();
    }
    else if(layer == false){
    display.clearDisplay();
         display.display();
          EEPROM.write (0,se0);
              EEPROM.write (1,se1);
                  EEPROM.write (2,se2);
                      EEPROM.write (3,se3);
    }

delay(ct);
  }
//ここまでがレイヤーの切り替えdkkkkjkfkfffkfjffffjjjjjjjjjdddddddddddkff

if(layer == true){
      if (digitalRead(key0) == LOW && lastkey0 == HIGH ) {//key0が押されたら
    key0f = true;
    key1f = false;
    key2f = false;
    key3f = false;
    //他のキー情報を格納するフラッグをすべてfalseにする

      count = se0;
      clearArea2(5,10,30,40);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5 ,10);
    display.println("0");
  //    display.display();
      delay(ct);
      }
          if (digitalRead(key1) == LOW && lastkey1 == HIGH) {//key1の切り替え
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
//      display.display();
      delay(ct);
          }
          if (digitalRead(key2) == LOW && lastkey2 == HIGH) {//key0の切り替え
    key2f = true;
    key1f = false;
    key0f = false;
    key3f = false;
    //他のキー情報を格納するフラッグをすべてfalseにする
count = se2;
    clearArea2(5,10,30,40);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5 ,10);
    display.println("2");
  //    display.display();
      delay(ct);
          }
          if (digitalRead(key3) == LOW && lastkey3 == HIGH) {//key0の切り替え
    key3f = true;
    key1f = false;
    key2f = false;
    key0f = false;
    //他のキー情報を格納するフラッグをすべてfalseにする
      count = se3;
    clearArea2(5,10,30,40);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5 ,10);
    display.println("3");
  //    display.display();
  delay(ct);
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
    clearArea(74, 11, 55, 23);
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(74 ,11);//(x.y)
    display.println(count);
     display.display();



  }
//display.clearDisplay();
 Serial.println(digitalRead(key3));
//Serial.println(layer);
//Serial.println("a");
 lastkey0 = digitalRead(key0);
 lastkey1 = digitalRead(key1);
 lastkey2 = digitalRead(key2);
 lastkey3 = digitalRead(key3);


 lastkeyl = digitalRead(keyl);//レイヤー

}
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
     





