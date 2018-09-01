#include <SPI.h>
#include <LoRa.h>
#include <SSD1306.h>

SSD1306 display(0x3c, 4, 15);

int counter = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender");
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

  pinMode(38, INPUT);
  analogSetAttenuation(ADC_11db);
  analogReadResolution(11);

  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();
}

void loop() {
  const float Vb = (7.1f * analogRead(38)) / 2047.0;

  String output = "v" + String(Vb) + "\n";
  output += "Sending packet: ";
  output += counter;
  Serial.println(output);

  display.clear();
  display.drawStringMaxWidth(0, 0, 128, output);
  display.display();

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  Serial.print(" 4:");
  Serial.print(touchRead(T4));
  Serial.print(" 5:");
  Serial.print(touchRead(T5));
  Serial.print(" 7:");
  Serial.print(touchRead(T7));
  Serial.print(" 8:");
  Serial.print(touchRead(T8));
  Serial.print(" 9:");
  Serial.print(touchRead(T9));
  Serial.println();

  counter++;

  delay(1000);
}
