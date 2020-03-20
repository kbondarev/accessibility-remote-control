// TODO HANDLE CONFIG MODE - WEB SERVER
// TODO CONFIG MODE - ADD "WAIT 3 SECOND"/"WAIT 5 SECONDS" AS A COMMAND SEQUENCE
// TODO SAVE CONFIG TO EEPROM / LOAD CONFIG FROM EEPROM

#include <ArduinoBLE.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <spieeprom.h>
#include <string.h>

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
#define DBG_WRITE(...)
#endif

#define BLE_PERIPHERAL_NAME "IRTx"
#define BLE_SERVICE_UUID "5C7D66C6-FC51-4A49-9D91-8C6439AEBA56"
#define BLE_CHAR "1010"

#define WIFI_SSID "Button Config"

#define PIN_BUTTON 2
#define PIN_BUZZER 3
#define PIN_CONFIG_MODE 5
#define PIN_STATUS_LED A3 // connect to blue of RGB pin
// #define PIN_STATUS_LED LED_BUILTIN // TODO change RGB pin ^
#define PIN_EEPROM_CS 7

#define BLINK_INTERVAL_SHORT 180 // milliseconds
#define BLINK_INTERVAL_LONG 1500 // milliseconds

// #define BUTTON_DEBOUNCE_TIME 5000 // debounce and avoid spamming signals
#define BUTTON_DEBOUNCE_TIME 100 // TODO change back to 5 seconds ^

SPIEEPROM eep(EEPROM_TYPE_16BIT, PIN_EEPROM_CS);

IPAddress ipAddress(10, 0, 0, 1);
WiFiServer server(80); // port 80
int wifiStatus = WL_IDLE_STATUS;

int oldBtnState = LOW;
long lastTimePushed = 0;

int statusLEDState = LOW;
long statusLEDPrevMillis = 0; // last time LED was toggled
int statusLEDValue = 0;
uint8_t statusLEDStep = 5; // -5 or 5

int isConfigMode = 0;

void eeReadCommands(uint8_t commands[], uint8_t *length)
{
  DBG_PRINTLN(F(">>> Reading from EEPROM..."));
  uint32_t msStart = millis();

  *length = eep.readByte(0);

  if (*length > 0 && *length != 0xFF) {
    eep.readByteArray(1, commands, *length);
  }

  // 0xFF is an empty eeprom byte
  if (*length == 0xFF) {
    *length = 0;
  }

  uint32_t msLapse = millis() - msStart;
  DBG_PRINT(F(">>> Read time: "));
  DBG_PRINT(msLapse);
  DBG_PRINTLN(F(" ms"));
}

void eeWriteCommands(uint8_t commands[], uint8_t length)
{
  DBG_PRINTLN(F(">>> Writing to EEPROM..."));
  uint32_t msStart = millis();

  DBG_PRINT(F("Commands size: "));
  DBG_PRINTLN(length);
  DBG_PRINT(F("Number of data bytes to write: "));
  DBG_PRINTLN(length);
  DBG_PRINT(F("Address: "));
  DBG_PRINTLN(0);
  DBG_PRINTLN();

  eep.write(0, length); // data size at first byte
  eep.write(1, commands, length);

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
    eep.readByteArray(a, d, 16);
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
  client.print(CONFIG_HTML1);
  client.print(CONFIG_HTML2);
  client.println();
}

void parseCommands(String line, uint8_t commands[], uint8_t *len)
{
  String substr = line.substring(line.indexOf('=') + 1, line.lastIndexOf(" "));
  // DBG_PRINTLN(substr);
  char str[substr.length()];
  substr.toCharArray(str, substr.length());

  // Extract the first token
  char *token = strtok(str, ",");
  commands[0] = atoi(token);
  // loop through the string to extract all other tokens
  int i = 1;
  while (token != NULL) {
    // DBG_PRINTLN(token); // printing each token
    token = strtok(NULL, ",");
    if (token != NULL) {
      commands[i] = atoi(token);
      i++;
    }
  }

  *len = i;
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
    DBG_PRINTLN("New client"); // print a message out the serial port
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
          // DO RESPONSE HERE

          sendHttpResponse(200, client);
          break; // exit loop and close http connection

        } else { // if you got a newline, then clear currentLine:
          currentLine = "";
        }
      } else if (c != '\r') { // if you got anything else but a carriage
                              // return character,
        currentLine += c;     // add it to the end of the currentLine
      }

      if (currentLine.endsWith("HTTP/") && currentLine.indexOf("c?=") > 0) {
        // The currentLine string was filled with path part of the first line
        // of the http request.
        // First line of HTTP looks like this:
        //      "GET /path/to/things HTTP/1.1"
        //
        // All commands are sent to path as a parameter list:
        //      "GET /c?=1,2,3,4,5,6,

        uint8_t commands[128] = {0}; // there will be no more than 128 commands
        uint8_t commandsLength = 0;

        parseCommands(currentLine, commands, &commandsLength);

        eeWriteCommands(commands, commandsLength);

#if PRINT_DEBUG
        DBG_PRINTLN("\n\rSAVED COMMANDS:");
        for (int i = 0; i < commandsLength; i++) {
          DBG_PRINTLN(commands[i]);
        }
#endif
      }
    } // end while connected

    // close the connection:
    client.stop();
    DBG_PRINTLN("client disconnected");
  } // end if client
} // end handleWifiConnections

