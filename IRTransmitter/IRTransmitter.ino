#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <IRremote.h>
#include <Wire.h>
#include <extEEPROM.h>

#include "BLE_Protocol.h"
#include "Config_HTML.h"

#define PRINT_DEBUG 1

#if PRINT_DEBUG
#define DBG_PRINT(...) Serial.print(__VA_ARGS__)
#define DBG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#define DBG_PRINTLN(...)
#endif

#define BLE_NAME "IRTx"
#define BLE_SERVICE_UUID "5C7D66C6-FC51-4A49-9D91-8C6439AEBA56"
#define BLE_CHAR "1010"
#define WIFI_SSID "IR Transmitter"

#define PIN_IR_SEND 9
#define PIN_IR_RECEIVE 10
#define PIN_CONFIG_MODE 6
#define PIN_STATUS_LED A3 // connect to blue

#define BLINK_INTERVAL_SHORT 500 // milliseconds
#define BLINK_INTERVAL_LONG 1500 // milliseconds

#define IR_RECEIVE_TIMEOUT 10000 // 10 seconds
#define IR_FRQ 38
// max size of the raw IR code
// #define IR_CODE_MAX_SIZE 256
// // number of IR codes
// #define IR_CODES_TOTAL 32

// Using the 24LC256 chip for EEPROM
// kbits_256 - Size of EEPROM
// 1         - Using only one EEPROM chip
// 32        - page size
// 0x50      - EEPROM address on I2C
extEEPROM eep(kbits_256, 1, 32, 0x50);

IRsend irsend(PIN_IR_SEND);
IRrecv irrecv(PIN_IR_RECEIVE);

BLEService irService(BLE_SERVICE_UUID);
BLEUnsignedShortCharacteristic remoteChar(BLE_CHAR, BLERead | BLEWrite);

IPAddress ipAddress(10, 1, 1, 1);
WiFiServer server(80); // port 80
int wifiStatus = WL_IDLE_STATUS;

int statusLEDState = 0;
long statusLEDPrevMillis = 0; // last time LED was toggled

int isConfigMode = 0;
bool isCentralConnected = false;

// Each code is an array of raw data up to IR_CODE_MAX_SIZE values.
// The index of each code matches to the index of the codes defined in
// BLE_Protocol.h.
// For example: irCode[TV_POWER] would be the array of the raw code for TV_POWER
// uint32_t irCodesArray[IR_CODES_TOTAL][IR_CODE_MAX_SIZE] = {0};
// uint8_t irCodesLengths[IR_CODES_TOTAL] = {0};

// // TODO remove
// unsigned int *irCode;
// unsigned int irCodeLength;

void printBuffer(byte *buf, int len)
{
  DBG_PRINT("\n\t{");
  for (int i = 0; i < len; i++) {
    if (buf[i] < 0x10) {
      // print a leading zero
      DBG_PRINT("0");
    }
    DBG_PRINT(buf[i], HEX);
    if (i < len - 1) {
      DBG_PRINT(" ");
    }
  }
  DBG_PRINTLN("}");
}

void initializeBLE()
{
  while (!BLE.begin()) {
    DBG_PRINTLN(F("Starting BLE failed!"));
    delay(3000); // wait 3 seconds
  }

  // set advertised local name and service UUID:
  BLE.setLocalName(BLE_NAME);
  BLE.setAdvertisedService(irService);

  // add the characteristic to the service
  irService.addCharacteristic(remoteChar);

  // add service
  BLE.addService(irService);

  // set the initial value for the characteristic:
  remoteChar.writeValue(0);
  remoteChar.setEventHandler(BLEWritten, bleCharWritten);

  // start advertising
  BLE.advertise();
}

void printBLEStatus()
{
  DBG_PRINTLN("BLE Peripheral is ready!");
  DBG_PRINT("BLE_NAME: ");
  DBG_PRINTLN(BLE_NAME);
  DBG_PRINT("SERVICE_UUID: ");
  DBG_PRINTLN(BLE_SERVICE_UUID);
  DBG_PRINT("CHARACTERISTIC: ");
  DBG_PRINTLN(BLE_CHAR);
  DBG_PRINTLN();
}

