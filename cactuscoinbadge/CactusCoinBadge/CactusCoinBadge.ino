#include <HTTPClient.h>
#include <SPIFFS.h>
#include <Ticker.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <SSD1306.h>
#include "mbedtls/pk.h"
#include "mbedtls/error.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/md.h"
#include "mbedtls/sha256.h"
#include "mbedtls/base64.h"

#include "config.h"
#include "credentials.h"
#include "CDP.h"

SSD1306 display(0x3c, 4, 15);
Ticker gbpTimer;
Ticker TXLogTimer;
int coinCounter = 0;
uint16_t myBadgeID = 0;
char myName[MAX_NAME_LENGTH] = "cybaix";
bool sendGBP = false;
bool TXLogFlush = false;
mbedtls_pk_context badgePK;
mbedtls_pk_context nodePK;
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;

void triggerGBP() {
  sendGBP = true;
}

void triggerPendingTXLogFlush() {
  TXLogFlush = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("CactusCoinBadge v1.0");

  if (!setupFS())
    return;

  if (!setupCrypto())
    return;
    
  registerBadge();

  WiFi.mode(WIFI_OFF);
  btStop();
  
  LoRa.setPins(18, 14, 26);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setTxPower(BADGE_TX_POWER);
  Serial.println("LoRa started...");

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();

  gbpTimer.attach(BROADCAST_TIME_SEC, triggerGBP);
  TXLogTimer.attach(TXLOG_FLUSH_SEC, triggerPendingTXLogFlush);
  
  updateDisplay();
}

void updateDisplay() {
  display.clear();
  String output = "Badge ID: ";
  output += myBadgeID;
  display.drawStringMaxWidth(0, 0, 128, output);
  output = "Name: ";
  output += myName;
  display.drawStringMaxWidth(0, 15, 128, output);
  output = "Coins: ";
  output += coinCounter;
  display.drawStringMaxWidth(0, 30, 128, output);
  display.display();
}

void processPacket() {
    int packetSize = LoRa.available();
    byte packet[packetSize];
    byte *packetPtr = &packet[1];
    String json;
    uint16_t broadcasterID;
    SignedCoin signedCoin;

    for (int i = 0; i < packetSize; i++)
      packet[i] = LoRa.read();

    // received a packet
    switch (packet[0]) {
      case CDP_GLOBALBROADCAST_TYPE:
        if (isValidGlobalBroadcast(packetPtr, packetSize))
          transmitCoinSigningRequest(myBadgeID, packetPtr, packetSize);
          
        break;
        
      case CDP_COINSIGNINGREQUEST_TYPE:
        if (isValidCoinSigningRequest(packetPtr, packetSize)) {
          transmitSignedCoin(myBadgeID, packetPtr, packetSize);
          coinCounter++;
        }
        //if (!generateSignedCoin(myBadgeID, packetPtr, packetSize, signedCoin))
        //  break;  
        //json = jsonifySignedCoin(&signedCoin);
        //Serial.println(json);
        //storeUnsentSignedCoinOnFS(signedCoin.csr.coin.broadcasterID, json);
        //transmitSignedCoin(myBadgeID, packetPtr, packetSize);
        //submitSignedCoinToAPI(json);
        break;
        
      case CDP_SIGNEDCOIN_TYPE:
        if (isValidSignedCoin(packetPtr, packetSize)) {
          json = jsonifySignedCoin(packetPtr, packetSize);
          broadcasterID = getBroadcasterIDFromBytes(packetPtr, packetSize);
          storeUnsentSignedCoinOnFS(broadcasterID, json);
          submitSignedCoinToAPI(json);
          coinCounter++;
        }
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

  if (TXLogFlush) {
    drainSignedCoinPendingTXLogOnFS();
    TXLogFlush = false;
  }

  updateDisplay();
}
