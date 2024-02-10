/*Arduinoからデータを送ってPythonで読む*/
/*参考文献*/
//[1]:https://tomson784.github.io/memo/python/arduino/2021/05/25/pyserial-arduino.html
//[2]:https://tanudon.work/499/
//[3]:https://spiceman.jp/arduino-cds-program/


/*ライブラリ*/
#include "BluetoothSerial.h"  //ESP32のBluetooth通信に使用
#define PI 3.141592653589793 //円周率

/*Bluetooth通信に必要*/
BluetoothSerial ESP_BT;  //ESP_BTという名前でオブジェクトを定義

/*ハードウェアの接続ピンの設定*/
#define SERVO_PWM_PIN 4    //サーボモータのPWMピン（信号入力ピン）をESP32の4番ピンに接続 ★回路と対応した番号にする
#define ESC_PWM_PIN 16     //ESCのPWMピン（信号入力ピン）をESP32の16番ピンに接続 ★回路と対応した番号にする

/*ライブラリ*/
#include <ESP32Servo.h> //ESC・サーボモータの制御に使用
#include <BLEDevice.h>  //Bluetooth通信用
#include <BLEServer.h>  //Bluetooth通信用    
#include <BLEUtils.h>   //Bluetooth通信用
#include <BLE2902.h>    //Bluetooth通信用

BLECharacteristic *pCharacteristic; //Characteristicを定義
bool deviceConnected = false; //接続状態を保存する変数

/*ESC,サーボのオブジェクト作成*/
Servo myservo;  // サーボモータを制御するためのServoオブジェクト作成
Servo myesc;    // ESCを制御するためのServoオブジェクト作成

int pulsew_min = 500;   //minimum pulse width of servo motor（サーボモータの最小パルス幅）
int pulsew_max = 2400;  //maximum pulse width of servo motor（サーボモータの最大パルス幅）

/*ステアリングの設定*/
unsigned long mov_speed_ST = 40;        //ステア移動速度
int center_pos = 95;                    //ステア中心位置 [サーボモータの中心位置 (90°)]　★まっすぐ走るように調整。90より大きい値にするとステア（ハンドル）が右寄りに、90より小さい値にするとステア（ハンドル）が左寄りになる
int left_DR = 20;                       //左の切れ角 ★:好みに合わせて調整。ただし大きくしすぎないように注意。
int right_DR = 25;                      //右の切れ角 ★:好みに合わせて調整。ただし大きくしすぎないように注意。
int left_max = center_pos - left_DR;    //左ステアの最大位置 [中心位置より反時計回りに20°（left_DR）回転した位置] ★逆に動くときはleft_DRの手前の符号をプラス（+）に
int right_max = center_pos + right_DR;  //右ステアの最大位置 [中心位置より時計回りに25°（right_DR）回転した位置] ★逆に動くときはright_DRの手前の符号をマイナス（-）に

/*スロットルの設定*/
unsigned long mov_speed_TH = 0;                //スロットル移動速度
unsigned long mov_speed_brk = 40;              //ブレーキ速度
int neutral_pos = 93;                          //中立位置 [スロットルの中立位置 (90) ★ESCの設定によってずれがあるので、前後に走行しないよう値を調整する。※ ESC側を90で中立になるよう設定（上級者向け。ESCの説明書通りプロポでニュートラル設定を済ませてから、このプログラムの値を調整するのがオススメ）してもよい。]
int forward_DR = 10;                           //前進の速さ ★好みの速度に調整
int backward_DR = 10;                          //バックの速さ ★好みの速度に調整
int forward_max = neutral_pos + forward_DR;    //前進の最大位置 ★逆に動くときはforward_DRの手前の符号をマイナス（-）に
int backward_max = neutral_pos - backward_DR;  //バックの最大位置 ★逆に動くときはbackward_DRの手前の符号をプラス（+）に
int turbo_speed = 120;                         //全開走行時の速度（180が最大。速すぎると思ったら170や160など値を小さくしてみる）

/*値設定の注意点*/
//速度(mov_speed_ST,mov_speed_TH)は 0-50 の範囲で与える。（0：最低速度、50:最大速度）
//スロットル、サーボモータの値(pos)の範囲は、 0≦ pos ≦180 で与える。
//myservo.write 関数には回転角を絶対的な位置で与える。例) 90°から 45°反時計回りに動いてほしいときは、-45ではなく、45を関数に入力する。

/*ステアとスロットルの位置を記憶する変数*/
int CH1 = center_pos;   //CH1:ステア
int CH2 = neutral_pos;  //CH2:スロットル

/*プログラムの流れを制御する変数*/
char input = 'C';             //入力信号
unsigned long curr;           /*現在時刻を保存*/
unsigned long prev_ST = 0;    /*前時刻を保存*/
unsigned long prev_TH = 0;    /*前時刻を保存*/



int val=0; //センサ入力値
float voltage=0; //電圧に変換したセンサ入力値
unsigned long prevReadTime_ControlSignal = millis(); // 制御命令の読み取り処理を始めた時間
long readPeriod_ControlSignal = 50; // 制御命令の読み取り周期 [ms]
unsigned long prevReadTime_Sensor1 = millis(); // センサ値の読み取り処理を始めた時間
long samplingRate_Sensor1 = 150; // センサの読み取り周期 [ms]
String received_data_byte;
String received_data = "1500";

