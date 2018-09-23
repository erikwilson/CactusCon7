### Generating and flashing ID + RSA Private Key
`mkdir -p badges/1
openssl genrsa -out badges/1/private.pem 768
echo 1 > badges/1/my.id
mkspiffs-0.2.3-2-g4eae236-arduino-esp32-linux64/mkspiffs -c badges/1 -b 4096 -p 256 -s 0x16F000 badge_1.bin
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 write_flash -z 0x291000 badge_1.bin`
