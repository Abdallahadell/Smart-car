#include <stdint.h>
//#include "TouchScreen.h"

const int XP=9,XM=A3,YP=A2,YM=8; //240x320 ID=0x7789
const int TS_LEFT=119,TS_RT=911,TS_TOP=111,TS_BOT=918;

// PORTRAIT CALIBRATION 240 x 320

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// LANDSCAPE CALIBRATION 320 x 240
// int x = map(p.y, TS_LEFT, TS_RT, 0, 320);
// int y = map(p.x, TS_TOP, TS_BOT, 0, 240);


void setup(void) {
 Serial.begin(9600);
}

void loop(void) {
   TSPoint p = ts.getPoint();
  int x = map(p.x, TS_LEFT, TS_RT, 0, 240);
  int y = map(p.y, TS_TOP, TS_BOT, 0, 320);
 if (p.z > ts.pressureThreshhold) {
    Serial.print("X = "); Serial.print(x);
    Serial.print("\tY = "); Serial.print(y);
    Serial.print("\tPressure = "); Serial.println(p.z);
 }
 delay(100);
}
