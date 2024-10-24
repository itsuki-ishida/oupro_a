/***************************************************
  HUSKYLENS An Easy-to-use AI Machine Vision Sensor
  <https://www.dfrobot.com/product-1922.html>

 ***************************************************
  This example shows the basic function of library for HUSKYLENS via Serial.

  Created 2020-03-13
  By [Angelo qiao](Angelo.qiao@dfrobot.com)

  GNU Lesser General Public License.
  See <http://www.gnu.org/licenses/> for details.
  All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
  1.Connection and Diagram can be found here
  <https://wiki.dfrobot.com/HUSKYLENS_V1.0_SKU_SEN0305_SEN0336#target_23>
  2.This code is tested on Arduino Uno, Leonardo, Mega boards.
 ****************************************************/

#define BLYNK_TEMPLATE_ID "TMPL6MIqKKVLW"
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "Tje2UKH81BzHPAIBEaOCO88u-qGQY0CJ"

#include <ESP32Servo.h> // <>で囲む場合：標準ライブラリやインストールされた外部ライブラリをインクルードする場合
#include <Wire.h>
#include <VL53L0X.h>
#include "HUSKYLENS.h" // ""で囲む場合：ユーザーがプロジェクト内で作成したファイルや特定のローカルディレクトリに存在するファイルをインクルードする場合
#include "SoftwareSerial.h"
#include <WiFi.h>
#include <WebServer.h>
#include <BlynkSimpleEsp32.h>

HUSKYLENS huskylens; // HUSKYLENSオブジェクトを生成
VL53L0X sensor; // VL53L0Xオブジェクトを生成


// 制御パラメータの定義
const float Kp = 1.5; //比例ゲイン
const float Ki = 0; //積分ゲイン

// 変数の定義
float dt;
float pretime;
float target;
float P;
float I;
float duty;
float range;
float start_time = 0;
float now_time = 0;

byte servoPinC = 25; // byteは8ビットのデータ型（0～255）
byte servoPinR = 26;
byte servoPinL = 27;
Servo servo_C; // Servoオブジェクトの生成（Servoはサーボモーターを制御するためのライブラリ）
Servo servo_R;
Servo servo_L;
bool back_flag = false;
bool start_flag = false;

char ssid[] = ssid; //文字列の宣言とssidの格納
char pass[] = pass;

WebServer server(80);

int R_Flag=0;
int C_Flag=0;
int L_Flag=0;

BLYNK_WRITE(V2)
{
  R_Flag = param.asInt();
  Serial.println("R");
  Serial.println(R_Flag);
  servo_R.writeMicroseconds(R_Flag);
  delay(5000);
}
BLYNK_WRITE(V0)
{
  L_Flag = param.asInt();
  Serial.println("L");
  Serial.println(L_Flag);
  servo_L.writeMicroseconds(L_Flag);
  delay(5000);
}
BLYNK_WRITE(V1)
{
  C_Flag = param.asInt();
  Serial.println("C");
  Serial.println(C_Flag);
  servo_C.writeMicroseconds(C_Flag);
  delay(5000);
}

SoftwareSerial mySerial(21, 22); // どこにesp32で繋げたか。21,22
//HUSKYLENS green line >> Pin 21; blue line >> Pin 22
void printResult(HUSKYLENSResult result);

void setup() {
  servo_C.attach(servoPinC);
  servo_R.attach(servoPinR);
  servo_L.attach(servoPinL);

  servo_C.writeMicroseconds(1500); // send "stop" signal to ESC.
  servo_R.writeMicroseconds(1500);
  servo_L.writeMicroseconds(1500);

  Serial.begin(115200);
  mySerial.begin(9600);

  WiFi.begin(ssid, pass);  // アクセスポイントに接続する
  while (WiFi.status() != WL_CONNECTED) { // 接続完了したかどうかを判断
    delay(500);                           // 完了していなければ500ミリ秒待つ
    Serial.println(".");
  }

  server.on("/", handleRoot);
  server.on("/RSERVO_ON", RSERVO_ON);
  server.on("/RSERVO_OFF", RSERVO_OFF);
  server.onNotFound(handleNotFound);
  // Webサーバーを起動
  server.begin();
  Serial.println("server begin");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  while (!huskylens.begin(mySerial))
  {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }

  /*
    //ここから距離センサの初期設定//
    //////////////////////////////////////////////////////////////
    //I2C通信ライブラリ(Wire)の初期化
    Wire.begin();

    //タイムアウトまでの時間をセット
    sensor.setTimeout(500);
    //距離センサ(VL53L0X)の初期化
    if (!sensor.init())
    {
      Serial.println("Failed to detect and initialize sensor!");
      while (1) {}
    }

    // Start continuous back-to-back mode (take readings as
    // fast as possible).  To use continuous timed mode
    // instead, provide a desired inter-measurement period in
    // ms (e.g. sensor.startContinuous(100)).
    sensor.startContinuous();
    //////////////////////////////////////////////////////////
    //距離センサ設定ここまで//

  */
  Serial.println(F("Begin!"));
}

