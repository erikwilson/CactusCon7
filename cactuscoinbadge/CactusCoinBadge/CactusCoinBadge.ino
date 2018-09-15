#include <Ticker.h>

#include <SPI.h>
#include <LoRa.h>
#include <SSD1306.h>

#define GBP_BROADCAST_TIME_MS 1000

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

void setup() {
  // Turn on wifi and check if badge is registered with API
  // if it isn't, call registerBadge (gets name from user and registers)
  // if it is, get the list of coined badges.
  Serial.begin(115200);
  while (!Serial);

  Serial.println("CactusCoinBadge v1.0");
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
    String output = "Received packet '";

    // read packet
    while (LoRa.available()) {
      output += (char)LoRa.read();
    }

    // received a packet
    //cdp.decodePacket(output);
    //cdp.
    switch (cdp.getType()) {
      case CDP::GBP:
        
        break;
      case CDP::CSR:
        break

      case CDP::CAD:
        // turn on wifi, report, turn off wifi
        break
    }
    
    output += "' with RSSI ";
    output +=  LoRa.packetRssi();
    Serial.println(output);
    
    display.clear();
    display.drawStringMaxWidth(0, 0, 128, output);
    display.display();

  // read entire packet
  // is the packet a GBP? if we haven't paired before and RSSI < 50, send a CSR
  // is packet a CSR? if so, respond with CAD
  // all other, log and return
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    processPacket();
  }

  if (sendGBP)
    transmitGBP();
  
}
