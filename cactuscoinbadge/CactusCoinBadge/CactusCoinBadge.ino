#include <Ticker.h>

#include <SPI.h>
#include <LoRa.h>
#include <SSD1306.h>

#include "config.h"

SSD1306 display(0x3c, 4, 15);
Ticker gbpTimer;
int counter = 0;
uint16_t myBadgeID = 1;
bool sendGBP = false;

void triggerGBP() {
  sendGBP = true;
}

void setup() {
  // Turn on wifi and check if badge is registered with API
  // if it isn't, call registerBadge (gets name from user and registers)
  // if it is, get the list of coined badges.
  Serial.begin(115200);
  while (!Serial);

  Serial.println("CactusCoinBadge v1.0");

  registerBadge();
  
  LoRa.setPins(18, 14, 26);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setTxPower(1);
  Serial.println("LoRa started...");

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

  pinMode(38, INPUT);
  analogSetAttenuation(ADC_11db);
  analogReadResolution(11);

  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();

  gbpTimer.attach(BROADCAST_TIME_SEC, triggerGBP);
}

void processPacket() {
    // read packet
    int packetSize = LoRa.available();
    int packetRssi = LoRa.packetRssi();
    byte packet[packetSize];
    byte *packetPtr = &packet[1];

    for (int i = 0; i < packetSize; i++)
      packet[i] = LoRa.read();

    // received a packet
    switch (packet[0]) {
      case CDP_GLOBALBROADCAST_TYPE:
        transmitCoinSigningRequest(myBadgeID, packetPtr, packetSize);
        break;
      case CDP_COINSIGNINGREQUEST_TYPE:
        transmitSignedCoin(myBadgeID, packetPtr, packetSize);
        //cdp.createSignedCoin(cdp.getMessage());
        break;
      case CDP_SIGNEDCOIN_TYPE:
        // turn on wifi, report, turn off wifi
        //reportToCactuscoinNode(cdp.getMessage());
        // submit multiple coins
        break;
    }
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    processPacket();
  }

  if (sendGBP) {
    transmitGlobalBroadcast(myBadgeID);
    sendGBP = false;
  }
  
}
