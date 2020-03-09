#include <ArduinoBLE.h>
#include <IRremote.h>
#include <SPI.h>
#include <WiFiNINA.h>

#include "BLE_Protocol.h"
#include "Config_HTML.h"

#define DEBUG 1

#if DEBUG
#define DBG_PRINT(...) Serial.print(__VA_ARGS__)
#define DBG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define DBG_WRITE(...) Serial.write(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#define DBG_PRINTLN(...)
#define DBG_WRITE(...)
#endif

#define BLE_NAME "IRTx"
#define BLE_SERVICE_UUID "5C7D66C6-FC51-4A49-9D91-8C6439AEBA56"
#define BLE_CHAR "1010"
#define WIFI_SSID "IR Transmitter"

#define PIN_IR_SEND 9
#define PIN_IR_RECEIVE 11
#define PIN_CONFIG_MODE 6
#define PIN_STATUS_LED A3 // connect to blue

#define BLINK_INTERVAL_SHORT 500 // milliseconds
#define BLINK_INTERVAL_LONG 1500 // milliseconds

#define IR_RECEIVE_TIMEOUT 10000 // 10 seconds

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

void setup()
{
  Serial.begin(9600);

  DBG_PRINTLN(F("------------ IR Transmitter ------------"));
  DBG_PRINTLN(F("----------------------------------------"));
  DBG_PRINTLN();

  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_CONFIG_MODE, INPUT);

  delay(3000); // wait 3 seconds

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
  remoteChar.setEventHandler(BLEWritten, remoteCharWritten);

  // start advertising
  BLE.advertise();
}

void printBLEStatus()
{
  DBG_PRINTLN("\nBLE Peripheral is ready!");
  DBG_PRINTLN("\tBLE_NAME:\t" + String(BLE_NAME));
  DBG_PRINTLN("\tSERVICE_UUID:\t" + String(BLE_SERVICE_UUID));
  DBG_PRINTLN("\tCHARACTERISTIC:\t" + String(BLE_CHAR));
  DBG_PRINTLN("\t\t\t(value size: " + String(remoteChar.valueSize()) +
              " bytes)");
}

void remoteCharWritten(BLEDevice central, BLECharacteristic characteristic)
{
  unsigned long currentMillis = millis();

  if (characteristic.written()) {
    byte buf[characteristic.valueSize()];
    characteristic.readValue(buf, characteristic.valueLength());

    printBuffer(buf, characteristic.valueLength());
    DBG_PRINT(F(">>> Command Received: "));
    unsigned int rawData[77] = {
        4600, 4550, 500,  500,  550,  500,  500,  500,  500,  550,  500,
        500,  500,  1500, 550,  500,  500,  550,  450,  550,  500,  500,
        500,  550,  450,  550,  500,  500,  500,  550,  500,  500,  500,
        500,  500,  4550, 550,  1500, 500,  1550, 500,  1500, 550,  500,
        500,  550,  500,  500,  500,  500,  500,  550,  500,  500,  500,
        500,  500,  550,  450,  550,  500,  1500, 550,  1500, 500,  1550,
        500,  1500, 550,  1500, 500,  1550, 500,  1500, 550,  1500, 550};
    int cmd = buf[0];
    switch (cmd) {
    case TV_POWER:
      DBG_PRINT("TV_POWER");
      // THIS CASE BLOCK IS EXECUTED WHEN BUTTON IS PUSHED ON THE TRIGGER
      // DEVICE!

      irsend.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), 38);
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
    if (i < results->rawlen - 1)
      DBG_PRINT(","); // ',' not needed on last one
    if (!(i & 1))
      DBG_PRINT(" ");
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
    DBG_PRINT(results->value, HEX);
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

  WiFiClient client = server.available(); // listen for incoming clients

  if (client) {                // if you get a client,
    DBG_PRINTLN("new client"); // print a message out the serial port
    String currentLine =
        ""; // make a String to hold incoming data from the client
    int proccessCommand = 0;
    int command = -1;

    while (client.connected()) { // loop while the client's connected
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        DBG_WRITE(c);            // print it out the serial monitor
        if (c == '\n') {         // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a
          // row. that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // DO RESPONSE HERE
            if (proccessCommand) {
              proccessCommand = 0;

              // request to set command
              DBG_PRINTLN();
              DBG_PRINT(">>>>>>>>> proccessCommand: command=");
              DBG_PRINTLN(command, DEC);
              DBG_PRINTLN();

              delay(1000);

              unsigned long irStartTime = millis();
              decode_results irResults;
              int received = 0;
              while ((received == 0) &&
                     (millis() - irStartTime < IR_RECEIVE_TIMEOUT)) {
                if (irrecv.decode(&irResults)) {
                  received = 1;
                  DBG_PRINT(F("irResults="));
                  DBG_PRINTLN(irResults.value, HEX);

                  dumpIRCode(&irResults);

                  // TODO save to EEPROM

                  irrecv.resume(); // Receive the next value
                }
                delay(100);
              }

              if (received) {
                client.println("HTTP/1.1 200 OK");
              } else {
                // timed out
                client.println("HTTP/1.1 408 Request Timeout");
              }

            } else {
              //
              client.println("HTTP/1.1 200 OK");
            }
            // Construct the rest of the response
            client.println("Content-type:text/html");
            client.println();
            client.print(CONFIG_HTML);
            client.println();
            // break out of the while loop:
            break;
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
      }
    }
    // close the connection:
    client.stop();
    DBG_PRINTLN("client disconnected");
  }
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
