#include <Ticker.h>

#include <SPI.h>
#include <LoRa.h>
#include <SSD1306.h>

#define GBP_BROADCAST_TIME_MS 1000
#define GBP_RSSI_THRESHOLD -60

#define CDP_VERSION 1
#define RUP_TYPE_NUMBER 1
#define GBP_TYPE_NUMBER 2

SSD1306 display(0x3c, 4, 15);
Ticker gbpTimer;
int counter = 0;
int myBadgeID = 1;
bool sendGBP = false;

void triggerGBP() {
  sendGBP = true;
}

void transmitGBP() {
  byte lowID = myBadgeID;
  byte highID = myBadgeID >> 8;
  byte packet[4] = {CDP_VERSION, highID, lowID, GBP_TYPE_NUMBER};
  LoRa.beginPacket();
  //LoRa.write(packet, sizeof(packet));
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket(true);
  Serial.println("Sent GPB");
  counter += 1;
  sendGBP = false;
}


/*
 * Obtains the name from the user using the dpad if this badge has never been registered, 
 * then registers the supplied name for this badge on a cactuscoin node over wifi.
 */
void registerBadge(){
  // turn on wifi
  // hit badge endpoint on cactuscoin node to determine if this badge already has a registered name with it (if so bail and/or give user option to update name).
  // ask the user for a name and register with the cactuscoin node.
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

  gbpTimer.attach(30, triggerGBP);
}

void processPacket() {
    // read packet
    int packetSize = LoRa.available();
    byte packet[packetSize];

    for (int i = 0; i < packetSize; i++)
      packet[i] = LoRa.read();

    // received a packet
    cdp.decodePacket(&packet);
    switch (cdp.getType()) {
      case CDP::GBP:
        if (LoRa.packetRssi() < GBP_RSSI_THRESHOLD)
          break;
        LoRa.beginPacket();
        LoRa.write(cdp.createCoinSigningRequest(&packet));
        LoRa.endPacket();
        break;
      case CDP::CSR:
        LoRa.beginPacket();
        LoRa.write(cdp.signCoinSigningRequest(cdp.getMessage()));
        LoRa.endPacket();
        break;
      case CDP::CAD:
        // turn on wifi, report, turn off wifi
        reportToCactuscoinNode(cdp.getMessage());
        break;
    }
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    processPacket();
  }

  if (sendGBP) {
    cdp.setType(CDP::GBP);
    cdp.setBadgeID(myBadgeID);
  }
  
}
