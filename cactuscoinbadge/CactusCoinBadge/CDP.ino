#include <ArduinoJson.h>

typedef struct GlobalBroadcast {
  uint16_t badgeID;  
};

typedef struct Coin {
  uint16_t CSRID;
  uint16_t broadcasterID;
};

typedef struct CoinSigningRequest {
  Coin coin;
  byte signatureCSR[CDP_MODULUS_SIZE];
};

typedef struct SignedCoin {
  CoinSigningRequest csr;
  byte signatureBroadcaster[CDP_MODULUS_SIZE];
};

int submitCoin(uint16_t myBadgeID, byte *scnPtr, int packetSize) {
  SignedCoin *scn;
  if ((packetSize - 1) != sizeof(SignedCoin)) {  // subtract 1 to account for type byte
    Serial.print("SignedCoin was an invalid length of ");
    Serial.println(packetSize);
    return -1;
  }

  scn = (SignedCoin *)scnPtr;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASSWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["CSRID"] = scn->csr.coin.CSRID;
  root["broadcasterID"] = scn->csr.coin.broadcasterID;
  root["signatureCSR"] = scn->csr.signatureCSR;
  root["signatureBroadcaster"] = scn->signatureBroadcaster;
  root.printTo(Serial);
  Serial.println();

  /*
  HTTPClient http;
  http.begin("http://www.mywebpage.com/php_page.php");
  http.addHeader("Content-Type", "application/json");
  http.POST(root.);*/
  WiFi.mode(WIFI_OFF);
}

int transmitSignedCoin(uint16_t myBadgeID, byte *csrPtr, int packetSize) {
  CoinSigningRequest *csr;
  
  if ((packetSize - 1) != sizeof(CoinSigningRequest)) {  // subtract 1 to account for type byte
    Serial.print("CoinSigningRequest was an invalid length of ");
    Serial.println(packetSize);
    return -1;
  }
  
  csr = (CoinSigningRequest *)csrPtr;
  
  if (csr->coin.broadcasterID != myBadgeID) {
    Serial.print("Ignoring CSR, it was intended for badge #");
    Serial.print(csr->coin.broadcasterID);
    Serial.print(" and my badge is #");
    Serial.println(csr->coin.broadcasterID);
    return -1;
  }
  
  SignedCoin signedCoin;
  signedCoin.csr = *csr;
  signedCoin.signatureBroadcaster[0] = 0x02;
  
  Serial.print("Signing CSR for Badge #");
  Serial.print(csr->coin.CSRID);
  Serial.print(" and my badge is #");
  Serial.println(myBadgeID);
  LoRa.beginPacket();
  LoRa.write(CDP_SIGNEDCOIN_TYPE);
  LoRa.write((byte *)&signedCoin, sizeof(signedCoin));
  LoRa.endPacket();
}

int transmitCoinSigningRequest(uint16_t myBadgeID, byte *gbpPtr, int packetSize) {
  GlobalBroadcast *gbp;
  CoinSigningRequest csr;
  Coin coin;
  int packetRssi = LoRa.packetRssi();
  
  if (packetRssi < CSR_RSSI_THRESHOLD) {
      Serial.print("Ignoring broadcast RSSI (");
      Serial.print(packetRssi);
      Serial.println(") below threshold");
      return -1;
  }
  
  if ((packetSize - 1) != sizeof(GlobalBroadcast)) {  // subtract 1 to account for type byte
    Serial.print("GlobalBroadcast was an invalid length of ");
    Serial.println(packetSize);
    return -1;
  }
    
  gbp = (GlobalBroadcast *)gbpPtr;
  coin.CSRID = myBadgeID;
  coin.broadcasterID = gbp->badgeID;
  csr.coin = coin;
  csr.signatureCSR[0] = 0x01;
  // csr->signatureCSR = we'll just leak some random bits for now
  
  Serial.print("Generating CSR for Badge #");
  Serial.print(gbp->badgeID);
  Serial.print(" my badge #");
  Serial.println(csr.coin.CSRID);
  LoRa.beginPacket();
  LoRa.write(CDP_COINSIGNINGREQUEST_TYPE);
  LoRa.write((byte *)&csr, sizeof(csr));
  LoRa.endPacket();
}

void transmitGlobalBroadcast(uint16_t myBadgeID) {
  GlobalBroadcast gbp;
  gbp.badgeID = myBadgeID;
  
  Serial.println("Transmitting broadcast");
  LoRa.beginPacket();
  LoRa.write(CDP_GLOBALBROADCAST_TYPE);
  LoRa.write((byte *)&gbp, sizeof(gbp));
  LoRa.endPacket();
}



