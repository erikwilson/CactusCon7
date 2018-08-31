# THE BADGE
* ESP32 LoRa & oLed board
* 18650 battery
* Joystick & Buttons?

## badge parts:
  * $12.80+ : SX1276 ESP32 w/ OLED https://wholesaler.alibaba.com/product-detail/TTGO-LORA32-868-915Mhz-SX1276-ESP32_60716800689.html
  * ~$3 : 18650 Battery https://www.imrbatteries.com/samsung-26f-18650-2600mah-5a-protected-button-top-battery/
  * $4 : Joystick https://www.sparkfun.com/products/9032
  * $2.22 : Battery Holder https://www.digikey.com/product-detail/en/mpd-memory-protection-devices/BH-18650-PC/BH-18650-PC-ND/3029216
  or <$1 https://www.digikey.com/product-detail/en/keystone-electronics/54/36-54-ND/2254090

## extra bot parts:
  * $5 : Motor driver https://www.sparkfun.com/products/14451
  * $4 : Gear motors https://www.sparkfun.com/products/13302
  * $3 : Wheels https://www.sparkfun.com/products/13259

## CACTUS COIN

* CactusCoin are generated using LoRa and proof of proximity.
* Each badge will contain a 'wallet' with a uuid and individual public/private key.
* Wallets are created/registered through on site hidden WiFi or LoRa broadcast (aka CactusCoin Node).
* Registration may contain additional personal data by encrypting it with a conference public key.
* Each badge will periodically broadcast it's wallet uuid (eg id_A).
* Nearby badges (as determined by rssi, snr, & time) will respond with pairing message & signature (id_A:id_B:id_B_sig_PKCS1).
* Requesting badge should respond with the other half of the coin request (id_B:id_A:id_A_sig_PKCS1).
* Newly generated coins are submitted with both parties cryptographic signatures of the pairing.
* The CactusCoin node will verify a coin submission using the registered public keys of the wallets, and create a new coin entry under each wallet.
* Duplicate coin submissions will/should occur, but only the first submission will update both wallets.
* Periodically badges may attempt to sync their wallet with a CactusCoin Node.
* Base stations in each room may provide real time data of attendee locations.

## CACTUS B0T

* Add on kit for badge
* Motor controller, motors, & wheels to move bot
* Remote control
  * LoRa (fast at low SF for 80 byte AES-HMAC messages)
  * BLE (poor range, maybe due to motor interference)
  * Classic Bluetooth (not tried, PS4 controller?)
  * Wifi AP Mode (implemented, captive portal & http/ws)
* Other servo control?
* Contest?!

## GAMES?
* Multi-player
  * Pandemic?
* Single-player
  * ?

## TOUCH

* Matrix Design (incomplete?!): https://github.com/espressif/esp-iot-solution/blob/master/documents/touch_pad_solution/touch_sensor_design_en.md#46-matrix-touch-button-firmware-design
* Correct equation is n choose floor(n/2), so 4 choose 2 = 6, 5 choose 2 = 10, 6 choose 3 = 20...! (where n = # capacitive sensor lines)
* Should allow for multi-touch. A button is composed of a set of lines, if all of the lines in the set are low the button is being pressed.

## ARDUINO LIBRARIES

* https://github.com/intrbiz/arduino-crypto
* https://github.com/sandeepmistry/arduino-LoRa
* https://github.com/nkolban/ESP32_BLE_Arduino
* https://github.com/ThingPulse/esp8266-oled-ssd1306
* https://github.com/me-no-dev/ESPAsyncWebServer
* https://github.com/me-no-dev/AsyncTCP

## OTHER LIBRARIES OF INTEREST

* https://github.com/bluekitchen/btstack
* https://github.com/felis/USB_Host_Shield_2.0

## OTHER REFERENCES

* https://github.com/espressif/arduino-esp32/
* https://esp-idf.readthedocs.io/en/latest/
* https://media.readthedocs.org/pdf/esp-idf/latest/esp-idf.pdf
* https://www.thethingsnetwork.org/forum/uploads/default/original/2X/a/a9f3e157802318f076c7eed7c657f433aad5f1bc.jpg
* http://nicerf.com/Upload/ueditor/files/2017-06-21/LORA1278-100mW%20long%20range%20Spread%20Spectrum%20modulation%20wireless%20transceiver%20module%20V2.1-d0f02517-7bc2-4307-89fd-c5a6df27300a.pdf
* https://www.ntia.doc.gov/files/ntia/publications/2003-allochrt.pdf
* https://arxiv.org/pdf/1607.08011.pdf
* https://arxiv.org/pdf/1712.02141.pdf
* http://www.sghoslya.com/p/lora_6.html
* https://docs.exploratory.engineering/lora/dr_sf/
* http://www.3glteinfo.com/lora/lorawan-frequency-bands/
* https://github.com/whitecatboard/Lua-RTOS-ESP32
* https://github.com/nodemcu/nodemcu-firmware/tree/dev-esp32
