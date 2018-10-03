#include <SSD1306.h>
#include <math.h>
#include <algorithm>
#include <LoRa.h>

#include "motor-crypto.h"

uint8_t key[] = { 'h', 'e', 'l', 'l', 'o' };
MotorCrypto motorCrypto = MotorCrypto(key, sizeof(key));

#define SS      18
#define RST     14
#define DI0     26
#define BAND    433E6  //915E6 -- 这里的模式选择中，检查一下是否可在中国实用915这个频段

// Select button is triggered when joystick is pressed
const byte PIN_BUTTON_SELECT = 34;

const byte PIN_ANALOG_X = 37;
const byte PIN_ANALOG_Y = 36;

int avgX = 0;
int avgY = 0;
int deltaX = 64;
int deltaY = 64;

SSD1306 display(0x3c, 4, 15);

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON_SELECT, INPUT);

  const int numSamples = 8;
  for (int i=-1; i<numSamples; i++) {
    delay(50);
    int x = analogRead(PIN_ANALOG_X);
    int y = analogRead(PIN_ANALOG_Y);
    if (i<0) continue;

    avgX += float(x) / numSamples;
    avgY += float(y) / numSamples;
  }

  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  display.init();
  display.flipScreenVertically();

  LoRa.setPins(SS, RST, DI0);

  if (!LoRa.begin(BAND)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  LoRa.setSignalBandwidth(500e3);
  LoRa.setPreambleLength(8);
  LoRa.setSpreadingFactor(7);
  LoRa.setSyncWord(0x7B);           // ranges from 0-0xFF, default 0x34, see API docs
  // register the receive callback
  Serial.println("LoRa init succeeded.");
}


void loop() {
  const int AX = analogRead(PIN_ANALOG_X);
  const int AY = analogRead(PIN_ANALOG_Y);

  double x = 0;
  double y = 0;

  if (fabs(avgX-AX) > deltaX || fabs(avgY-AY) > deltaY) {
    if (AX < avgX) {
      x = -double(AX - avgX) / avgX;
    } else {
      x = -double(AX - avgX) / (4095 - avgX);
    }
    if (AY < avgY) {
      y = double(AY - avgY) / avgY;
    } else {
      y = double(AY - avgY) / (4095 - avgY);
    }
  }

  const double angleJ = atan2(y,x) * 180. / M_PI;

  const double r = std::max(fabs(x),fabs(y));
  const double newX = r * cos(angleJ * M_PI / 180);
  const double newY = r * sin(angleJ * M_PI / 180);
  const int16_t dispX = 32+24*newX;
  const int16_t dispY = 32-24*newY;

  const double angleM = r > 0 ? (angleJ < 45 ? 360. : 0.) + angleJ - 45 : 0;

  const double powerL = r * cos(angleM * M_PI / 180);
  const double powerR = r * sin(angleM * M_PI / 180);

  double scale = 1;
  double mx = 0;
  double my = 0;
  double angleMr = angleM * M_PI / 180;

  if (x<=0 && y>0 || x>=0 && y<0) {
    my = 1;
    mx = 1./tan(angleMr);
  }
  if (x>0 && y>=0 || x<0 && y<=0) {
    mx = 1;
    my = tan(angleMr);
  }
  scale = sqrt(pow(mx, 2) + pow(my, 2));

  const int motorBL = scale*fabs(powerL)*265;
  uint8_t motorL = 0;
  if (motorBL>255) motorL = 255;
  else if (motorBL>0) motorL = motorBL;

  const int motorBR = scale*fabs(powerR)*265;
  uint8_t motorR = 0;
  if (motorBR>255) motorR = 255;
  else if (motorBR>0) motorR = motorBR;

  uint8_t mode = 0;
  if (r > 0) {
    if (powerL<=0 && powerR<=0) mode = 4;
    if (powerL>=0 && powerR<=0) mode = 3;
    if (powerL<=0 && powerR>=0) mode = 2;
    if (powerL>=0 && powerR>=0) mode = 1;
  }

  const int DS = digitalRead(PIN_BUTTON_SELECT);

  display.clear();
  display.drawCircle(32,32,31);
  if (dispX != 32 || dispY != 32) display.drawCircle(dispX,dispY,5);
  display.setPixel(dispX,dispY);
  display.drawStringMaxWidth(72, 0, 128, String(mode));
  display.drawStringMaxWidth(85, 0, 128, String(motorL));
  display.drawStringMaxWidth(110, 0, 128, String(motorR));
  
  display.drawStringMaxWidth(72, 20, 128, String(DS));
  
  display.drawStringMaxWidth(72, 42, 128, String(motorCrypto.getCounter()));
  display.drawStringMaxWidth(60, 52, 128, String(motorCrypto.getConnectionId()));
  display.display();

  MotorEncrypted encrypted = motorCrypto.encrypt(mode,motorL,motorR);

  LoRa.beginPacket(80);
  LoRa.write(encrypted.data,sizeof(encrypted.data));
  LoRa.endPacket();
}