// This function is called when a new value is written onto the characteristic
void bleCharWritten(BLEDevice central, BLECharacteristic characteristic)
{
  unsigned long currentMillis = millis();

  if (characteristic.written()) {
    byte buf[characteristic.valueSize()];
    characteristic.readValue(buf, characteristic.valueLength());

    printBuffer(buf, characteristic.valueLength());
    DBG_PRINT(F(">>> Command Received: "));
    // unsigned int rawData[77] = {
    //     4600, 4550, 500,  500,  550,  500,  500,  500,  500,  550,  500,
    //     500,  500,  1500, 550,  500,  500,  550,  450,  550,  500,  500,
    //     500,  550,  450,  550,  500,  500,  500,  550,  500,  500,  500,
    //     500,  500,  4550, 550,  1500, 500,  1550, 500,  1500, 550,  500,
    //     500,  550,  500,  500,  500,  500,  500,  550,  500,  500,  500,
    //     500,  500,  550,  450,  550,  500,  1500, 550,  1500, 500,  1550,
    //     500,  1500, 550,  1500, 500,  1550, 500,  1500, 550,  1500, 550};
    int cmd = buf[0];
    switch (cmd) {
    case TV_POWER:
      DBG_PRINT("TV_POWER");
      // THIS CASE BLOCK IS EXECUTED WHEN BUTTON IS PUSHED ON THE TRIGGER
      // DEVICE!

      // TODO
      // irsend.sendRaw(irCode, irCodeLength, IR_FRQ);

      break;
    case DVD_POWER:
      DBG_PRINT("DVD_POWER");
      break;
    default:
      DBG_PRINT("UNKNOWN!");
      break;
    }

    DBG_PRINT("\t[");
    DBG_PRINT(cmd); // print decimal value
    DBG_PRINTLN("]");
  }
}

void dumpIRCode(decode_results *results)
{
  // Start declaration
  DBG_PRINT("unsigned int  ");         // variable type
  DBG_PRINT("rawData[");               // array name
  DBG_PRINT(results->rawlen - 1, DEC); // array size
  DBG_PRINT("] = {");                  // Start declaration

  // Dump data
  for (int i = 1; i < results->rawlen; i++) {

    DBG_PRINT(results->rawbuf[i] * USECPERTICK, DEC);
    if (i < results->rawlen - 1) {
      DBG_PRINT(","); // ',' not needed on last one
    }
    if (!(i & 1)) {
      DBG_PRINT(" ");
    }
  }

  // End declaration
  DBG_PRINT("};"); //
  // Newline
  DBG_PRINTLN("");

  // Now dump "known" codes
  if (results->decode_type != UNKNOWN) {

    // Some protocols have an address
    if (results->decode_type == PANASONIC) {
      DBG_PRINT("unsigned int  addr = 0x");
      DBG_PRINT(results->address, HEX);
      DBG_PRINTLN(";");
    }

    // All protocols have data
    DBG_PRINT("unsigned int  data = 0x");
    // DBG_PRINT(results->value, HEX);
    DBG_PRINTLN(";");
  }
}

void initializeWifi()
{
  WiFi.config(ipAddress);
  wifiStatus = WiFi.beginAP(WIFI_SSID);
  while (wifiStatus != WL_AP_LISTENING) {
    DBG_PRINTLN("Creating access point failed!");
    delay(3000); // wait 3 seconds
  }

  delay(3000);

  // start the web server on port 80
  server.begin();
}

