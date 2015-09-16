ARDUINO_DIR   = /usr/share/arduino
ARDMK_DIR     = /usr/share/arduino
AVR_TOOLS_DIR = /usr

ifneq ("$(wildcard ./config.mk)","")
include ./config.mk
endif

BOARD_TAG    ?= uno

AVRDUDE       = avrdude-scp
ARDUINO_PORT ?= pi@robbie:/dev/ttyS99
#ARDUINO_PORT ?= $(firstword $(wildcard /dev/ttyACM? /dev/ttyUSB?) NotFound)

ARDUINO_LIBS = PID_v1 microM AFMotor Servo Wire

# default: upload

include ./Arduino.mk

get_monitor_port = $(ARDUINO_PORT)

