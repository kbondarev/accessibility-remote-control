#include <ArduinoBLE.h>
#include <IRremote.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <spieeprom.h>

#include "BLE_Protocol.h"
#include "Config_HTML.h"

#define PRINT_DEBUG 1

#if PRINT_DEBUG
#define DBG_PRINT(...) Serial.print(__VA_ARGS__)
#define DBG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define DBG_WRITE(...) Serial.write(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#define DBG_PRINTLN(...)
#endif

#define BLE_NAME "IRTx"
#define BLE_SERVICE_UUID "5C7D66C6-FC51-4A49-9D91-8C6439AEBA56"
#define BLE_CHAR "1010"

#define WIFI_SSID "ARB: IR Emitter"

#define PIN_IR_SEND 2     // connect it out of IR emitter led
#define PIN_IR_RECEIVE 3  // connect to pin 1 of TSO32338
#define PIN_CONFIG_MODE 5 // connect to switch and ground
#define PIN_STATUS_LED A3 // connect to blue of RGB LED
#define PIN_EEPROM_CS 7
// #define PIN_EEPROM_MOSI 8
// #define PIN_EEPROM_SCK 9
// #define PIN_EEPROM_MISO 10

#define BLINK_INTERVAL_SHORT 500 // milliseconds
#define BLINK_INTERVAL_LONG 1500 // milliseconds

#define IR_RECEIVE_TIMEOUT 10000 // 10 seconds
#define IR_FRQ 38

SPIEEPROM eep(EEPROM_TYPE_16BIT, PIN_EEPROM_CS);

IRsend irsend(PIN_IR_SEND);
IRrecv irrecv(PIN_IR_RECEIVE);

BLEService irService(BLE_SERVICE_UUID);
BLEUnsignedShortCharacteristic remoteChar(BLE_CHAR, BLERead | BLEWrite);

IPAddress ipAddress(10, 0, 0, 1);
WiFiServer server(80); // port 80
int wifiStatus = WL_IDLE_STATUS;

int statusLEDState = LOW;
long statusLEDPrevMillis = 0; // last time LED was toggled

int isConfigMode = 0;
bool isCentralConnected = false;

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
    uint8_t buf[characteristic.valueSize()];
    characteristic.readValue(buf, characteristic.valueLength());

    printBuffer(buf, characteristic.valueLength());
    DBG_PRINT(F(">>> Command Received: "));

    uint8_t cmd = buf[0];
    DBG_PRINT(F("["));
    DBG_PRINT(cmd);
    DBG_PRINTLN(F("]"));

    if (cmd == WAIT_3) {
      // wait 3 seconds
      DBG_PRINTLN(F(">>> WAITING 3 SECONDS!"));
      delay(3000);
    } else {
      // send IR Code
      uint32_t rawCode[255] = {0};
      uint32_t codeLen = 0;
      eeReadIRCode(1, rawCode, &codeLen);

      for (int i = 0; i < codeLen; i++) {
        DBG_PRINT(rawCode[i]);
        DBG_PRINT(" ");
      }
      DBG_PRINTLN();

      unsigned int * irCodeSend = (unsigned int*)&rawCode;
      unsigned int irCodeLengthSend = (unsigned int)codeLen;
      irsend.sendRaw(irCodeSend, irCodeLengthSend, IR_FRQ);

      DBG_PRINTLN(F(">>> IR CODE SENT!"));
    }
  }
}

#if PRINT_DEBUG
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
#endif

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