void loop() {
  Blynk.run();
  if(WiFi.status()==WL_CONNECTED)
  {
    Serial.println("WiFi connected");       // シリアルモニタに「WiFi Connected」と出力
    Serial.println(WiFi.localIP());         //IP
    Serial.println(WiFi.subnetMask());      //subnet mask
    Serial.println(WiFi.RSSI());      //RSSI
  }
  else
  {
    Serial.println("x");
  }

  now_time = micros();

  if (now_time - start_time >= 180000000) { // スタートから180秒以上経っている場合
    if (start_flag == true) {
      servo_C.writeMicroseconds(1200);
      servo_L.writeMicroseconds(1200);
      servo_R.writeMicroseconds(1200);
      delay(5000);
      start_flag = false;
    }
    else
    {
      servo_C.writeMicroseconds(1500);
      servo_L.writeMicroseconds(1500);
      servo_R.writeMicroseconds(1500);
      delay(5000);
    }
  }

  // 微小時間を計算、ここは常に動かしておく
  // dt = (micros() - pretime) / 1000000;
  // pretime = micros();



  /*
    //距離センサで検知した距離
    int32_t range;
    range = (int32_t)sensor.readRangeContinuousMillimeters();
    Serial.println(range);
  */
  else // スタートから180秒以上経っていない場合
  {
    if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    else if (!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
    else if (!huskylens.available() && back_flag == false && start_flag == true) {
      Serial.println(F("not available"));
      servo_C.writeMicroseconds(1500);
      servo_R.writeMicroseconds(1440);
      servo_L.writeMicroseconds(1560);
      delay(500);
    }
    else if (!huskylens.available() && back_flag == false && start_flag == false) {
      servo_C.writeMicroseconds(1500);
      servo_R.writeMicroseconds(1500);
      servo_L.writeMicroseconds(1500);
      delay(500);
    }
    else // 何かしらのQRコードを読んだ場合
    {
      //////////////////////////////////////////このあたりが動作
      Serial.println(F("###########"));

      while (huskylens.available())
      {
        HUSKYLENSResult result = huskylens.read();
        printResult(result);

        int32_t error;
        error = (int32_t)result.xTarget - (int32_t)160;


        if (result.ID == 1 && back_flag == false && start_flag == false) {
          servo_C.writeMicroseconds(1400);
          servo_R.writeMicroseconds(1400);
          servo_L.writeMicroseconds(1400);
          
          start_flag = true;
          start_time = micros();
          delay(5000);
          servo_C.writeMicroseconds(1500);
          servo_R.writeMicroseconds(1500);
          servo_L.writeMicroseconds(1500);
          
        }

        else if (result.ID == 2 && back_flag == false && start_flag == true) {
          servo_C.writeMicroseconds(1500);
          servo_R.writeMicroseconds(1500);
          servo_L.writeMicroseconds(1500);
          start_flag = false;
        }

        else if (result.ID == 3 && back_flag == false && start_flag == true) {
          //以下はid=1（メインのコードが読みとれてる場合）のみに作動
          /*
          target = 160; // 画面のセンター
          P = (int32_t)result.xTarget - (int32_t)target;
          I += P * dt;
          duty = Kp * P + Ki * I; // この値をもとにR,Lをコントロール
          */
          // dutyが正なら右に曲がりたい
          /*if (duty >= 0) {
            servo_R.writeMicroseconds(1500 - duty);
            servo_L.writeMicroseconds(1500 - duty / 2);
            }
            else {
            servo_R.writeMicroseconds(1500 + duty / 2);
            servo_L.writeMicroseconds(1500 + duty);

            }
            servo_C.writeMicroseconds(1450);*/

          if (error > 120) { //右に曲がりたい時、すなわち画面の右サイドにある時
            servo_C.writeMicroseconds(1600);
            servo_R.writeMicroseconds(1400);
            servo_L.writeMicroseconds(1600);
            delay(300);
          }
          else if (error < -120) {
            servo_C.writeMicroseconds(1600);
            servo_R.writeMicroseconds(1600);
            servo_L.writeMicroseconds(1400);
            delay(300);
          }
          else {
            servo_C.writeMicroseconds(1600);
            servo_L.writeMicroseconds(1600);
            servo_R.writeMicroseconds(1600);
            delay(300);
          }

        }

        
      }
    }
      ///////////////////////////////////////////このあたりが動作
  }
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") + result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
  }
  else if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow:xOrigin=") + result.xOrigin + F(",yOrigin=") + result.yOrigin + F(",xTarget=") + result.xTarget + F(",yTarget=") + result.yTarget + F(",ID=") + result.ID);
  }
  else {
    Serial.println("Object unknown!");
  }
}

void handleRoot() {
  String html;
   
  // HTMLを組み立てる
  html = "<!DOCTYPE html>"; //「文書型の定義」 
  html += "<html>"; //htmlの文書であることを示す
  html += "<head>"; //ヘッダー情報
  html += "<meta charset=\"utf-8\">"; //文字コードを指定
  html += "<title>SERVOをONOFFする</title>"; //title情報
  html += "</head>";
  html += "<body>"; //文章の本文、ここの内容がwebページに表示される
  html += "<p>リンクをクリックするとLEDがON,OFFします</p>"; //<p>はテキストの段落を表す
  html += "<ul>"; //<ul>内で箇条書き開始、<li>内で各項目について記載  
  html += "<li><a href=\"/RSERVO_ON\">RSERVO_ON</a></li>"; //<a href="リンク先のＵＲＬ">アンカーテキスト</a>
  html += "<li><a href=\"/RSERVO_OFF\">RSERVO_OFF</a></li>"; //<a href="リンク先のＵＲＬ">アンカーテキスト</a>  
  html += "</ul>";//箇条書き終了
  html += "</body>";
  html += "</html>";
  
  // HTMLを出力する
  server.send(200, "text/html", html);
}

void RSERVO_ON(){
  servo_R.writeMicroseconds(1600);
  server.send(200, "text/plain; charset=utf-8", "RSERVOをONしました");
}

void RSERVO_OFF(){
  servo_R.writeMicroseconds(1500);
  server.send(200, "text/plain; charset=utf-8", "RSERVOをOFFしました");
}

void handleNotFound(void) {
  server.send(404, "text/plain", "Not Found");
}
