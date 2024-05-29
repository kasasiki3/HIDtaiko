#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
const int A_pin = 0; // エンコーダーのpin
const int B_pin = 1;
int count = 0;
int lastcount = 0;

char key0 = 6;//左から順に
char key1 = 4;
char key2 = 7;
char key3 = 5;
int keyl = 8;//レイヤー用
char ct = 50;//ディスプレイ操作のdelay

bool layer = false;  // 現在のフラグ状態
bool lastkeyl = HIGH;  // 前回のスイッチの状態

bool key0f = false;  // 前回のスイッチの状態
bool lastkey0 = HIGH;  // 前回のスイッチの状態


Adafruit_SSD1306 display(-1);
int se0 = 0;
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

}



void loop() {
  lastcount = count;
  count = 0;
  //Serial.println(digitalRead(keyl));
delay(1);//このdelayがないとswitchが反応しねぇ
    if (digitalRead(keyl) == LOW && lastkeyl == HIGH) {//レイヤーの切り替え
    layer = !layer;
    if(layer == true){
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("edit");
      display.display();
    }
    else if(layer == false){
      //HIDtaiko ver.2.0と表示させたりしたいね
    }

  }
//ここまでがレイヤーの切り替え

      if (digitalRead(key0) == LOW && lastkey0 == HIGH) {//key0の切り替え
    key0f = !key0f;
    //他のキー情報を格納するフラッグをすべてfalseにする
    if(key0f == true){
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10 ,10);
    display.println("0");
      display.display();
    }
  
  }
  if(key0f == true && count != lastcount){//えんこの代入
 // Serial.println("AAD");
    se0 += count;
    //display.clearDisplay();
    clearArea(31, 5, 35, 21);
    EEPROM.write (0,se0);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(30 ,10);
    display.println(se0);
     display.display(); 

  }


  


 //display.clearDisplay();
 



//Serial.println(layer);
Serial.println(se0);
 lastkey0 = digitalRead(key0);
 lastkeyl = digitalRead(keyl);//レイヤー

}
void pulse_counter() {
 // count = 0;
  if(digitalRead(A_pin) ^ digitalRead(B_pin)) { //XOR　AとBが異なっていた時にfalseそれっていたらtrue
    count += 5;
  } else {
    count -= 5;
  }

}

void clearArea(int16_t x, int16_t y, int16_t w, int16_t h) {//塗りつぶし
    display.fillRect(x, y, w, h, SSD1306_BLACK);
}




 

  

 


