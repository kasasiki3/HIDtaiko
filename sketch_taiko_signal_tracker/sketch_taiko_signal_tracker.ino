
int de = 220;
const int A1pin = A1;
const int A2pin = A2;
const int A0pin = A0; 
const int A3pin = A3; 


//(PC)それぞれの数値を変更することで感度を調節ができます。　例 int lefts = 75; 
int lefts = 65;           //左カッ　
int middlelefts = 55;     //左ドン  
int middlerights = 55;    //右ドン　
int rights = 65;          //右カッ　

//(PC)叩いた時に入力されるキーを変更できます。例 left = 'y';
char left = 'd';        //左カッ
char middleleft = 'j';  //左ドン
char middletight = 'f'; //右ドン
char right = 'k';       //右カッ

/*(PC)Aはどれかのキーが入力されてから、そのキーの次の入力を受け付けない時間です。
(PC)Bはどれかのキーが入力されてから、4キーすべての入力を受け付けない時間です。
*/


long int ti1 =  0;
long int ti2 =  0;
long int ti3 =  0;
long int ti0 =  0;
long int time =  0;
long int ti = 0;
bool swswitching = false;
void setup() {
  Serial.begin(9600);
  pinMode(15, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  delay(100);


}

void loop() {

  if(swswitching == false){
  long int a3 = analogRead(A3pin);
  long int a0 = analogRead(A0pin); 
  long int a1 = analogRead(A1pin);
  long int a2 = analogRead(A2pin);
  
  Serial.print(a0);
  Serial.print(",");
  Serial.print(a3);//
  Serial.print(",");
  Serial.print(a1);
  Serial.print(",");
  Serial.print(a2);
  Serial.println(",");
//delay(10);

  

  }
}
  
  
  
 
