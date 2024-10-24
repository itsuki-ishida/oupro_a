#include <ESP32Servo.h>
#include "HUSKYLENS.h"
#include <SoftwareSerial.h>

HUSKYLENS huskylens;

float dt;
float pretime;
float start_time = 0;
float now_time = 0;

byte servoPinC = 25;
byte servoPinR = 26;
byte servoPinL = 27;
Servo servo_C;
Servo servo_R;
Servo servo_L;
bool stop_flag = false;
bool back_flag = false;
bool start_flag = true;

SoftwareSerial mySerial(21, 22); // HUSKYLENS green line >> Pin 21; blue line >> Pin 22
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

  while (!huskylens.begin(mySerial)) {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }

  Serial.println(F("Begin!"));
}

void loop() {
  if (start_time != 0) {
    now_time = micros();
  }

  if (now_time - start_time >= 180000000) {
    servo_C.writeMicroseconds(1200);
    servo_L.writeMicroseconds(1200);
    servo_R.writeMicroseconds(1200);
    delay(15000);
    servo_C.writeMicroseconds(1500);
    servo_L.writeMicroseconds(1500);
    servo_R.writeMicroseconds(1500);
    start_time = 0;
    now_time = 0;
    start_flag = false;
  }

  // 微小時間を計算、ここは常に動かしておく
  dt = (micros() - pretime) / 1000000;
  pretime = micros();

  if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
  else if (!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
  else if (!huskylens.available() && stop_flag == false && back_flag == false && start_flag == true) {
    Serial.println(F("not available"));
    servo_C.writeMicroseconds(1500);
    servo_R.writeMicroseconds(1440);
    servo_L.writeMicroseconds(1560);
  } else {
    Serial.println(F("###########"));

    while (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();
      printResult(result);

      int32_t error;
      error = (int32_t)result.xTarget - (int32_t)160;

      if (result.ID == 1 && back_flag == false && start_flag == true) {
        //以下はid=1（メインのコードが読みとれてる場合）のみに作動
        if (error > 120) { //右に曲がりたい時、すなわち画面の右サイドにある時
          servo_C.writeMicroseconds(1525);
          servo_R.writeMicroseconds(1500);
          servo_L.writeMicroseconds(1475);
        } else if (error < -120) {
          servo_C.writeMicroseconds(1525);
          servo_R.writeMicroseconds(1500);
          servo_L.writeMicroseconds(1475);
        } else {
          servo_C.writeMicroseconds(1525);
          servo_L.writeMicroseconds(1525);
          servo_R.writeMicroseconds(1525);
        }
      } else if (result.ID == 2 && back_flag == false && start_flag == true) {
        if (start_time == 0) {
          start_time = micros();
        }
        //ゆっくり前進
        int servo_C = 1500; // 中央サーボ
        int servo_R = 1500; // 右側サーボ
        int servo_L = 1500; // 左側サーボ

        // errorが40から-40以内のときは特別な設定
        if (error >= -40 && error <= 40) {
          servo_C = 1550; // 中央サーボを1550に設定
          servo_R = 1500; // 右側サーボは1500
          servo_L = 1500; // 左側サーボは1500
        } else {
          // errorが40以上または-40以下の場合、通常の制御
          // 右側サーボの計算 (errorが負のときに減少)
          if (error < 0) {
            servo_R = 1500 + (error * 50 / 160); // errorが負のときに減少
          }

          // 左側サーボの計算 (errorが正のときに増加)
          if (error > 0) {
            servo_L = 1500 + (error * 50 / 160); // errorが正のときに増加
          }
        }

        // 計算結果をサーボモーターに適用
        servo_C.writeMicroseconds(servo_C);
        servo_R.writeMicroseconds(servo_R);
        servo_L.writeMicroseconds(servo_L);
      } else if (result.ID == 3 && back_flag == false && start_flag == true) {
        servo_C.writeMicroseconds(1400);
        servo_L.writeMicroseconds(1450);
        servo_R.writeMicroseconds(1400);
        delay(3000);
      } else if (result.ID == 4 && back_flag == false && start_flag == true) {
        servo_C.writeMicroseconds(1400);
        servo_L.writeMicroseconds(1400);
        servo_R.writeMicroseconds(1450);
        delay(3000);
      } else if (result.ID == 5 && back_flag == false && start_flag == false) {
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
    }
  }
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") + result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
  } else if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow:xOrigin=") + result.xOrigin + F(",yOrigin=") + result.yOrigin + F(",xTarget=") + result.xTarget + F(",yTarget=") + result.yTarget + F(",ID=") + result.ID);
  } else {
    Serial.println("Object unknown!");
  }
}
