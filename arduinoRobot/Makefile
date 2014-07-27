ARDUINO_DIR   = /usr/share/arduino
ARDMK_DIR     = /usr
AVR_TOOLS_DIR = /usr

ifneq ("$(wildcard ./config.mk)","")
include ./config.mk
endif

ARDUINO_PORT ?= $(firstword $(wildcard /dev/ttyACM? /dev/ttyUSB?) NotFound)
BOARD_TAG    ?= uno

ARDUINO_LIBS = 

# default: upload

include /usr/share/arduino/Arduino.mk