/*PWM制御でサーボモータの角度を制御する関数*/
/*ステアを操作する関数*/
void change_ST_pos(int goal_pos, unsigned long mov_speed) {
  unsigned long delay_time = 50 - mov_speed;  //処理を遅くする時間（この値が大きいとゆっくりな操作に）

  if ((curr - prev_ST) >= delay_time) {
    /*ステアを切る処理*/
    //右折
    if (goal_pos - CH1 > 0) {
      CH1++;
      myservo.write(CH1);
      //左折
    } else if (goal_pos - CH1 < 0) {
      CH1--;
      myservo.write(CH1);

      //その他→ステアはそのまま
    } else {
      myservo.write(CH1);
    }
    prev_ST = curr;  //前回に処理を実行した時刻を現在時刻に更新
  }
}

/*スロットルを操作する関数*/
void change_TH_pos(int goal_pos, unsigned long mov_speed) {
  unsigned long delay_time = 50 - mov_speed;  //処理を遅くする時間（この値が大きいとゆっくりな操作に）

  if ((curr - prev_TH) >= delay_time) {
    /*スロットルを操作する処理*/
    //前進
    if (goal_pos - CH2 > 0) {
      CH2++;
      myesc.write(CH2);
      //後退
    } else if (goal_pos - CH2 < 0) {
      CH2--;
      myesc.write(CH2);

      //その他→スロットルはそのまま
    } else {
      myesc.write(CH2);
    }
    prev_TH = curr;  //前回に処理を実行した時刻を現在時刻に更新
  }
}


void setup() {
    Serial.begin(115200); //伝送速度設定
    ESP_BT.begin("ESP32_RC_Receiver");  //接続画面で表示される名前を設定 ★好きな名前にしてよい
    myservo.attach(SERVO_PWM_PIN, pulsew_min, pulsew_max);  //サーボモータのPWM端子とArduinoの4番ピンを接続 ★回路と対応した番号にする
    myesc.attach(ESC_PWM_PIN, pulsew_min, pulsew_max);      //ESCのPWM端子とArduinoの16番ピンを接続 ★回路と対応した番号にする
    Serial.println("Initializing ST and TH position....");
    myservo.write(center_pos);  // ステアを中心(Center)に
    myesc.write(neutral_pos);   //中立(Neutral)
    delay(2000);
}

void loop() {
  
    unsigned long currentTime = millis(); // 現在時刻を取得
    // lcd.setCursor(0,0);

    /* 1:Receive data from PC */
    if(currentTime-prevReadTime_ControlSignal>readPeriod_ControlSignal){
        prevReadTime_ControlSignal=currentTime; // 前時刻を現在の時刻に更新
        if(ESP_BT.available()>0){
          received_data_byte=ESP_BT.readStringUntil(';'); // read data from PC
          ESP_BT.println("received_data="+received_data_byte);
        }
    }
    
    /* 2:Send data to PC */
    if(currentTime-prevReadTime_Sensor1>samplingRate_Sensor1){
        prevReadTime_Sensor1=currentTime; // 前時刻を現在の時刻に更新
        val=analogRead(25); //センサの値などを読む
        voltage=float(val)*(3.3)/(4096); // AD変換
        ESP_BT.println((String)"V="+voltage+" [V]"); //文字をシリアルモニタに表示
    }

    curr = millis();  //現在時刻を取得
    /*命令に基づいてラジコンを制御*/
    if (received_data_byte == "forward") {
      change_ST_pos(center_pos, mov_speed_ST);   // ステアを中心(Center)に
      change_TH_pos(forward_max, mov_speed_TH);  //前進(Forward)
    } else if (received_data_byte == "backward") {
      change_ST_pos(center_pos, mov_speed_ST);    // ステアを中心(Center)に
      change_TH_pos(backward_max, mov_speed_TH);  //後退(Backward)
    } else if (received_data_byte == "stop") {
      forward_max = neutral_pos + forward_DR;     //標準の速度に設定
      change_ST_pos(center_pos, mov_speed_ST);    // ステアを中心(Center)に
      change_TH_pos(neutral_pos, mov_speed_brk);  //中立(Neutral)
    } else if (received_data_byte == "left") {
      change_ST_pos(left_max, mov_speed_ST);      // ステアを左(Left)に切る
      change_TH_pos(neutral_pos, mov_speed_brk);  //中立(Neutral)
    } else if (received_data_byte == "right") {
      change_ST_pos(right_max, mov_speed_ST);     // ステアを右(Right)に切る
      change_TH_pos(neutral_pos, mov_speed_brk);  //中立(Neutral)
    } else if (received_data_byte == "left_and_forward") {
      change_ST_pos(left_max, mov_speed_ST);     // ステアを左(Left)に切る
      change_TH_pos(forward_max, mov_speed_TH);  //前進(Forward)
    } else if (received_data_byte == "right_and_forward") {
      change_ST_pos(right_max, mov_speed_ST);    // ステアを右(Right)に切る
      change_TH_pos(forward_max, mov_speed_TH);  //前進(Forward)
    } else if (received_data_byte == "left_and_backward") {
      change_ST_pos(left_max, mov_speed_ST);      // ステアを左(Left)に切る
      change_TH_pos(backward_max, mov_speed_TH);  //後退(Backward)
    } else if (received_data_byte == "right_and_backward") {
      change_ST_pos(right_max, mov_speed_ST);     // ステアを右(Right)に切る
      change_TH_pos(backward_max, mov_speed_TH);  //後退(Backward)
    } else if (received_data_byte == "accelerate") {
      forward_max = turbo_speed;                 //ターボの速度に設定
      change_TH_pos(forward_max, mov_speed_TH);  //前進(Forward)
    } else if (received_data_byte == "deaccelerate") {
      forward_max = neutral_pos + forward_DR;    //標準の速度に設定
      change_TH_pos(forward_max, mov_speed_TH);  //前進(Forward)
    } else {
      change_ST_pos(center_pos, 50);   // ステアを中心(Center)に
      change_TH_pos(neutral_pos, 50);  //中立(Neutral)
    }
}


