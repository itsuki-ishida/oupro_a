//***************************************************
//   Sample program of the 4 wheels vehicle
//   Course : Applied Project 2023
//   Instructor : Ken Takagi 

//Definition of output pin numbers for OSOYOO Model-X Motor Driver
#define IN1L 8   // IN3 of OSOYOO Model-X  
#define IN2L 9   // IN4 of OSOYOO Model-X
#define ENAL 10  // ENB of OSOYOO Model-X
#define IN1R 11  // IN1 of OSOYOO Model-X
#define IN2R 12  // IN2 of OSOYOO Model-X
#define ENAR 13  // ENA of OSOYOO Model-X

#include "HUSKYLENS.h"       //HuskyLens sub-routine
#include "SoftwareSerial.h"  // Software serial communication sub-routine

HUSKYLENS huskylens;
SoftwareSerial mySerial(2, 3);  // RX, TX //HUSKYLENS green line >> Pin 2; blue line >> Pin 3 : Connection with Arduino Uno

int x;         // x coordinate of the block(tag) center
int onof = 0;  // Start/Stop switch 1>>start 0>>sop

void setup() {
  pinMode(IN1L, OUTPUT);  // Setup Arduino pins for output
  pinMode(IN2L, OUTPUT);
  pinMode(ENAL, OUTPUT);
  pinMode(IN1R, OUTPUT);
  pinMode(IN2R, OUTPUT);
  pinMode(ENAR, OUTPUT);

  Serial.begin(9600);    //Setup Serial communication speed
  mySerial.begin(9600);  //Setup Software communication speed

  while (!huskylens.begin(mySerial))  //Start check of Huskylens
  {
    Serial.print(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }
}

void loop() {
  if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
  else if (!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
  else if (!huskylens.available()) {  // Is Huskylens recognizing any tag?
    if (onof == 1) { turning();}
  }
  else {
    while (huskylens.available()) {  //Huskylens is recognizing a tag.
      HUSKYLENSResult result = huskylens.read();
      Serial.println(result.ID);
      if (result.ID == 1) {
        onof = 1;  // Tag 1 (ID == 1) >> Start
        int ItL = 150, ItR = 150 ;  //Speed of the left and right wheel

        digitalWrite(IN1L,LOW);  // Turn the left wheels to move backward (LOW and HIGH)
        digitalWrite(IN2L,HIGH);
        digitalWrite(IN1R,LOW); // Turn the right wheels to move forward (HIGH and LOW)
        digitalWrite(IN2R,HIGH);
      
        analogWrite(ENAL,ItL); //Power output
        analogWrite(ENAR,ItR);
        delay(100);            //Continue for 200ms
      
        digitalWrite(IN1L,LOW); //Stop the wheels
        digitalWrite(IN2L,LOW);
        digitalWrite(IN1R,LOW);
        digitalWrite(IN2R,LOW);
      }
      else if (result.ID == 2) {
        onof = 0;  // Tag 2 (ID == 2) >> Stop
        stop();
      }
      if (onof == 1) {
        if (result.ID == 3) {  //Tag 3 (ID == 3) >> Run to the target
          x = result.xCenter;  //x coordinate of of the tag 3 in the Huskylens display 
          target(x);}
        else { turning();
        }
      }
    }       
    Serial.print('Q');  // Header of the on/off status
    Serial.println(onof);  // Print out the on/off status
  }
} 


void target(int x) {  // Running to the target
  int ImaxL = 150, ImaxR = 150 ;  //Maximum speed of the left and right wheel
  int IL, IR;      //Speed of left and right wheel
  Serial.print('X');  //Header
  Serial.println(x);  //x-coordinate of the block monitering

  digitalWrite(IN1L,HIGH);  // Turn the left wheels to move forward (HIGH and LOW)
  digitalWrite(IN2L,LOW);
  digitalWrite(IN1R,HIGH);  // Turn the right wheels to move forward (HIGH and LOW)
  digitalWrite(IN2R,LOW);
  if (x < 160) {
    IR = ImaxR;
    IL = ImaxL/160.0*x;}
  else {
    IR = 2*ImaxR-ImaxR/160.0*x;
    IL = ImaxL;
  }
 
  analogWrite(ENAL,IL); //Magnitude of output power for the left wheels
  analogWrite(ENAR,IR); //Magnitude of output power for the right wheels
  delay(100);           //Keep it for 100ms

  Serial.print('L');  //Motor output monitering to the serial monitaring wind in the Arduino aplication.
  Serial.println(IL);
  Serial.print('R');
  Serial.println(IR);
 
}

void turning() {  // Turning to find the target
  Serial.print('T');  //Header 'T' is printed out to the serial monitaring wind

  int ItL = 150, ItR = 150 ;  //Speed of the left and right wheel

  digitalWrite(IN1L,LOW);  // Turn the left wheels to move backward (LOW and HIGH)
  digitalWrite(IN2L,HIGH);
  digitalWrite(IN1R,HIGH); // Turn the right wheels to move forward (HIGH and LOW)
  digitalWrite(IN2R,LOW);

  analogWrite(ENAL,ItL); //Power output
  analogWrite(ENAR,ItR);
  delay(100);            //Continue for 200ms

  digitalWrite(IN1L,LOW); //Stop the wheels
  digitalWrite(IN2L,LOW);
  digitalWrite(IN1R,LOW);
  digitalWrite(IN2R,LOW);
  delay(200);             //Keep stopping for 200ms
  
}

void stop() {  // Stop

  digitalWrite(IN1L,LOW);  
  digitalWrite(IN2L,LOW);
  digitalWrite(IN1R,LOW);
  digitalWrite(IN2R,LOW);
 
}
