![CactusCon 2018](https://github.com/erikwilson/CactusCon7/raw/master/images/board1.png "CactusCon 2018")

# CactusCoin

## Hack All The Things

This year's badge is based around a TTGO LoRa OLED ESP32 and contains six capacitive 'buttons' driven from four sensor inputs (classic [n choose floor(n/2)](http://www.wolframalpha.com/input/?i=n+choose+floor(n%2F2)) problem). The ESP32 sports 4MB of programmable flash, WiFi, Bluetooth, dual core 240MHz processor, combined with a 433MHz long range radio and display. It requires a single 18650 lithium ion rechargable battery. Optional circuitry supports a motor driver which operates on the same lines as the OLED by disabling screen controls when using the motor controller.  Add in some d-pad action, motors, wheels, and a jöystick. It has an access point, captive portal, websockets, and hidden secrets. The firmware was written using Arduino. It’s completely hackable. Wear it, play with it, use it, modify it, break it, learn from it.

## Volunteering

Every year countless hackers toil for endless hours to bring you this most exquisite and bespoke artisanal CactusCon experience. Please email badge@cactuscon.com if you wish to sacrifice yourself and contribute to the great process. Help build the global hacker community!

![Volunteer](https://github.com/erikwilson/CactusCon7/raw/master/images/board2.png "Volunteer")

## ̶B̶u̶g̶s̶ ̶ Features and Pull Requests

Feel free to submit issues, or even better submit a pull request with extra functionality!

The coin provides a swiss army knife of utility, what can you make with it?

![Pull Request](https://github.com/erikwilson/CactusCon7/raw/master/images/duck1.gif "Pull Request")

## Parts

#### basic

* 1 [TTGO LoRa OLED ESP32](https://www.aliexpress.com/item/2pcs-sets-TTGO-LORA-SX1278-ESP32-0-96-OLED-16-Mt-bytes-128-Mt-bit-433Mhz/32832523252.html)
* 2 x [18 pos female header](https://www.digikey.com/product-detail/en/PPTC181LFBN-RC/S7016-ND/810156/?itemSeq=272349209)
* 1 [battery holder](https://www.digikey.com/product-detail/en/BH-18650-PC/BH-18650-PC-ND/3029216/?itemSeq=272342283)
* 1 [switch](https://www.digikey.com/product-detail/en/MHSS1105/679-1849-ND/1949465/?itemSeq=272294708)

#### bot upgrade

* 1 [Motor driver](https://www.sparkfun.com/products/14450)
* 2 x [8 pos female header](https://www.digikey.com/product-detail/en/sullins-connector-solutions/PPTC081LFBN-RC/S7006-ND/810147)
* 1 [set Wheels](https://www.sparkfun.com/products/13259)
* 1 [set Motors](https://www.sparkfun.com/products/13302)
* 2 x [2 pos female header](https://www.digikey.com/product-detail/en/sullins-connector-solutions/PPPC021LFBN-RC/S7035-ND/810174)
* 9v [battery connector](https://www.digikey.com/products/en?keywords=36-232-ND) (optional, can use 18650 power instead)
* 1.5" x 3 [Velcro Tape](https://www.amazon.com/VELCRO-Brand-Sticky-Fasteners-Perfect/dp/B00006IC2L)
* 2 x [Zip Tie](https://www.amazon.com/dp/B01018DC96/)
* 1 [Joystick](https://www.sparkfun.com/products/9032P) (for seperate badge)

![Spare Parts](https://github.com/erikwilson/CactusCon7/raw/master/images/board3.png "Spare Parts")

## Additional Arduino Libraries

* https://github.com/intrbiz/arduino-crypto
* https://github.com/sandeepmistry/arduino-LoRa
* https://github.com/ThingPulse/esp8266-oled-ssd1306
* https://github.com/bblanchon/ArduinoJson
* https://github.com/me-no-dev/AsyncTCP
* https://github.com/me-no-dev/ESPAsyncWebServer
* https://github.com/me-no-dev/EspExceptionDecoder
* https://github.com/me-no-dev/arduino-esp32fs-plugin
