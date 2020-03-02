#include <SPI.h>
#include <IRremote.h>
#include <ArduinoBLE.h>
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
#define WIFI_SSID "IR-Tx"

IRsend irsend(3);

BLEService irService(BLE_SERVICE_UUID);
BLEUnsignedShortCharacteristic remoteChar(BLE_CHAR, BLERead | BLEWrite);

IPAddress ipAddress(10, 1, 1, 1);

int wifiStatus = WL_IDLE_STATUS;
WiFiServer server(80);

const int bleRxLedPin = 2; // flashes when new BLE signal received
int bleRxLedState = LOW;
unsigned long bleRxLedPreviousMillis = 0; // last time the LED was updated
const long bleRxLedInterval = 700;        // milliseconds

const int bleStatusLedPin = LED_BUILTIN; // on when central connected
int bleStatusLedState = LOW;
long bleStatusLedPrevMillis = 0; // last time LED was toggled
const int bleStatusLedInterval = 700;

const int configModePin = 6;
int isConfigModeRead = 0;
int isConfigMode = 0;

bool isCentralConnected = false;

void setup()
{
  Serial.begin(9600);

  DBG_PRINTLN(F("------------ IR Transmitter ------------"));
  DBG_PRINTLN(F("----------------------------------------"));
  DBG_PRINTLN();

  pinMode(bleRxLedPin, OUTPUT);
  pinMode(bleStatusLedPin, OUTPUT);
  pinMode(configModePin, INPUT);

  delay(5000);

  isConfigMode = digitalRead(configModePin);

  if (isConfigMode) {
    DBG_PRINTLN(F("!!! CONFIG MODE"));
    initializeWifi();
  } else {
    DBG_PRINTLN(F("!!! OPERATION MODE"));
    initializeBLE();
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
    DBG_PRINTLN("Starting BLE failed!");
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

  printBLEStatus();
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
    DBG_PRINT(">>> Command Received: ");

    // turn on the LED when receving signal
    bleRxLedState = HIGH;
    digitalWrite(bleRxLedPin, bleRxLedState);
    bleRxLedPreviousMillis = currentMillis;

    int cmd = buf[0];
    switch (cmd) {
    case TV_POWER:
      DBG_PRINT("TV_POWER");
      // THIS CASE BLOCK IS EXECUTED WHEN BUTTON IS PUSHED ON THE TRIGGER
      // DEVICE!
      irsend.sendNEC(0x61A0F00F, 32);
      delay(40);
      irsend.sendNEC(0xFFFFFFFF, 32);
      delay(40);
      irsend.sendNEC(0xFFFFFFFF, 32);
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

void initializeWifi()
{
  WiFi.config(ipAddress);
  // attempt to connect to Wifi network:
  while (wifiStatus != WL_AP_LISTENING) {
    wifiStatus = WiFi.beginAP(WIFI_SSID);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
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
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    DBG_PRINTLN("new client");
    String requestLine = "";
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        requestLine += c;
        // DBG_WRITE(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println(
              "Connection: close"); // the connection will be closed after
                                    // completion of the response
          // client.println(
          //     "Refresh: 5"); // refresh the page automatically every 5 sec
          client.println();
          client.println(CONFIG_HTML);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        } else if (c == '\r') {
          DBG_PRINTLN(requestLine);
          if (requestLine.startsWith("GET")) {

          }
          requestLine = "";
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void handleStatusLED()
{
  if (isCentralConnected) {
    if (bleStatusLedState == LOW) {
      // BLE Central got connected, therefore turn status LED on
      digitalWrite(bleStatusLedPin, HIGH);
    }
  } else {
    // BLE Central not connected, threfore blink status LED
    int now = millis();
    if (now - bleStatusLedPrevMillis > bleStatusLedInterval) {
      bleStatusLedPrevMillis = now;
      if (bleStatusLedState == HIGH) {
        digitalWrite(bleStatusLedPin, LOW);
        bleStatusLedState = LOW;
      } else {
        digitalWrite(bleStatusLedPin, HIGH);
        bleStatusLedState = HIGH;
      }
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
