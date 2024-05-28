#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
char key0 = 6;//左から順に
char key1 = 4;
char key2 = 7;
char key3 = 5;
int keyl = 8;//レイヤー用
char ct = 50;//ディスプレイ操作のdelay

bool layer = false;  // 現在のフラグ状態
bool lastkeylState = HIGH;  // 前回のスイッチの状態

bool key0f = false;  // 前回のスイッチの状態
bool lastkey0State = HIGH;  // 前回のスイッチの状態

Adafruit_SSD1306 display(-1);
int til = 0;
void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(keyl, INPUT_PULLUP);
  pinMode(key0, INPUT_PULLUP);
Serial.begin(9600);

}



void loop() {
  //Serial.println(digitalRead(keyl));
delay(5);
    if (digitalRead(keyl) == LOW && lastkeylState == HIGH) {//レイヤーの切り替え
    layer = !layer;
    key0f = false;//レイヤーが切り替わったら各keyを初期化
    delay(ct);  
  }
      if (digitalRead(key0) == LOW && lastkey0State == HIGH) {//key0の切り替え
    key0f = !key0f;
    delay(ct);  
  }


if(layer == true){
  

 display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("edit");
  display.display();
  if(key0f == true){
    //ローターリーエンコードの値を感度変数に反映させる
    //感度の値をディスプレイに表示

  }
}



Serial.println(layer);
 lastkey0State = digitalRead(key0);
 lastkeylState = digitalRead(keyl);//レイヤー

}




 

  

 


