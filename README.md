
Tinker Stuff
============

A bunch of little projects for Arduino/ESP8266. I use the Sonof switches a lot. Some are switches operating light bulbs and I have reworked some to be remote temperature sensors with a DS1820. I also have an Adafruit Huzzah monitoring the output signals of my thermostat.

Arduino Setup to program Sonoff switch
======================================

Use:
* Generic ESP8266 Module
* Upload SPeed: 115200
* CPU Frequency: 80MHz
* Flash Frequency: 40MHz
* Flash Mode: DOUT
* Flash Size: 1M, 64k SPIFFS


Program Settings from Command Line
=========================

$ curl -X PUT http://192.168.123.1/config --header "Content-Type: application/json" -d @homie-example.json