void sendHttpResponse(int statusCode, WiFiClient client)
{
  if (statusCode == 200) {
    client.println("HTTP/1.1 200 OK");
  } else if (statusCode == 408) {
    client.println("HTTP/1.1 408 Request Timeout");
  } else {
    client.println("HTTP/1.1 500 Internal Server Error");
  }
  client.println("Content-type:text/html");
  client.println();
  client.print(CONFIG_HTML);
  client.println();
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

  if (client) {                // if you get a client,
    DBG_PRINTLN("new client"); // print a message out the serial port
    // make a String to hold incoming data from the client
    String currentLine = "";
    int proccessCommand = 0;
    int codeId = -1;

    while (client.connected()) { // loop while the client's connected
      if (!client.available()) {
        continue;
      }

      char c = client.read(); // read a byte, then
      DBG_WRITE(c);           // print it out the serial monitor
      if (c == '\n') {        // if the byte is a newline character

        // if the current line is blank, you got two newline characters in a
        // row. that's the end of the client HTTP request, so send a response:
        if (currentLine.length() == 0) {
          int receivedIRCode = 0;
          // DO RESPONSE HERE
          if (!proccessCommand) {
            // Return the html page
            sendHttpResponse(200, client);
            break; // break out of the while loop:

          } else {
            // Process the codeId
            proccessCommand = 0;

            // request to set codeId
            DBG_PRINTLN();
            DBG_PRINT(">>>>>>>>> proccessCommand: codeId=");
            DBG_PRINTLN(codeId, DEC);
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
#if PRINT_DEBUG
                dumpIRCode(&irResults);
#endif

                // save IR code to EEPROM
                // first need to parse the raw data received from IR
                uint8_t codeSize = irResults.rawlen - 1;
                uint32_t irCode[codeSize];
                // for some reason we ignore the first element in results.rawbuf
                // and start at i=1
                for (int i = 1; i < irResults.rawlen; i++) {
                  // this decodes into actual time in microseconds
                  irCode[i - 1] = irResults.rawbuf[i] * USECPERTICK;
                }
                eeWriteIRCode(codeId, irCode, codeSize);

                irrecv.resume(); // Receive the next value
              }
              delay(100); // important! without this we don't receive IR
            }
            // END - wait for IR Code

            if (receivedIRCode) {
              sendHttpResponse(200, client);
#if PRINT_DEBUG
              // delay(60000 * 2);
              dumpEEPROM(codeId * 1024, 512);
#endif
              break;
            } else {
              // timed out
              sendHttpResponse(408, client);
              break;
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
        // the next code block parses the codeId number from the path:
        int idx = currentLine.indexOf("cmd/");
        if (idx >= 0) { // -1 means not found
          int startIdx = idx + 4;
          // find the index of the second space, 4 chars in "GET "
          int endIdx = currentLine.indexOf(" ", 4);
          String cmdStr = currentLine.substring(startIdx, endIdx);
          DBG_PRINT(">>>>>>>>>>> cmd=");
          DBG_PRINTLN(cmdStr);
          codeId = cmdStr.toInt(); // parsed command number
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

void printReadEEPROMChunk(int address, uint8_t data[], uint32_t chunk)
{
  for (int i = 0; i < chunk; i += 4) {

    DBG_PRINT(F(" 0x"));
    DBG_PRINT(address + i, HEX);
    DBG_PRINT(F(" ("));
    DBG_PRINT(address + i, DEC);
    DBG_PRINT(F("):\t"));

    if (data[i + 0] < 16) {
      DBG_PRINT(F("0"));
    }
    DBG_PRINT(data[i + 0], HEX);

    DBG_PRINT(F(" "));
    if (data[i + 1] < 16) {
      DBG_PRINT(F("0"));
    }
    DBG_PRINT(data[i + 1], HEX);

    DBG_PRINT(F(" "));
    if (data[i + 2] < 16) {
      DBG_PRINT(F("0"));
    }
    DBG_PRINT(data[i + 2], HEX);

    DBG_PRINT(F(" "));
    if (data[i + 3] < 16) {
      DBG_PRINT(F("0"));
    }
    DBG_PRINT(data[i + 3], HEX);

    DBG_PRINTLN();
  }
}

void eeReadIRCode(uint32_t codeId, uint32_t irCode[], uint32_t *codeSize)
{
  DBG_PRINTLN(F(">>> Reading from EEPROM..."));
  uint32_t msStart = millis();

  uint8_t dataSize[4] = {0};
  uint8_t readData[1020] = {0};

  // For each IR code we allocate 1024 bytes in the EEPROM. Hence, the address
  // of each code is its the codeId times 1024. The first 3 bytes are ignored.
  // The 4th byte represents the codeSize (number of uint32_t in the IR code
  // array; max value is 255). The following 1020 bytes represent the IR code,
  // were each 4 consecutive bytes are one uint32_t element of the IR code
  // array.
  const uint32_t dataSizeAddress = codeId * 1024 + 3;
  *codeSize = (uint32_t)eep.read_byte(dataSizeAddress);

  const uint32_t bytesToRead = (*codeSize * 4);
  const uint32_t dataStartAddress = dataSizeAddress + 1;

#if PRINT_DEBUG
  DBG_PRINT(F("Address: "));
  DBG_PRINTLN(dataSizeAddress);
  DBG_PRINT(F("Data starts at: "));
  DBG_PRINTLN(dataStartAddress);
  DBG_PRINTLN();
  DBG_PRINT(F("Code size: "));
  DBG_PRINTLN(*codeSize);
  DBG_PRINT(F("Number of bytes to read: "));
  DBG_PRINTLN(bytesToRead);
  DBG_PRINTLN();
#endif

  eep.read_byte_array(dataStartAddress, readData, bytesToRead);

#if PRINT_DEBUG
  DBG_PRINTLN(F("READ DATA:"));
  printReadEEPROMChunk(dataStartAddress, readData, bytesToRead);
  DBG_PRINTLN();
#endif

  int k = 0; // irCode counter
  for (int i = 0; i < bytesToRead; i += 4) {
    // merge 4 bytes into one integer
    irCode[k] = ((uint32_t)readData[i + 0] << 24) +
                ((uint32_t)readData[i + 1] << 16) +
                ((uint32_t)readData[i + 2] << 8) + ((uint32_t)readData[i + 3]);

    DBG_PRINT(k);
    DBG_PRINT(F("\t0x"));
    DBG_PRINT(irCode[k], HEX);
    DBG_PRINT(F(" ("));
    DBG_PRINT(irCode[k], DEC);
    DBG_PRINT(F(")"));
    DBG_PRINTLN();

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
void eeWriteIRCode(uint32_t codeId, uint32_t irCode[], uint8_t codeSize)
{
  // For each code we allocate 1024 bytes in the EEPROM.
  // The first byte of each code is the size of the raw IR code (as number of
  // uint32_t in the irCode).

  DBG_PRINTLN(F(">>> Writing to EEPROM..."));
  uint32_t msStart = millis();

  uint32_t addr = codeId * 1024;
  uint32_t startAddrData = addr + 4;

  DBG_PRINT(F("Code size: "));
  DBG_PRINTLN(codeSize);
  DBG_PRINT(F("Number of data bytes to write: "));
  DBG_PRINTLN(codeSize * 4);
  DBG_PRINT(F("Address: "));
  DBG_PRINTLN(addr);
  DBG_PRINT(F("Data starts at: "));
  DBG_PRINTLN(startAddrData);
  DBG_PRINTLN();

  // first 4-bytes (integer) is for the data size
  uint8_t codeSizeData[4] = {0, 0, 0, codeSize};
  eep.write(addr, codeSizeData, 4);

  for (uint32_t i = 0; i < codeSize; i++) {
    // Split each value in the irCode array into 4 bytes.
    // Because there are 4 bytes in each value (1 uint32_t = 4 bytes).
    uint8_t data[4];
    data[0] = irCode[i] >> 24;
    data[1] = irCode[i] >> 16;
    data[2] = irCode[i] >> 8;
    data[3] = irCode[i];

    uint32_t writeAddr = startAddrData + (i * 4);

    DBG_PRINT(i, DEC);
    DBG_PRINT(F("\t: "));
    DBG_PRINT(data[0], HEX);
    DBG_PRINT(F(" "));
    DBG_PRINT(data[1], HEX);
    DBG_PRINT(F(" "));
    DBG_PRINT(data[2], HEX);
    DBG_PRINT(F(" "));
    DBG_PRINT(data[3], HEX);
    DBG_PRINT(F("\t| 0x"));
    DBG_PRINT(irCode[i], HEX);
    DBG_PRINT(F("\t| "));
    DBG_PRINT(irCode[i], DEC);
    DBG_PRINT("-> ");
    DBG_PRINT(writeAddr, DEC);
    DBG_PRINTLN();
    // Write the 4 bytes onto the EEPROM

    eep.write(writeAddr, data, 4);
    // delay(500);
  }

  uint32_t msLapse = millis() - msStart;
  DBG_PRINT(F(">>> Write time: "));
  DBG_PRINT(msLapse);
  DBG_PRINTLN(F(" ms"));
}

// writes 0xFF to eeprom
void eeErase(uint32_t startAddr, uint32_t endAddr)
{
  const uint8_t chunk = 32; // cannot be higher! 32bytes is the size of the page
  uint8_t data[chunk];
  DBG_PRINTLN(F("Erasing..."));
  for (int i = 0; i < chunk; i++) {
    data[i] = 0xFF;
  }
  uint32_t msStart = millis();

  for (uint32_t a = startAddr; a <= endAddr; a += chunk) {
    if ((a & 0xFFF) == 0) {
      DBG_PRINTLN(a);
    }
    eep.write(a, data, chunk);
  }
  uint32_t msLapse = millis() - msStart;
  DBG_PRINT(F("Erase lapse: "));
  DBG_PRINT(msLapse);
  DBG_PRINTLN(F(" ms"));
}

#if PRINT_DEBUG
// dump eeprom contents, 16 bytes at a time.
// always dumps a multiple of 16 bytes.
void dumpEEPROM(uint32_t startAddr, uint32_t nBytes)
{
  DBG_PRINTLN();
  DBG_PRINT(F("EEPROM DUMP 0x"));
  DBG_PRINT(startAddr, HEX);
  DBG_PRINT(F(" 0x"));
  DBG_PRINT(nBytes, HEX);
  DBG_PRINT(F(" "));
  DBG_PRINT(startAddr);
  DBG_PRINT(F(" "));
  DBG_PRINTLN(nBytes);
  uint32_t nRows = (nBytes + 15) >> 4;

  for (uint32_t r = 0; r < nRows; r++) {
    uint8_t d[16] = {0};
    uint32_t a = startAddr + 16 * r;
    eep.read_byte_array(a, d, 16);
    DBG_PRINT(F("0x"));
    if (a < 16 * 16 * 16)
      DBG_PRINT(F("0"));
    if (a < 16 * 16)
      DBG_PRINT(F("0"));
    if (a < 16)
      DBG_PRINT(F("0"));
    DBG_PRINT(a, HEX);
    DBG_PRINT(F(" ("));
    DBG_PRINT(a, DEC);
    DBG_PRINT(F(") "));
    for (int c = 0; c < 16; c++) {
      if (d[c] < 16) {
        DBG_PRINT(F("0"));
      }
      DBG_PRINT(d[c], HEX);
      DBG_PRINT(c == 7 ? "  " : " ");
    }
    DBG_PRINTLN(F(""));
  }
}
#endif

void setup()
{
  Serial.begin(9600);

#if PRINT_DEBUG
  while (!Serial) {
    ;
  }
#endif

  DBG_PRINTLN(F("------------ IR Transmitter ------------"));
  DBG_PRINTLN(F("----------------------------------------"));
  DBG_PRINTLN();

  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_CONFIG_MODE, INPUT);

  eep.setup();
  /*
  // DELETE AND DUMP ALL EEPROM
  // DO THIS ONLY ONCE WHEN CONNECTING A NEW EEPROM CHIP
  eeErase(0, 32000);
  dumpEEPROM(0, 32000);
  */

  //  eeErase(0, 2048);

  // dump for testing
  // dumpEEPROM(1024, 1440); // TODO remove

  // uint32_t irCode[255] = {0};
  // uint32_t codeSize = 0;
  // eeReadIRCode(1, irCode, &codeSize);

  isConfigMode = digitalRead(PIN_CONFIG_MODE);
  // isConfigMode = true; // testing
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

int printed = 0;
void loop()
{
  handleStatusLED();
  if (!printed) {
    printed = 1;
    dumpEEPROM(1024, 1440); // TODO remove
  }

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
