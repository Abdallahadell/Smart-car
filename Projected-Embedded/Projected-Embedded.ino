#include <Arduino_FreeRTOS.h>

//mp3
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include "SoftwareSerial.h"
#include "Wire.h"

SoftwareSerial mySoftwareSerial(10, 11);

char cc; // variable incoming from communication with Arduino UNO
int musicNumber = 1;

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

int Xin = A5; // X Input Pin
int Yin = A4; // Y Input Pin
int KEYin = 7; // Push Button
int e = 53;
int d = 51;
int c = 49;
int b = 52;
int a = 48;
int f = 46;
int g = 44;

int gear = 4; // 4 = P, 3 = R, 2 = N, 1 = D

//IR
int IRvalueAR = 0;
int IRvalueAL = 0;
const int pinIRaL = A0;
const int pinIRaR = A10;
const int ledLeft = 32;
const int ledRight = 30;

// Motor A FRONT WHEEL LEFT
int enFR = 5;
#define ForwardFR 29
#define BackwardFR 31

// Motor B FRONT WHEEL RIGHT
#define enFL 4
#define ForwardFL 35
#define BackwardFL 33

// Motor C BACK WHEEL RIGHT
#define enBR 3
#define ForwardBR 37
#define BackwardBR 39

// Motor D BACK WHEEL LEFT
#define enBL 2
#define ForwardBL 43
#define BackwardBL 41

int ldr = A1; //Set A1(Analog Input) for LDR.
int value = 0;
int ledLDR = 8;

//FreeRTOS tasks reference
void vTaskGearBox(void *pvParameters); // priority = 2
void vTaskLaneKeepAssist(void *pvParameters); // priority = 2
void vTaskAdaptiveLights(void *pvParameters); // priority = 2

void setup()
{
  pinMode (KEYin, INPUT);

  pinMode (e, OUTPUT);
  pinMode (d, OUTPUT);
  pinMode (c, OUTPUT);
  pinMode (b, OUTPUT);
  pinMode (a, OUTPUT);
  pinMode (f, OUTPUT);
  pinMode (g, OUTPUT);

  //IR
  pinMode(pinIRaL, INPUT);
  pinMode(pinIRaR, INPUT);
  pinMode(ledLeft, OUTPUT);
  pinMode(ledRight, OUTPUT);
  //LDR
  pinMode(ledLDR, OUTPUT);
  //mp3
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    //        while(true);
  }
  Serial.println(F("DFPlayer Mini online."));


  //I2C
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Wire.onRequest(requestEvent);
  //setup motors
  //            Serial.println("1");
  pinMode(8, OUTPUT);
  pinMode(enFR, OUTPUT);
  pinMode(enFL, OUTPUT);
  pinMode(enBR, OUTPUT);
  pinMode(enBL, OUTPUT);
  pinMode(ForwardFR, OUTPUT);
  pinMode(BackwardFR, OUTPUT);
  pinMode(ForwardFL, OUTPUT);
  pinMode(BackwardFL, OUTPUT);
  pinMode(ForwardBR, OUTPUT);
  pinMode(BackwardBR, OUTPUT);
  pinMode(ForwardBL, OUTPUT);
  pinMode(BackwardBL, OUTPUT);
  //Motor A
  //            Serial.println("2");
  digitalWrite(enFR, LOW);
  digitalWrite(ForwardFR, HIGH);
  digitalWrite(BackwardFR, LOW);
  //            Serial.println("3");
  // Motor B
  digitalWrite(enFL, LOW);
  digitalWrite(ForwardFL, HIGH);
  digitalWrite(BackwardFL, LOW);
  // Motor C
  digitalWrite(enBR, LOW);
  digitalWrite(ForwardBR, HIGH);
  digitalWrite(BackwardBR, LOW);
  // Motor D
  digitalWrite(enBL, LOW);
  digitalWrite(ForwardBL, HIGH);
  digitalWrite(BackwardBL, LOW);

  myDFPlayer.volume(30);
  myDFPlayer.play(musicNumber);
  Serial.println("LKA");
  xTaskCreate(vTaskLaneKeepAssist, "Lane Keep Assistance", 500, NULL, 2, NULL);
  xTaskCreate(vTaskAdaptiveLights, "Adaptive Headlights", 500, NULL, 2, NULL);
  xTaskCreate(vTaskGearBox, "Gearbox", 500, NULL, 2, NULL);
  Serial.println("LKA");
//  cc = '1';
  vTaskStartScheduler();
}

void loop() {}

void vTaskLaneKeepAssist(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1) {
    Serial.println("LKA");

    if (left() && right()) {
      // Serial.println("S");
      Stop();
    } else if (left() && !right()) {
      // Serial.println("L");
      adjustLeft();
    } else if (right() && !left()) {
      // Serial.println("R");
      adjustRight();
    } else {
      // Serial.println("F");
      forward();
    }
    vTaskDelayUntil(xLastWakeTime, pdMS_TO_TICKS(10));
  }
}

