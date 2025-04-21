

	. ~/esp/esp-idf/export.sh

	idf.py -p /dev/cu.usbmodem1101 build flash

	idf.py -p /dev/cu.usbmodem1101 monitor