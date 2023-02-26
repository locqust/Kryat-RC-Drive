# Kryat-RC-Drive

Arduino sketches for FrSky SBUS control of Kryat Drive
Bulk of original code thanks to Patrick Ryan and Darren Poulson. This code will utilise an ESP32 to control a BB8 over RC using SBus.

Required Libraries
https://www.arduino.cc/reference/en/libraries/dfrobotdfplayermini/
https://www.arduino.cc/reference/en/libraries/bolder-flight-systems-sbus/
https://www.arduino.cc/reference/en/libraries/esp32servo/
https://github.com/adafruit/Adafruit_NeoPixel

Usage
drive/constants.h
This file contains all the tweakable settings for the droid, including PWM values, channels to use, and pins on the esp32. Update the values to match how you have your RC system set up.

Hardware
ESP32 - 30 pin dev board for Body
SBus Receiver - I use an X8R
DFPlayer Mini

ESP8266 dev board for Dome
Neopixels strip for dome lights

TODO
Maybe add Neopixel code back in to light up the body, currently using a LED string light.
General cleanup of code.
Try to get automation dome roatation work
Start assigning proper audio files to buttons
Send light sequence change upon button trigger to dome