void vTaskAdaptiveLights(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    Serial.println("LIGHT");
    value = analogRead(ldr); //Reads the Value of LDR(light).
    //    Serial.println(value);
    if (value < 100) {
      analogWrite(ledLDR, 0);
      //       Serial.println("1");
    } else if (value > 100 && value < 200) {
      analogWrite(ledLDR, 59);
      //       Serial.println("2");
    } else if (value > 200 && value < 300) {
      analogWrite(ledLDR, 155);
      //       Serial.println("3");
    } else {
      analogWrite(ledLDR, 255);
      //       Serial.println("4");
    }
    vTaskDelayUntil(xLastWakeTime, pdMS_TO_TICKS(1000));
  }
}
int yVal;
void vTaskGearBox(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    int xVal, yVal, buttonVal;

    xVal = analogRead (Xin);
    yVal = analogRead (Yin);
    buttonVal = digitalRead (KEYin);

    if (yVal > 600) {
      gear++;
      gear = gear > 4 ? 4 : gear;
    }

    if (yVal < 400) {
      gear--;
      gear = gear < 1 ? 1 : gear;
    }

    drawGear();

    //  Serial.println (yVal, DEC);

    delay (500);
    vTaskDelayUntil(xLastWakeTime, pdMS_TO_TICKS(250));
  }

}

void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void forward() {
  digitalWrite(ForwardFR, HIGH);
  digitalWrite(BackwardFR, LOW);
  digitalWrite(ForwardFL, HIGH);
  digitalWrite(BackwardFL, LOW);
  digitalWrite(ForwardBR, HIGH);
  digitalWrite(BackwardBR, LOW);
  digitalWrite(ForwardBL, HIGH);
  digitalWrite(BackwardBL, LOW);
  analogWrite(enFR, 80);
  analogWrite(enFL, 80);
  analogWrite(enBR, 80);
  analogWrite(enBL, 80);
  digitalWrite(ledLeft, LOW);
  digitalWrite(ledRight, LOW);
}


void receiveEvent(int howMany) {
  while (0 < Wire.available()) {
    cc = Wire.read();      /* receive byte as a character */
  }
  if (cc == '1') {
      Serial.println("Play");
      myDFPlayer.play(musicNumber);
      cc = '0';
    }
    else if (cc == '2') {
      Serial.println("Next");
      if (musicNumber < 5) {
        musicNumber++;
      } else {
        musicNumber = 1;
      }
      myDFPlayer.next();
      cc = '0';
    }
    else if (cc == '3') {
      Serial.println("Pause");
      myDFPlayer.pause();
      cc = '0';
    }
    else if (cc == '4') {
      Serial.println("Previous");
      if (musicNumber > 1){
        musicNumber--;
      
      myDFPlayer.previous();
      }
      cc = '0';
    }
  Serial.println("Recived");
  delay(100);/* to newline */
}
void requestEvent() {
  char snum[5];
  sprintf(snum, "%d", musicNumber);
  Wire.write(musicNumber);
  delay(100);/*send string on request */
}

bool left() {
  IRvalueAL = analogRead(pinIRaL);
  //  Serial.println(IRvalueAL);
  if (IRvalueAL > 680) {
    return true;
  }
  else {
    return false;
  }
}

bool right() {
  IRvalueAR = analogRead(pinIRaR);
  //  Serial.println(IRvalueAR);

  if (IRvalueAR > 700 ) {
    return true;
  }
  else {
    return false;
  }
}

void adjustLeft() {
  digitalWrite(ForwardFR, HIGH);
  digitalWrite(BackwardFR, LOW);
  digitalWrite(ForwardFL, LOW);
  digitalWrite(BackwardFL, HIGH);
  digitalWrite(ForwardBR, LOW);
  digitalWrite(BackwardBR, HIGH);
  digitalWrite(ForwardBL, HIGH);
  digitalWrite(BackwardBL, LOW);
  analogWrite(enFR, 150);
  analogWrite(enFL, 150);
  analogWrite(enBR, 150);
  analogWrite(enBL, 150);
  digitalWrite(ledLeft, HIGH);
  digitalWrite(ledRight, LOW);
}

void adjustRight() {
  digitalWrite(ForwardFR, LOW);
  digitalWrite(BackwardFR, HIGH);
  digitalWrite(ForwardFL, HIGH);
  digitalWrite(BackwardFL, LOW);
  digitalWrite(ForwardBR, HIGH);
  digitalWrite(BackwardBR, LOW);
  digitalWrite(ForwardBL, LOW);
  digitalWrite(BackwardBL, HIGH);
  analogWrite(enFR, 150);
  analogWrite(enFL, 150);
  analogWrite(enBR, 150);
  analogWrite(enBL, 150);
  digitalWrite(ledLeft, LOW);
  digitalWrite(ledRight, HIGH);
}
void Stop() {
  analogWrite(enFR, 0);
  analogWrite(enFL, 0);
  analogWrite(enBR, 0);
  analogWrite(enBL, 0);
  digitalWrite(ledLeft, HIGH);
  digitalWrite(ledRight, HIGH);
}

void drawGear() {

  switch (gear) {
    case 1 :
      turnOffBCD();
      digitalWrite(e, HIGH);
      digitalWrite(d, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(g, HIGH);
      break;
    case 2 :
      turnOffBCD();
      digitalWrite(e, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(b, HIGH);
      digitalWrite(a, HIGH);
      digitalWrite(f, HIGH);
      break;
    case 3 :
      turnOffBCD();
      digitalWrite(e, HIGH);
      digitalWrite(a, HIGH);
      digitalWrite(f, HIGH);
      break;
    case 4 :
      turnOffBCD();
      digitalWrite(e, HIGH); //
      digitalWrite(g, HIGH); //
      digitalWrite(b, HIGH); //
      digitalWrite(a, HIGH); //
      digitalWrite(f, HIGH); //
      break;
  }

}

void turnOffBCD() {
  digitalWrite(d, LOW);
  digitalWrite(c, LOW);
  digitalWrite(e, LOW);
  digitalWrite(b, LOW);
  digitalWrite(a, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);

}