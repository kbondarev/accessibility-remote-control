#include <ArduinoBLE.h>
#include <SPI.h>
#include <WiFiNINA.h>

#include "BLE_Protocol.h"
#include "Config_HTML.h"

#define BLE_NAME "IRTx"
#define BLE_SERVICE_UUID "5C7D66C6-FC51-4A49-9D91-8C6439AEBA56"
#define BLE_CHAR "1010"
#define WIFI_SSID "IR-Tx"

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

bool isCentralConnected = false;

void setup()
{
  Serial.begin(9600);
  delay(5000);

  Serial.println(F("------------ IR Transmitter ------------"));
  Serial.println(F("----------------------------------------"));
  Serial.println();

  pinMode(bleRxLedPin, OUTPUT);
  pinMode(bleStatusLedPin, OUTPUT);

  initializeBLE();
  printBLEStatus();

  // TODO: Wifi stuff
  // initializeWifi();
  // printWifiStatus();
}

void loop()
{
  handleStatusLED();

  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();
  if (central) {
    if (!isCentralConnected && central.connected()) {
      Serial.print(F("Device connected: "));
      Serial.println(central.address());
      isCentralConnected = true;
    } else {
      Serial.print(F("Device disconnected: "));
      Serial.println(central.address());
      isCentralConnected = false;
    }
  }

  // TODO: wifi stuff
  // It seems like Wifi cannot be enabled at the same time with bluetooth
  // need to have a switch that would go into setup mode: BLE off, wifi ON
  // handleWifiConnections();
}

void initializeBLE()
{
  while (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
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
  Serial.println("\nBLE Peripheral is ready!");
  Serial.println("\tBLE_NAME:\t" + String(BLE_NAME));
  Serial.println("\tSERVICE_UUID:\t" + String(BLE_SERVICE_UUID));
  Serial.println("\tCHARACTERISTIC:\t" + String(BLE_CHAR));
  Serial.println("\t\t\t(value size: " + String(remoteChar.valueSize()) +
                 " bytes)");
}

void remoteCharWritten(BLEDevice central, BLECharacteristic characteristic)
{
  unsigned long currentMillis = millis();

  if (characteristic.written()) {
    byte buf[characteristic.valueSize()];
    characteristic.readValue(buf, characteristic.valueLength());

    printBuffer(buf, characteristic.valueLength());
    Serial.print(">>> Command Received: ");

    // turn on the LED when receving signal
    bleRxLedState = HIGH;
    digitalWrite(bleRxLedPin, bleRxLedState);
    bleRxLedPreviousMillis = currentMillis;

    int cmd = buf[0];
    switch (cmd) {
    case TV_POWER:
      Serial.print("TV_POWER");
      break;
    case DVD_POWER:
      Serial.print("DVD_POWER");
      break;
    default:
      Serial.print("UNKNOWN!");
      break;
    }

    Serial.print("\t[");
    Serial.print(cmd); // print decimal value
    Serial.println("]");
  }
}

void initializeWifi()
{
  WiFi.config(ipAddress);
  wifiStatus = WiFi.beginAP(WIFI_SSID);
  while (wifiStatus != WL_AP_LISTENING) {
    Serial.println("Creating access point failed!");
    delay(3000); // wait 3 seconds
  }

  // wait 10 seconds for connection:
  // delay(10000);

  // start the web server on port 80
  server.begin();
}

void printWifiStatus()
{
  Serial.println("\nWiFi AP is ready!");

  // print the SSID of the network you're attached to:
  Serial.print("\tSSID:\t");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("\tIP Address:\t");
  Serial.println(ip);
}

void handleWifiConnections()
{
  if (wifiStatus != WiFi.status()) {
    // it has changed update the variable
    wifiStatus = WiFi.status();
    if (wifiStatus == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening
      // mode
      Serial.println("Device disconnected from AP");
    }
  }

  WiFiClient client = server.available(); // listen for incoming clients

  if (client) {                   // if you get a client,
    Serial.println("new client"); // print a message out the serial port
    String currentLine =
        ""; // make a String to hold incoming data from the client
    int found = 0;
    while (client.connected()) { // loop while the client's connected
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        if (c == '\n') {         // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a
          // row. that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200
            // OK) and a content-type so the client knows what's coming, then a
            // blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print(CONFIG_HTML);

            // The HTTP response ends with another blank line:
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

        if (currentLine.endsWith("&")) {
          Serial.println("!!!!!");

          found = currentLine.lastIndexOf("&", sizeof(currentLine));
          if (found == -1) {
            found = currentLine.lastIndexOf("?", sizeof(currentLine));
          }
          Serial.print(found);
          String s = currentLine.substring(
              found, currentLine.lastIndexOf("&", found + 1));
          Serial.println(s);
        }
      }
    }
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
  Serial.print("\n\t{");
  for (int i = 0; i < len; i++) {
    if (buf[i] < 0x10) {
      // print a leading zero
      Serial.print("0");
    }
    Serial.print(buf[i], HEX);
    if (i < len - 1) {
      Serial.print(" ");
    }
  }
  Serial.println("}");
}