void printWifiStatus()
{
  DBG_PRINTLN("\nWiFi AP is ready!");

  // print the SSID of the network you're attached to:
  DBG_PRINT("\tSSID:\t");
  DBG_PRINTLN(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  DBG_PRINT("\tIP Address:\t");
  DBG_PRINTLN(ip);
}

void handleWifiConnections()
{
#if PRINT_DEBUG
  if (wifiStatus != WiFi.status()) {
    // it has changed update the variable
    wifiStatus = WiFi.status();
    if (wifiStatus == WL_AP_CONNECTED) {
      // a device has connected to the AP
      DBG_PRINTLN("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening
      // mode
      DBG_PRINTLN("Device disconnected from AP");
    }
  }
#endif

  WiFiClient client = server.available(); // listen for incoming clients

  if (client) { // if you get a client,
    DBG_PRINTLN("new client"); // print a message out the serial port
    // make a String to hold incoming data from the client
    String currentLine = "";
    int proccessCommand = 0;
    int command = -1;

    while (client.connected()) { // loop while the client's connected
      if (!client.available()) {
        continue;
      }

      char c = client.read(); // read a byte, then
      // DBG_WRITE(c);            // print it out the serial monitor
      if (c == '\n') { // if the byte is a newline character

        // if the current line is blank, you got two newline characters in a
        // row. that's the end of the client HTTP request, so send a response:
        if (currentLine.length() == 0) {
          int receivedIRCode = 0;
          // DO RESPONSE HERE
          if (!proccessCommand) {
            // Return the html page
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print(CONFIG_HTML);
            client.println();
            // break out of the while loop:
            break;

          } else {
            // Process the command
            proccessCommand = 0;

            // request to set command
            DBG_PRINTLN();
            DBG_PRINT(">>>>>>>>> proccessCommand: command=");
            DBG_PRINTLN(command, DEC);
            DBG_PRINTLN();

            // wait for IR Code
            unsigned long irStartTime = millis();
            decode_results irResults;

            while ((receivedIRCode == 0) &&
                   (millis() - irStartTime < IR_RECEIVE_TIMEOUT)) {

              if (irrecv.decode(&irResults)) {
                receivedIRCode = 1;
                DBG_PRINT(F("irResults="));
                DBG_PRINTLN(irResults.value, HEX);
                dumpIRCode(&irResults);
              }

              irrecv.resume(); // Receive the next value
            }
            // END - wait for IR Code

            if (receivedIRCode) {
              client.println("HTTP/1.1 200 OK");
            } else {
              // timed out
              client.println("HTTP/1.1 408 Request Timeout");
            }
          }

        } else { // if you got a newline, then clear currentLine:
          currentLine = "";
        }
      } else if (c != '\r') { // if you got anything else but a carriage
                              // return character,
        currentLine += c;     // add it to the end of the currentLine
      }

      if (currentLine.endsWith("HTTP/")) {
        // The currentLine string was filled with path part of the first line
        // of the http request.
        // First line of HTTP looks like this:
        //      "GET /path/to/things HTTP/1.1"
        //
        // All commands are sent to path:
        //      "GET /cmd/{command number}"
        // e.g: "GET /cmd/42"
        //
        // the next code block parses the command number from the path:
        int idx = currentLine.indexOf("cmd/");
        if (idx >= 0) { // -1 means not found
          int startIdx = idx + 4;
          // find the index of the second space, 4 chars in "GET "
          int endIdx = currentLine.indexOf(" ", 4);
          String cmdStr = currentLine.substring(startIdx, endIdx);
          DBG_PRINT(">>>>>>>>>>> cmd=");
          DBG_PRINTLN(cmdStr);
          command = cmdStr.toInt(); // parsed command number
          // set flag to proccess the command at the end of the HTTP request
          // body
          proccessCommand = 1;
        }
      }
    } // end while connected

    // close the connection:
    client.stop();
    DBG_PRINTLN("client disconnected");
  } // end if client
} // end handleWifiConnections

void toggleStatusLEDState(int interval)
{
  int now = millis();
  if (now - statusLEDPrevMillis > interval) {
    // toggle LED state
    statusLEDState = statusLEDState == HIGH ? LOW : HIGH;
    statusLEDPrevMillis = now;
    digitalWrite(PIN_STATUS_LED, statusLEDState);
  }
}

void handleStatusLED()
{
  if (isConfigMode) {
    // CONFIGURATION MODE:
    // Status LED should flash/blink slowly
    toggleStatusLEDState(BLINK_INTERVAL_LONG);
  } else {
    // OPERATION MODE:
    // Status LED should be ON when BLE is connected
    // Status LED should flash/blink quickly when BLE is NOT connected
    if (isCentralConnected) {
      if (statusLEDState == LOW) {
        // BLE Central got connected, therefore turn the Status LED on
        statusLEDState = HIGH;
        digitalWrite(PIN_STATUS_LED, statusLEDState);
      }
    } else {
      // BLE Central not connected, therefore flash the Status LED
      toggleStatusLEDState(BLINK_INTERVAL_SHORT);
    }
  }
}

/*
void loadCodesFromEEPROM()
{
  // DBG_PRINTLN(F("-----------------------------"));
  // DBG_PRINTLN(F(">>>>>>>>> loadCodesFromEEPROM"));
  // DBG_PRINTLN(F("-----------------------------"));

  uint32_t msStart = millis();

  for (int i = 0; i < IR_CODES_TOTAL; i++) {
    eeReadIRCode(i, irCodesArray[i], &irCodesLengths[i]);
  }

  uint32_t msLapse = millis() - msStart;

  // DBG_PRINTLN(F("-----------------------------"));
  // DBG_PRINT(F(">>>>>>>>> loadCodesFromEEPROM DONE: "));
  // DBG_PRINT(msLapse);
  // DBG_PRINTLN(F(" ms"));
  // DBG_PRINTLN(F("-----------------------------"));
}
*/
void eeReadIRCode(uint32_t codeId, uint32_t *irCode, uint8_t *codeSize)
{
  // DBG_PRINTLN(F(">>> Reading from EEPROM..."));
  uint32_t msStart = millis();

  uint32_t addr = codeId * 1024;
  // first byte is the length of the IR Code (number of uint32_t in the array)
  *codeSize = eep.read(addr);
  uint32_t startAddrData = addr + 1;
  uint8_t bytesToRead = *codeSize * 4;

  DBG_PRINT(F("Number of bytes to read: "));
  DBG_PRINTLN(bytesToRead);
  DBG_PRINT(F("Data start address: "));
  DBG_PRINTLN(startAddrData);
  DBG_PRINTLN();

  uint32_t k = 0;
  for (uint32_t i = startAddrData; i < bytesToRead; i += 4) {
    uint8_t data[4];
    eep.read(i, data, 4); // read 4 bytes into data

    DBG_PRINT(F("Read bytes: "));
    DBG_PRINT(data[0], HEX);
    DBG_PRINT(F(" "));
    DBG_PRINT(data[1], HEX);
    DBG_PRINT(F(" "));
    DBG_PRINT(data[2], HEX);
    DBG_PRINT(F(" "));
    DBG_PRINT(data[3], HEX);
    DBG_PRINTLN();

    // add the 4 bytes to construct one uint32_t:
    irCode[k] = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
                ((uint32_t)data[2] << 8) | ((uint32_t)data[3]);

    DBG_PRINT(F("Summed bytes: 0x"));
    DBG_PRINT(irCode[k], HEX);
    DBG_PRINT(F(" | "));
    DBG_PRINTLN(irCode[k], DEC);
    DBG_PRINTLN(F("--------\n"));

    k++;
  }

  uint32_t msLapse = millis() - msStart;
  DBG_PRINT(F(">>> Read time: "));
  DBG_PRINT(msLapse);
  DBG_PRINTLN(F(" ms"));
}

/**
 * Write IR Code to EEPROM
 * codeId - the ID of the code you wish to write
 */
void eeWriteIRCode(uint32_t codeId, uint32_t *irCode, uint8_t codeSize)
{
  // For each code we allocate 1024 bytes in the EEPROM.
  // The first byte of each code is the size of the raw IR code (as number of
  // uint32_t in the irCode).

  DBG_PRINTLN(F(">>> Writing to EEPROM..."));
  uint32_t msStart = millis();

  uint32_t addr = codeId * 1024;
  uint32_t startAddrData = addr + 1;

  DBG_PRINT(F("Number of bytes to write: "));
  DBG_PRINTLN(codeSize * 4);
  DBG_PRINT(F("Data start address: "));
  DBG_PRINTLN(startAddrData);
  DBG_PRINTLN();

  eep.write(addr, codeSize); // first byte is allocated for data size

  for (uint32_t i = 0; i < codeSize; i++) {
    // Split each value in the irCode array into 4 bytes.
    // Because there are 4 bytes in each value (1 uint32_t = 4 bytes).
    uint8_t data[4];
    data[0] = irCode[i] >> 24;
    data[1] = irCode[i] >> 16;
    data[2] = irCode[i] >> 8;
    data[3] = irCode[i];

    DBG_PRINT(i, DEC);
    DBG_PRINT("\t\t: ");
    DBG_PRINT(data[0], HEX);
    DBG_PRINT(F(" "));
    DBG_PRINT(data[1], HEX);
    DBG_PRINT(F(" "));
    DBG_PRINT(data[2], HEX);
    DBG_PRINT(F(" "));
    DBG_PRINT(data[3], HEX);
    DBG_PRINTLN();

    // Write the 4 bytes onto the EEPROM
    eep.write(startAddrData + (i * 4 + 0), data[0]);
    eep.write(startAddrData + (i * 4 + 1), data[1]);
    eep.write(startAddrData + (i * 4 + 2), data[2]);
    eep.write(startAddrData + (i * 4 + 3), data[3]);
  }

  uint32_t msLapse = millis() - msStart;
  DBG_PRINT(F(">>> Write time: "));
  DBG_PRINT(msLapse);
  DBG_PRINTLN(F(" ms"));
}

// writes 0xFF to eeprom
void eeErase(uint32_t startAddr, uint32_t endAddr)
{
  const uint8_t chunk = 64;
  uint8_t data[chunk];
  DBG_PRINTLN(F("Erasing..."));
  for (int i = 0; i < chunk; i++)
    data[i] = 0xFF;
  uint32_t msStart = millis();

  for (uint32_t a = startAddr; a <= endAddr; a += chunk) {
    if ((a & 0xFFF) == 0)
      DBG_PRINTLN(a);
      eep.write(a, data, chunk);
  }
  uint32_t msLapse = millis() - msStart;
  DBG_PRINT(F("Erase lapse: "));
  DBG_PRINT(msLapse);
  DBG_PRINT(F(" ms"));
}

#if PRINT_DEBUG
// dump eeprom contents, 16 bytes at a time.
// always dumps a multiple of 16 bytes.
void dumpEEPROM(uint32_t startAddr, uint32_t nBytes)
{
  DBG_PRINT(F("EEPROM DUMP 0x"));
  DBG_PRINT(startAddr, HEX);
  DBG_PRINT(F(" 0x"));
  DBG_PRINT(nBytes, HEX);
  DBG_PRINT(F(" "));
  DBG_PRINT(startAddr);
  DBG_PRINT(F(" "));
  DBG_PRINTLN(nBytes);
  uint32_t nRows = (nBytes + 15) >> 4;

  uint8_t d[16];
  for (uint32_t r = 0; r < nRows; r++) {
    uint32_t a = startAddr + 16 * r;
    eep.read(a, d, 16);
    DBG_PRINT(F("0x"));
    if (a < 16 * 16 * 16)
      DBG_PRINT(F("0"));
      if (a < 16 * 16)
        DBG_PRINT(F("0"));
        if (a < 16)
          DBG_PRINT(F("0"));
          DBG_PRINT(a, HEX);
          DBG_PRINT(F(" "));
          for (int c = 0; c < 16; c++) {
            if (d[c] < 16) {
              DBG_PRINT(F("0"));
              DBG_PRINT(d[c], HEX);
              DBG_PRINT(c == 7 ? "  " : " ");
            }
          }
    DBG_PRINTLN(F(""));
  }
}
#endif

void setup()
{
  Serial.begin(9600);

  DBG_PRINTLN(F("------------ IR Transmitter ------------"));
  DBG_PRINTLN(F("----------------------------------------"));
  DBG_PRINTLN();

  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_CONFIG_MODE, INPUT);

  delay(5000); // wait 3 seconds

  // init EEPROM
  byte i2cStat = eep.begin(eep.twiClock400kHz);
  if (i2cStat != 0) {
    DBG_PRINTLN(F("Failed to connect to external EEPROM!"));
  } else {
    DBG_PRINTLN(F("Initilized external EEPROM successfully"));
  }

  // loadCodesFromEEPROM();

  // isConfigMode = digitalRead(PIN_CONFIG_MODE);
  isConfigMode = true; // testing
  if (isConfigMode) {
    DBG_PRINTLN(F("!!! CONFIG MODE"));
    initializeWifi();
    printWifiStatus();
    irrecv.enableIRIn();
  } else {
    DBG_PRINTLN(F("!!! OPERATION MODE"));
    initializeBLE();
    printBLEStatus();
  }
}

void loop()
{
  handleStatusLED();

  if (isConfigMode) {
    handleWifiConnections();
  } else {
    // listen for BLE peripherals to connect:
    BLEDevice central = BLE.central();
    if (central) {
      if (!isCentralConnected && central.connected()) {
        DBG_PRINT(F("Device connected: "));
        DBG_PRINTLN(central.address());
        isCentralConnected = true;
      } else if (!central.connected()) {
        DBG_PRINT(F("Device disconnected: "));
        DBG_PRINTLN(central.address());
        isCentralConnected = false;
      }
    }
  }
}
