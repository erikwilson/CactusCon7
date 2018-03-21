#include <Adafruit_NeoPixel.h>

#include <LoRa.h>
#include <Arduino.h>
#include <SSD1306.h>

#include <math.h>
#include <algorithm>

// #define AUTO_PIXEL_PIN 13
// #define CONTROL_PIXEL_PIN 12
//
// Adafruit_NeoPixel autostrip = Adafruit_NeoPixel(3, AUTO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
// Adafruit_NeoPixel controlstrip = Adafruit_NeoPixel(3, CONTROL_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <DNSServer.h>

const byte DNS_PORT = 53;

unsigned long uptime = 0;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

DNSServer dnsServer;

#include "motor-crypto.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERIAL_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"

#define LORA_CHAR_UUID      "00bada55-dead-1337-beef-cac75c07b075"
#define MOVE_CHAR_UUID      "01bada55-dead-1337-beef-cac75c07b075"

char macString[] = "abcd";

char spaces[] = "                                                                                                                                ";

// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)

#define SS      18
#define RST     14
#define DI0     26
#define BAND    470E6  //915E6 -- 这里的模式选择中，检查一下是否可在中国实用915这个频段

#define PWMA_CH 1
#define PWMA    25
#define AI2     33
#define AI1     32
#define STDBY   17
#define BI1     12
#define BI2     13
#define PWMB    21
#define PWMB_CH 2

#define LEDC_TIMER_13_BIT  13
#define LEDC_BASE_FREQ     5000

SSD1306  display(0x3c, 4, 15);

byte mac[6];
int counter = 0;
int fontSize = 10;
int drawX = 0;
int drawY = 0;

int tick = 0;

volatile char message[255];
volatile bool messageIdle = true;

BLECharacteristic *pSerialChar;

void writeOled(String message) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  if (fontSize == 10) {
    display.setFont(ArialMT_Plain_10);
  }
  else if (fontSize == 16) {
    display.setFont(ArialMT_Plain_16);
  }
  else if (fontSize == 24) {
    display.setFont(ArialMT_Plain_24);
  }

  display.drawStringMaxWidth(drawX, drawY, 128, message);
  display.display();
}

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * std::min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

class LoraCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    int len = value.length();
    if (value.length() > 0 && messageIdle) {
      for (int i = 0; i < len; i++) {
        message[i] = value[i];
      }
      message[len] = '\0';
      messageIdle = false;
    }
  }
};

void move(uint8_t mode, uint8_t motorL, uint8_t motorR) {
  uptime = millis();

  if (mode == 0) {
    digitalWrite(STDBY, LOW);
  } else {
    digitalWrite(STDBY, HIGH);
  }
  if (mode == 1) {
    digitalWrite(AI1, HIGH);
    digitalWrite(AI2, LOW);
    digitalWrite(BI1, HIGH);
    digitalWrite(BI2, LOW);
  }
  if (mode == 2) {
    digitalWrite(AI1, LOW);
    digitalWrite(AI2, HIGH);
    digitalWrite(BI1, HIGH);
    digitalWrite(BI2, LOW);
  }
  if (mode == 3) {
    digitalWrite(AI1, HIGH);
    digitalWrite(AI2, LOW);
    digitalWrite(BI1, LOW);
    digitalWrite(BI2, HIGH);
  }
  if (mode == 4) {
    digitalWrite(AI1, LOW);
    digitalWrite(AI2, HIGH);
    digitalWrite(BI1, LOW);
    digitalWrite(BI2, HIGH);
  }
  ledcAnalogWrite(PWMA_CH, motorL);
  ledcAnalogWrite(PWMB_CH, motorR);
}


uint8_t key[] = { 'h', 'e', 'l', 'l', 'o' };
MotorCrypto motorCrypto = MotorCrypto(key, sizeof(key));

void onReceive(int packetSize) {
  // Serial.println("Got LoRa DATA!");
  uint8_t data[packetSize] = {};
  // read packet
  for (int i = 0; i < packetSize; i++) {
    data[i] = LoRa.read();
  }

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  if (packetSize == 80) {
    MotorEncrypted &encrypted = *(MotorEncrypted*)&data;
    if (motorCrypto.verifyHmac(encrypted)) {
      MotorMessage message = motorCrypto.decrypt(encrypted);
      if (motorCrypto.validateMessage(message)) {
        display.drawStringMaxWidth(0, 0, 128, String(message.mode));
        display.drawStringMaxWidth(20, 0, 128, String(message.motorL));
        display.drawStringMaxWidth(60, 0, 128, String(message.motorR));

        move(message.mode,message.motorL,message.motorR);
      } else {
        display.drawStringMaxWidth(0, 0, 128, "validate fail");
      }
    } else {
      display.drawStringMaxWidth(0, 0, 128, "HMAC fail");
    }
  }

  display.drawStringMaxWidth(0, 18, 128, String(LoRa.packetRssi()));
  display.drawStringMaxWidth(50, 18, 128, String(packetSize));
  display.drawStringMaxWidth(80, 18, 128, String(LoRa.packetSnr()));

  display.drawStringMaxWidth(0, 42, 128, String(motorCrypto.getCounter()));
  display.drawStringMaxWidth(0, 52, 128, String(motorCrypto.getConnectionId()));

  display.display();
}

class MoveCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      Serial.println("got MoveCallbacks packet!");
      if (value.length() != 3) {
        Serial.println("Invalid value length");
        return;
      }
      move(value[0],value[1],value[2]);
    }
};

class MySecurity : public BLESecurityCallbacks {

  uint32_t onPassKeyRequest(){
    Serial.println("PassKeyRequest");
    return 123456;
  }
  void onPassKeyNotify(uint32_t pass_key){
    Serial.print("On passkey Notify number:");
    Serial.println(pass_key);
    if (pass_key > 999999) {
      Serial.println("key bigger than 6 digits!");
      writeOled("passkey too large!");
      return;
    }
    char buf[7] = {0,0,0,0,0,0,0};
    snprintf(buf, 7, "%06d", pass_key);
    writeOled("BLE passkey: " + String(buf));
  }
  bool onConfirmPIN(uint32_t){
    Serial.println("onConfirmPIN");
    return true;
  }
  bool onSecurityRequest(){
    Serial.println("On Security Request");
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl){
    Serial.println("Starting BLE work!");
    if(cmpl.success){
      writeOled("BLE connected!");
      uint16_t length;
      esp_ble_gap_get_whitelist_size(&length);
      Serial.print("size:");
      Serial.println(length);
    }
  }
};

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  //Handle WebSocket event
  uint16_t *posData = (uint16_t*)data;
  if (len != 8) return;

  int maxX = posData[2];
  int maxY = posData[3];
  int avgX = maxX/2;
  int avgY = maxY/2;
  int x = posData[0];
  int y = maxY - posData[1];

  uint8_t *motor = getMotorMove(x,y,avgX,avgY,0,0,maxX,maxY);
  move(motor[0],motor[1],motor[2]);
}

uint8_t* getMotorMove(
  int AX,
  int AY,
  int avgX,
  int avgY,
  int deltaX,
  int deltaY,
  int maxX,
  int maxY
) {

  static uint8_t result[3];

  double x = 0;
  double y = 0;

  if (fabs(avgX-AX) > deltaX || fabs(avgY-AY) > deltaY) {
    if (AX < avgX) {
      x = double(AX - avgX) / avgX;
    } else {
      x = double(AX - avgX) / (maxX - avgX);
    }
    if (AY < avgY) {
      y = double(AY - avgY) / avgY;
    } else {
      y = double(AY - avgY) / (maxY - avgY);
    }
  }

  const double angleJ = atan2(y,x) * 180. / M_PI;

  const double r = std::max(fabs(x),fabs(y));
  const double newX = r * cos(angleJ * M_PI / 180);
  const double newY = r * sin(angleJ * M_PI / 180);

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

  result[0] = mode;
  result[1] = motorL;
  result[2] = motorR;

  return result;
}

void setupBle() {

  BLEDevice::init(macString);
  BLEDevice::setPower(ESP_PWR_LVL_P7);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P7);

  BLEServer *pServer = BLEDevice::createServer();

  // BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_NO_MITM);
  // BLEDevice::setSecurityCallbacks(new MySecurity());

  BLEService *serialService = pServer->createService(SERIAL_SERVICE_UUID);

  pSerialChar = serialService->createCharacteristic(
    LORA_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ      |
    BLECharacteristic::PROPERTY_WRITE     |
    BLECharacteristic::PROPERTY_NOTIFY    |
    BLECharacteristic::PROPERTY_INDICATE  |
    BLECharacteristic::PROPERTY_BROADCAST |
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  pSerialChar->setCallbacks(new LoraCallbacks());

  BLECharacteristic *iotdfRGBChar = serialService->createCharacteristic(
    MOVE_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  iotdfRGBChar->setCallbacks(new MoveCallbacks());

  BLE2902* p2902Descriptor = new BLE2902();
  p2902Descriptor->setNotifications(true);
  p2902Descriptor->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);

  pSerialChar->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  pSerialChar->addDescriptor(p2902Descriptor);

  // BLESecurity *pSecurity = new BLESecurity();
  // pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
  // pSecurity->setCapability(ESP_IO_CAP_OUT);
  // pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  //  pSerialChar->setValue("Hello World");
  // Start the service
  pServer->getAdvertising()->addServiceUUID(SERIAL_SERVICE_UUID);
  serialService->start();

  // Start advertising
  pServer->getAdvertising()->start();

}

