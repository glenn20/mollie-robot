ARDUINO_DIR   = /opt/arduino
ARDMK_DIR     = /opt/arduino
AVR_TOOLS_DIR = /usr

ifneq ("$(wildcard ./config.mk)","")
include ./config.mk
endif

BOARD_TAG    ?= uno

ARDUINO_PORT ?= $(firstword $(wildcard /dev/ttyACM? /dev/ttyUSB?) NotFound)

ARDUINO_LIBS  = ArduinoJson PID_v1 microM AFMotor Servo Wire

# default: upload

include ./Arduino.mk

# This is an override of Arduino.mk so "make do_upload" won't second guess ARDUINO_PORT
#  - I need this to use avrdude-scp
get_monitor_port = $(ARDUINO_PORT)

