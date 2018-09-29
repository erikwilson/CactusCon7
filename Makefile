dump:
	esptool.py -p /dev/cu.SLAB_USBtoUART -b 460800 read_flash 0 0x150000 cc7.bin

flash:
	esptool.py -p /dev/cu.SLAB_USBtoUART -b 460800 write_flash 0x0 cc7.bin

key:
	esptool.py -p /dev/cu.SLAB_USBtoUART -b 460800 write_flash -z 0x291000 ~/Projects/CactusCon7/cactuscoinapi/utils/keys/spiffs/badge_1.bin