void setupLora() {
  // SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);
  //  Serial.println("LoRa Sender");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSignalBandwidth(500e3);
  LoRa.setPreambleLength(8);
  LoRa.setSpreadingFactor(7);
  LoRa.setSyncWord(0x7B);           // ranges from 0-0xFF, default 0x34, see API docs
  // register the receive callback
  // LoRa.onReceive(onReceive);
  // // put the radio into receive mode
  // LoRa.receive();

  Serial.println("LoRa Initial OK!");
}

void setupWifi() {
  const char *ssid = "CCB07";
  const char *password = "12345678";
  const char *hostname = "bot";

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", myIP);

  SPIFFS.begin();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.serveStatic("/", SPIFFS, "/www/");
  server.onNotFound(redirect);

  server.begin();
}

void setup() {
  pinMode(25, OUTPUT); //Send success, LED will bright 1 second

  pinMode(AI1, OUTPUT);
  pinMode(AI2, OUTPUT);
  pinMode(BI1, OUTPUT);
  pinMode(BI2, OUTPUT);
  pinMode(STDBY, OUTPUT);

  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);

  ledcSetup(PWMA_CH, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(PWMA, PWMA_CH);

  ledcSetup(PWMB_CH, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(PWMB, PWMB_CH);

  message[0] = '\0';

  Serial.begin(115200);
  while (!Serial); //If just the the basic function, must connect to a computer

  uint8_t chipid[6];
  esp_efuse_read_mac(chipid);
  sprintf(macString, "%02x%02x", chipid[4], chipid[5]);
  Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);

  setupWifi();

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  display.init();
  display.flipScreenVertically();

  writeOled("chirp chrip !");

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);

  display.drawStringMaxWidth(0, 0, 128, "me: " + String(macString));
  display.drawStringMaxWidth(0, 18, 128, "chirp chrip!");

  display.display();

  // autostrip.begin();
  // controlstrip.begin();

  // LoRa.beginPacket();
  // LoRa.print(macString);
  // LoRa.print("|chirp!");
  // LoRa.endPacket();
}

void redirect(AsyncWebServerRequest *request){
  //Handle Unknown Request
  request->redirect("http://robutts/");
}

void parsePacket() {
  int packetSize = LoRa.parsePacket(80);
  if (packetSize > 0) {
    onReceive(packetSize);
  }
  if ((millis() - uptime) > 1000) {
    move(0,0,0);
  }
}

void loop() {
  dnsServer.processNextRequest();

  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "mem: " + String(ESP.getFreeHeap()));
  display.display();

  // delay(200);
  // display.clear();
  // float VBAT1 = analogRead(32) * (7.2 / 4095.0);
  // display.drawStringMaxWidth(0, 0, 128, "32: v" + String(VBAT1));
  // float VBAT2 = analogRead(33) * (7.2 / 4095.0);
  // display.drawStringMaxWidth(0, 40, 128, "33: v" + String(VBAT2));
  // display.display();
  //
  // if (messageIdle == false) {
  //   LoRa.beginPacket();
  //   LoRa.print(macString);
  //   LoRa.print("|");
  //
  //   Serial.println("*********");
  //   Serial.print("New serial value: ");
  //
  //   int len = 0;
  //
  //   for (int i = 0; message[i] != '\0'; i++) {
  //     Serial.print(message[i]);
  //     LoRa.print(message[i]);
  //
  //     len++;
  //   }
  //
  //   LoRa.endPacket();
  //
  //   Serial.println();
  //   Serial.println("length ");
  //   Serial.println(len);
  //   Serial.println("*********");
  //
  //   message[0] = '\0';
  //   messageIdle = true;
  // }
  //
  // // try to parse packet
  // int packetSize = LoRa.parsePacket();
  // if (packetSize) {
  //   char blah[packetSize + 1];
  //   // received a packet
  //   Serial.print("Received packet '");
  //   int count = 0;
  //   // read packet
  //   while (LoRa.available()) {
  //     char c = LoRa.read();
  //     Serial.print(c);
  //     blah[count] = c;
  //     count++;
  //   }
  //
  //   blah[count] = '\0';
  //
  //   pSerialChar->setValue(blah);
  //   pSerialChar->notify();
  //   // print RSSI of packet
  //   Serial.print("' with RSSI ");
  //   Serial.println(LoRa.packetRssi());
  // }
  //
  // tick++;
  //
  // delay(1000);
}
