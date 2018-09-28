#include <SPI.h>
#include <LoRa.h>
#include <SSD1306.h>

SSD1306 display(0x3c, 4, 15);

int counter = 0;
int t4Max, t5Max, t8Max, t9Max;

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
  display.setBrightness(20);
  display.clear();

  touchSetCycles(0xA00, 0x2000);
  touchRead(T4);
  touchRead(T5);
  touchRead(T8);
  touchRead(T9);

  // delay(100);

  // t4Max = touchRead(T4);
  // t5Max = touchRead(T5);
  // t8Max = touchRead(T8);
  // t9Max = touchRead(T9);

  // Serial.print(" 4 Max:");
  // Serial.print(t4Max);
  // Serial.print(" 5 Max:");
  // Serial.print(t5Max);
  // Serial.print(" 8 Max:");
  // Serial.print(t8Max);
  // Serial.print(" 9 Max:");
  // Serial.print(t9Max);
  // Serial.println();
}

void loop() {
  const float Vb = (7.1f * analogRead(38)) / 2047.0;

  // String output = "v" + String(Vb) + "\n";
  // output += "Sending packet: ";
  String output = "";
  output += counter;
  output += ": ";

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  int t4 = touchRead(T4);
  int t5 = touchRead(T5);
  int t8 = touchRead(T8);
  int t9 = touchRead(T9);
  if (t4 > t4Max) { t4Max = t4; }
  if (t5 > t5Max) { t5Max = t5; }
  if (t8 > t8Max) { t8Max = t8; }
  if (t9 > t9Max) { t9Max = t9; }

  const float diff = 0.9;
  Serial.print(" 4:");
  Serial.print(t4);
  Serial.print(" 5:");
  Serial.print(t5);
  Serial.print(" 8:");
  Serial.print(t8);
  Serial.print(" 9:");
  Serial.print(t9);
  Serial.println();

  Serial.print(" 4:");
  Serial.print(t4Max*diff);
  Serial.print(" 5:");
  Serial.print(t5Max*diff);
  Serial.print(" 8:");
  Serial.print(t8Max*diff);
  Serial.print(" 9:");
  Serial.print(t9Max*diff);
  Serial.println();

  String button = "";
  int numButtons = 0;
  if (t8 < (t8Max*diff)  && t9 < (t9Max*diff)) {
    button += "A";
    numButtons++;
  }
  if (t4 < (t4Max*diff)  && t8 < (t8Max*diff)) {
    button += "B";
    numButtons++;
  }
  if (t4 < (t4Max*diff)  && t5 < (t5Max*diff)) {
    button += "Up";
    numButtons++;
  }
  if (t5 < (t5Max*diff)  && t9 < (t9Max*diff)) {
    button += "Down";
    numButtons++;
  }
  if (t5 < (t5Max*diff)  && t8 < (t8Max*diff)) {
    button += "Left";
    numButtons++;
  }
  if (t4 < (t4Max*diff)  && t9 < (t9Max*diff)) {
    button += "Right";
    numButtons++;
  }
  if (numButtons > 1) {
    button = "...";
  }
  output += button;
  Serial.println(output);
  
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, output);
  display.display();

  counter++;

  // delay(10);
}
