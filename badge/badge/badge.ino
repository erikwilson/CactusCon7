#include <HTTPClient.h>
#include <SPIFFS.h>
#include <Ticker.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <SSD1306.h>
#include "Font.h"
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
char myName[MAX_NAME_LENGTH];
bool sendGBP = false;
bool TXLogFlush = false;
bool catastrophicFailure = false; 
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
  Serial.println(F("CactusCoinBadge v1.1"));
  btStop();

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

  touchSetCycles(0xA00, 0x2000);
  setupPrintable();

  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setBrightness(20);

  display.clear();
  display.setFont(Roboto_Light_15);
  display.drawStringMaxWidth(0, 24, 128, "Connecting...");
  display.display();

  if (!setupFS()) {
    catastrophicFailure = true;
    return;
  }

  if (!setupCrypto()) {
    catastrophicFailure = true;
    return;
  }
  drainSignedCoinPendingTXLogOnFS();
  turnWiFiOnAndConnect();

  if (!registerBadge()) {
    turnWiFiOff();
    catastrophicFailure = true;
    return;
  }
  
  display.clear();
  display.setFont(Roboto_Light_15);
  display.drawStringMaxWidth(0, 12, 128, "Reticulating Splines...");
  display.setFont(Roboto_Light_11);
  display.display();

  drainSignedCoinPendingTXLogOnFS();
  refreshLocalCoinListFromAPI();

  turnWiFiOff();
  
  LoRa.setPins(18, 14, 26);
  if (!LoRa.begin(433E6)) {
    Serial.println(F("Starting LoRa failed!"));
    while (1);
  }
  LoRa.setTxPower(BADGE_TX_POWER);
  Serial.println(F("LoRa started..."));

  gbpTimer.attach(random(60, 120), triggerGBP);
  TXLogTimer.attach(TXLOG_FLUSH_SEC, triggerPendingTXLogFlush);

  updateDisplay();
}

void updateDisplay() {
  char badgeIDMessage[20], nameMessage[MAX_NAME_LENGTH + 6], coinMessage[13];
  if (catastrophicFailure) {
    display.setFont(ArialMT_Plain_10);
    display.clear();
    display.drawStringMaxWidth(0, 0, 128, F("Problem bootstrapping badge.  Get closer to the WiFi and press the restart button (RST) next to the USB port."));
    display.display();
    return;
  }
  
  sprintf(badgeIDMessage, "Badge ID: %d", myBadgeID);
  sprintf(nameMessage, "Name: %s", myName);
  sprintf(coinMessage, "Coins: %d", coinCounter);
  
  display.clear();
  display.setFont(Roboto_Light_15);
  display.drawString(0, 0, badgeIDMessage);
  display.drawString(0, 21, nameMessage);
  display.drawString(0, 42, coinMessage);
  display.display();
}

void processPacket() {
    int packetSize = LoRa.available();
    byte packet[packetSize];
    byte *packetPtr = &packet[1];
    char json[MAX_JSON_SIZE];
    uint16_t broadcasterID;
    SignedCoin signedCoin;

    for (int i = 0; i < packetSize; i++)
      packet[i] = LoRa.read();
      
    //Serial.print(F("LORA: Got a packet of type "));
    //Serial.printf("%02X\r\r\n", packet[0]);
    
    switch (packet[0]) {
      case CDP_GLOBALBROADCAST_TYPE:
        if (isValidGlobalBroadcast(packetPtr, packetSize))
          transmitCoinSigningRequest(myBadgeID, packetPtr, packetSize);
          
        break;
        
      case CDP_COINSIGNINGREQUEST_TYPE:
        if (isValidCoinSigningRequest(packetPtr, packetSize)) {
          transmitSignedCoin(myBadgeID, packetPtr, packetSize);
        }
        break;
        
      case CDP_SIGNEDCOIN_TYPE:
        if (isValidSignedCoin(packetPtr, packetSize)) {
          jsonifySignedCoin(packetPtr, packetSize, json, MAX_JSON_SIZE);
          broadcasterID = getBroadcasterIDFromBytes(packetPtr, packetSize);

          turnWiFiOnAndConnect();
          if (!submitSignedCoinToAPI(json))
            storeUnsentSignedCoinOnFS(broadcasterID, json);
          else
            storeCompletedCoinOnFS(broadcasterID);
          turnWiFiOff();
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
    turnWiFiOnAndConnect();
    drainSignedCoinPendingTXLogOnFS();  // always drain coins before doing an FS update, otherwise problems can abound
    refreshLocalCoinListFromAPI();
    turnWiFiOff();
    TXLogFlush = false;
  }

  updateDisplay();
}