void testButton()
{
  long now = millis();
  int newBtnState = digitalRead(PIN_BUTTON);
  if (newBtnState == HIGH && oldBtnState != newBtnState &&
      (now - lastTimePushed > BUTTON_DEBOUNCE_TIME)) {
    DBG_PRINTLN(">>>TEST: Button pushed!");
    playTune();
    lastTimePushed = now;
  }
  oldBtnState = newBtnState;
}

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
    // BLE Central not connected, therefore flash the Status LED
    toggleStatusLEDState(BLINK_INTERVAL_SHORT);
  }
}

void playTune()
{
  tone(PIN_BUZZER, 587);
  delay(220);
  tone(PIN_BUZZER, 784);
  delay(220);
  tone(PIN_BUZZER, 1046);
  delay(300);
  noTone(PIN_BUZZER);
}

void handleBLECommunications(BLEDevice peripheral,
                             BLECharacteristic characteristic)
{
  while (peripheral.connected()) {

    // Turn LED on
    digitalWrite(PIN_STATUS_LED, LOW);

    // while the peripheral is connected
    long now = millis();
    int newBtnState = digitalRead(PIN_BUTTON);
    if (newBtnState == HIGH && oldBtnState == LOW &&
        (now - lastTimePushed > BUTTON_DEBOUNCE_TIME)) {
      DBG_PRINTLN(">>> Button pushed");
      playTune();

      uint8_t commands[128] = {0};
      uint8_t length = 0;
      eeReadCommands(commands, &length);
      
      for (int i = 0; i < length; i++) {
        DBG_PRINT(F("Sending command: "));
        DBG_PRINTLN(commands[i]);

        characteristic.writeValue(commands[i]);
        delay(1); // wait until value is read
      }

      lastTimePushed = now;
    }
    oldBtnState = newBtnState;
  }
}

void scanAndConnectToEmitterDevice()
{
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised
    // service
    DBG_PRINTLN(F("BLE Peripheral discovered: "));
    DBG_PRINT(peripheral.address());
    DBG_PRINT(F(" '"));
    DBG_PRINT(peripheral.localName());
    DBG_PRINTLN(F("' "));
    DBG_PRINT(peripheral.advertisedServiceUuid());
    DBG_PRINTLN();

    if (peripheral.localName() != BLE_PERIPHERAL_NAME) {
      return;
    }

    BLE.stopScan();

    // CONNECT TO PERIPHERAL:

    DBG_PRINT(F("Connecting...\n\r"));

    if (peripheral.connect()) {
      DBG_PRINT(F("Connected\n\r"));
    } else {
      DBG_PRINT(F("Failed to connect!\n\r"));
      return;
    }

    // discover peripheral attributes
    DBG_PRINTLN("Discovering attributes ...");
    if (peripheral.discoverAttributes()) {
      DBG_PRINTLN("Attributes discovered");
    } else {
      DBG_PRINTLN("Attribute discovery failed!");
      peripheral.disconnect();
      return;
    }

    // retrieve the LED characteristic
    BLECharacteristic characteristic = peripheral.characteristic(BLE_CHAR);

    if (!characteristic) {
      DBG_PRINTLN("Peripheral does not the characteristic!");
      peripheral.disconnect();
      return;
    } else if (!characteristic.canWrite()) {
      DBG_PRINTLN("Peripheral does not have a writable characteristic!");
      peripheral.disconnect();
      return;
    }

    handleBLECommunications(peripheral,
                            characteristic); // loops inside while connected

    DBG_PRINTLN("Peripheral disconnected");
    // peripheral disconnected, start scanning again
    BLE.scanForUuid(BLE_SERVICE_UUID);
  }
}

void setup()
{
  Serial.begin(9600);

#if PRINT_DEBUG
  while (!Serial) {
    ;
  }
#endif

  DBG_PRINTLN(F("--------------- Trigger ---------------"));
  DBG_PRINTLN(F("---------------------------------------"));
  DBG_PRINTLN();

  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_CONFIG_MODE, INPUT_PULLUP);
  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // init EEPROM
  eep.setup();

#if PRINT_DEBUG
  dumpEEPROM(0, 128);
#endif

  isConfigMode = !digitalRead(PIN_CONFIG_MODE);
  // isConfigMode = true; // testing
  if (isConfigMode) {
    DBG_PRINTLN(F("!!! CONFIG MODE"));
    initializeWifi();
    printWifiStatus();
  } else {
    DBG_PRINTLN(F("!!! OPERATION MODE"));
    // initialize the BLE hardware and start scanning
    BLE.begin();
    BLE.scanForUuid(BLE_SERVICE_UUID);
  }
}

void loop()
{
  handleStatusLED();
  // testButton();

  if (isConfigMode) {
    handleWifiConnections();
  } else {
    scanAndConnectToEmitterDevice();
  }
}
