flash-bot:
	esptool.py -p /dev/cu.SLAB_USBtoUART -b 460800 write_flash 0x0 bot.bin

flash-joy:
	esptool.py -p /dev/cu.SLAB_USBtoUART -b 460800 write_flash 0x0 joy.bin

dump:
	esptool.py -p /dev/cu.SLAB_USBtoUART -b 460800 read_flash 0 0x400000 dump.bin
