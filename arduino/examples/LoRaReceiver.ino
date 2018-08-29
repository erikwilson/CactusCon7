#include <SPI.h>
#include <LoRa.h>
#include <SSD1306.h>

SSD1306 display(0x3c, 4, 15);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Receiver");
  LoRa.setPins(18, 14, 26);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa started...");

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Receiving...");
  display.display();
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String output = "Received packet '";
    // received a packet

    // read packet
    while (LoRa.available()) {
      output += (char)LoRa.read();
    }

    output += "' with RSSI ";
    output +=  LoRa.packetRssi();
    Serial.println(output);

    display.clear();
    display.drawStringMaxWidth(0, 0, 128, output);
    display.display();
  }
}
