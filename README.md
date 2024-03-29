# Accessibility Remote Control

Design project #3 for ES1050 course at University of Western Ontario.

This project contains the source code for Arduino MKR 1010 boards:
- **IREmitter** - the Arduino board that trasmits the infrared signal
- **Trigger** - the Arduino board which takes the user input (push button)

▶️ [Walk-Through Demo Video](https://youtu.be/Hx5gjYDBuyM)

## Dependecies

- [IRremote](https://github.com/z3t0/Arduino-IRremote) - library that helps to send/receive infrared signal 
- [WiFiNINA](https://www.arduino.cc/en/Reference/WiFiNINA) - library to control the WiFi and BLE chips on the MKR1010
- [spieeprom](https://github.com/kbondare/spieeprom) - library to help write configuration to an expernal EEPROM chip over SPI

## Hardware

### MKR1010 Board Pinout

![MKR1010 Board Pinout](./MKR_WiFi_1010_Pinout.jpg)

### 25LC640A (SPI EEPROM)

![MKR1010 Board Pinout](./25LC640A_pinout.png)


### TSOP32338 (IR Receiver @ 38kHz base frequency)

![TSOP32338 Pinout](./TSOP32338_pinout.png)
