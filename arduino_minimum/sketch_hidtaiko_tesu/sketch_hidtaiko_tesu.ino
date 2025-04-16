#include <EEPROM.h>

int max_kando = 9;
int kando[9];
String cmds[9] = {"\0"};
int count = 0;
bool reada = false;

String part1 = "";  // `:` の前の部分
String part2 = "";  // `:` の後の部分
int receiveC = 0;
void setup() {
  Serial.begin(9600);
  delay(10);
}


void loop() {
  
  if(reada && Serial.available()){//データーを分割し、kandoに代入
    String rawdata = Serial.readStringUntil('\n');
    int index = rawdata.indexOf(':');
    if(index != -1){
      part1 = rawdata.substring(0, index);
      part2 = rawdata.substring(index + 1);
      kando[part1.toInt()] = part2.toInt();
      receiveC++;
      }
      if(receiveC == 9){
        reada = false;
        receiveC = 0;
        }
      /*
      Serial.println("Part 1: " + part1);
      Serial.println("Part 2: " + part2);
      */
  }
  
  
  if (Serial.available() > 0 && !reada) {  // reada=trueの時はコマンドを受け取らない
    char commandBuf[16];
    int len = Serial.readBytesUntil('\n', commandBuf, sizeof(commandBuf) - 1);
    commandBuf[len] = '\0';

   // Serial.print("Received command: ");
    //Serial.println(commandBuf);

    int command = atoi(commandBuf);

    if (command == 1000) {//最初に1000が送られてきたら感度を順番に送信する。例　0:100 1:200 3:300...
      for (int i = 0; i < max_kando; i++) {
        Serial.print(i);
        Serial.print(":");
        Serial.println(kando[i]);
        delay(40);
      }
    } else if (command == 1001) {//1001が送られてきた感度をEEPROMに保存
      for (int i = 0; i < max_kando; i++) {
        EEPROM.put(i, kando[i]);
        delay(10);
      }
    } else if (command == 1002) {//1002で感度受信モードに切り替え
      reada = true;
    }
  }

}
