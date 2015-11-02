ARDUINO_DIR   = /usr/share/arduino
ARDMK_DIR     = /usr/share/arduino
# AVR_TOOLS_DIR = /usr

ifneq ("$(wildcard ./config.mk)","")
include ./config.mk
endif

BOARD_TAG    ?= uno

ARDUINO_PORT ?= $(firstword $(wildcard /dev/ttyACM? /dev/ttyUSB?) NotFound)

ARDUINO_LIBS  = PID_v1 AFMotor Servo ArduinoJson MemoryFree # microM Wire

LOCAL_INO_SRCS  ?= arduinoRobot.ino
LOCAL_CC_SRCS   ?= Setup.cc Robot.cc Head.cc Wheel.cc Motor-AFMotor.cc Encoder.cc Comms-serial.cc

LOCAL_C_SRCS    ?= 
LOCAL_CPP_SRCS  ?= 
LOCAL_PDE_SRCS  ?=
LOCAL_AS_SRCS   ?=

# default: upload

include $(ARDMK_DIR)/Arduino.mk

# This is an override of Arduino.mk so "make do_upload" won't
# second guess ARDUINO_PORT - I need this to use avrdude-scp
get_monitor_port = $(ARDUINO_PORT)

