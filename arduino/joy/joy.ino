#include <SSD1306.h>
#include <math.h>
#include <algorithm>

#include <LoRa.h>

#include <BLEDevice.h>
//#include "BLEScan.h"

#include "motor-crypto.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("01bada55-dead-1337-beef-cac75c07b075");

static BLEClient *pClient;
static BLEAddress *pServerAddress = new BLEAddress("30:ae:a4:59:1f:e2");

static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;

#define SS      18
#define RST     14
#define DI0     26
#define BAND    470E6  //915E6 -- 这里的模式选择中，检查一下是否可在中国实用915这个频段

class MyClientCallbacks: public BLEClientCallbacks {
    void onConnect(BLEClient *pClient) {
      Serial.println(" - Client connected");
      doConnect = false;
      connected = true;
    };

    void onDisconnect(BLEClient *pClient) {
      Serial.println(" - Client disconnected");
      doConnect = true;
      connected = false;
    }
};


static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
}

bool connectToServer(BLEAddress pAddress) {

  BLEDevice::init("");
  BLEDevice::setPower(ESP_PWR_LVL_P7);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P7);
  if (pClient) delete pClient;
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallbacks());

  Serial.print("Forming a connection to ");
  Serial.println(pAddress.toString().c_str());

  if (pClient->isConnected()) {
    Serial.println(" - Already connected, disconnecting client");
    pClient->disconnect();
  }
  Serial.println(" - Connecting client");
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  if (!pClient->isConnected()) {
    Serial.println(" - Client failed to connect");
    return false;
  }
  Serial.println(" - Connected to server");
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    return false;
  }
  Serial.println(" - Found our characteristic");
  return true;

  // Read the value of the characteristic.
  // std::string value = pRemoteCharacteristic->readValue();
  // Serial.print("The characteristic value was: ");
  // Serial.println(value.c_str());
  //
  // pRemoteCharacteristic->registerForNotify(notifyCallback);
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {

      //
      Serial.print("Found our device!  address: ");
      advertisedDevice.getScan()->stop();

      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

// Store the Arduino pin associated with each input

// Select button is triggered when joystick is pressed
const byte PIN_BUTTON_SELECT = 2;

const byte PIN_BUTTON_RIGHT = 35;
const byte PIN_BUTTON_UP    = 34;
const byte PIN_BUTTON_DOWN  = 39;
const byte PIN_BUTTON_LEFT  = 38;

const byte PIN_ANALOG_X = 36;
const byte PIN_ANALOG_Y = 37;

int avgX = 0;
int avgY = 0;
int deltaX = 64;
int deltaY = 64;

SSD1306 display(0x3c, 4, 15);

int fontSize = 10;

void writeOled(String message, int drawX=0, int drawY=0) {
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


void setup() {
  Serial.begin(115200);

  pinMode(PIN_BUTTON_SELECT, INPUT);
  digitalWrite(PIN_BUTTON_SELECT, HIGH);

  pinMode(PIN_BUTTON_RIGHT, INPUT);
  digitalWrite(PIN_BUTTON_RIGHT, HIGH);

  pinMode(PIN_BUTTON_LEFT, INPUT);
  digitalWrite(PIN_BUTTON_LEFT, HIGH);

  pinMode(PIN_BUTTON_UP, INPUT);
  digitalWrite(PIN_BUTTON_UP, HIGH);

  pinMode(PIN_BUTTON_DOWN, INPUT);
  digitalWrite(PIN_BUTTON_DOWN, HIGH);

  const int numSamples = 8;
  for (int i=-1; i<numSamples; i++) {
    delay(50);
    int x = analogRead(PIN_ANALOG_X);
    int y = analogRead(PIN_ANALOG_Y);
    if (i<0) continue;

    avgX += float(x) / numSamples;
    avgY += float(y) / numSamples;
  }

  Serial.println(avgX);
  Serial.println(avgY);
  Serial.println(deltaX);
  Serial.println(deltaY);

  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  display.init();
  display.flipScreenVertically();

  // override the default CS, reset, and IRQ pins (optional)
  // LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
  // SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);

  if (!LoRa.begin(BAND)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  LoRa.setSignalBandwidth(500e3);
  LoRa.setPreambleLength(8);
  LoRa.setSpreadingFactor(7);
  LoRa.setSyncWord(0x7B);           // ranges from 0-0xFF, default 0x34, see API docs
  // register the receive callback
  Serial.println("LoRa init succeeded.");

  // doConnect = true;
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  // BLEScan* pBLEScan = BLEDevice::getScan();
  // pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  // pBLEScan->setActiveScan(true);
  // pBLEScan->start(30);
}

void printBuf(uint8_t *buf, uint16_t size) {
  for (int i=0; i<size; i++) {
    char hex[3] = {0};
    sprintf(hex, "%02X",buf[i]);
    Serial.print(hex);
  }
  Serial.println();
}

uint8_t key[] = { 'h', 'e', 'l', 'l', 'o' };
MotorCrypto motorCrypto = MotorCrypto(key, sizeof(key));

void loop() {

  // analogRead(PIN_ANALOG_X);
  const int AX = analogRead(PIN_ANALOG_X);
  // analogRead(PIN_ANALOG_Y);
  const int AY = analogRead(PIN_ANALOG_Y);

  double x = 0;
  double y = 0;

  if (fabs(avgX-AX) > deltaX || fabs(avgY-AY) > deltaY) {
    if (AX < avgX) {
      x = double(AX - avgX) / avgX;
    } else {
      x = double(AX - avgX) / (4095 - avgX);
    }
    if (AY < avgY) {
      y = double(AY - avgY) / avgY;
    } else {
      y = double(AY - avgY) / (4095 - avgY);
    }
  }

  const double angleJ = atan2(y,x) * 180. / M_PI;

  const double r = std::max(fabs(x),fabs(y));
  const double newX = r * cos(angleJ * M_PI / 180);
  const double newY = r * sin(angleJ * M_PI / 180);
  const int16_t dispX = 32+24*newX;
  const int16_t dispY = 32-24*newY;

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


  display.clear();
  display.drawCircle(32,32,31);
  if (dispX != 32 || dispY != 32) display.drawCircle(dispX,dispY,5);
  display.setPixel(dispX,dispY);
  display.drawStringMaxWidth(72, 0, 128, String(mode));
  display.drawStringMaxWidth(85, 0, 128, String(motorL));
  display.drawStringMaxWidth(110, 0, 128, String(motorR));

  display.drawStringMaxWidth(72, 42, 128, String(motorCrypto.getCounter()));
  display.drawStringMaxWidth(60, 52, 128, String(motorCrypto.getConnectionId()));

  // display.drawStringMaxWidth(85, 10, 128, String(scale));
  // display.drawStringMaxWidth(85, 20, 128, connected ? "ready" : (doConnect ? "find" : "no con"));
  // display.drawStringMaxWidth(85, 40, 128, String(angleM));
  display.display();

  MotorEncrypted encrypted = motorCrypto.encrypt(mode,motorL,motorR);

  LoRa.beginPacket(80);
  LoRa.write(encrypted.data,sizeof(encrypted.data));
  LoRa.endPacket();

  // display.clear();
  // display.drawStringMaxWidth(0, 0, 128, "x:" + String(dispX));
  // display.drawStringMaxWidth(0, 40, 128, "y:" + String(dispY));
  // display.display();


  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  // if (doConnect) {
  //   Serial.println("do connect!");
  //   doConnect = false;
  //   if (connectToServer(*pServerAddress)) {
  //     Serial.println("We are now connected to the BLE Server.");
  //     connected = true;
  //   } else {
  //     Serial.println("We have failed to connect to the server; there is nothin more we will do.");
  //     doConnect = true;
  //     connected = false;
  //   }
  // }
  //
  // // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // // with the current time since boot.
  // if (connected) {
  //   // String newValue = "Time since boot: " + String(millis()/1000);
  //   // Serial.println("Setting new characteristic value to \"" + newValue + "\"");
  //
  //   // Set the characteristic's value to be the array of bytes that is actually a string.
  //   uint8_t data[] = {mode,motorL,motorR};
  //   pRemoteCharacteristic->writeValue(data, 3, false);
  // }

  //
  // const int DS = digitalRead(PIN_BUTTON_SELECT);
  // const int DR = digitalRead(PIN_BUTTON_RIGHT);
  // const int DL = digitalRead(PIN_BUTTON_LEFT);
  // const int DU = digitalRead(PIN_BUTTON_UP);
  // const int DD = digitalRead(PIN_BUTTON_DOWN);
  // delay(100);
  // return;

  // delay(20);
}
