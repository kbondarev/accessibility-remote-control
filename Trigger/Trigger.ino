// TODO HANDLE CONFIG MODE - WEB SERVER
// TODO ADD BLE PROTOCOL CODES
// TODO CONFIG MODE - ADD "WAIT 3 SECOND"/"WAIT 5 SECONDS" AS A COMMAND SEQUENCE
// TODO SAVE CONFIG TO EEPROM / LOAD CONFIG FROM EEPROM
// TODO CHANGE TUNE

#include <ArduinoBLE.h>

#include "BLE_Protocol.h"

#define BLE_PERIPHERAL_NAME "IRTx"
#define BLE_SERVICE_UUID "5C7D66C6-FC51-4A49-9D91-8C6439AEBA56"
#define BLE_CHAR "1010"

#define DEBUG  1

#if DEBUG
#	define DBG_PRINT(...)    Serial.print(__VA_ARGS__)
#	define DBG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#	define DBG_PRINT(...)
#	define DBG_PRINTLN(...)
#endif

#define PIN_BUTTON 7
#define PIN_BUZZER 9
#define PIN_CONFIG_MODE 6
// #define PIN_STATUS_LED A3  // connect to blue of RGB pin
#define PIN_STATUS_LED LED_BUILTIN // TODO change RGB pin ^

#define BLINK_INTERVAL_SHORT 500 // milliseconds
#define BLINK_INTERVAL_LONG 1500 // milliseconds

// #define BUTTON_DEBOUNCE_TIME 5000 // debounce and avoid spamming signals
#define BUTTON_DEBOUNCE_TIME 100 // TODO change back to 5 seconds ^

int oldBtnState = LOW;
long lastTimePushed = 0;

int statusLedState = LOW;
long statusLEDPrevMillis = 0; // last time LED was toggled

void setup()
{
  Serial.begin(9600);

  DBG_PRINTLN(F("--------------- Trigger ---------------"));
  DBG_PRINTLN(F("---------------------------------------"));
  DBG_PRINTLN();

    // initialize the BLE hardware
  BLE.begin();
  // start scanning for peripherals
  BLE.scanForUuid(BLE_SERVICE_UUID);
}

void loop()
{
  blinkStatusLED();
  // testButton();

  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised
    // service
    DBG_PRINTLN(F("Found "));
    DBG_PRINT(peripheral.address());
    DBG_PRINT(F(" '"));
    DBG_PRINT(peripheral.localName());
    DBG_PRINTLN(F("' "));
    DBG_PRINT(peripheral.advertisedServiceUuid());
    DBG_PRINTLN();

    if (peripheral.localName() != BLE_PERIPHERAL_NAME) {
      return;
    }

    // stop scanning
    BLE.stopScan();

    controlLed(peripheral);

    // peripheral disconnected, start scanning again
    BLE.scanForUuid(BLE_SERVICE_UUID);
  }
}

void controlLed(BLEDevice peripheral)
{
  // connect to the peripheral
  DBG_PRINT(F("Connecting...\n"));

  if (peripheral.connect()) {
    DBG_PRINT(F("Connected\n"));
  } else {
    DBG_PRINT(F("Failed to connect!\n"));
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

  int isConnected = -1;
  while (peripheral.connected()) {
    // while the peripheral is connected
    isConnected = 1;
    }

    long now = millis();
    int newBtnState = digitalRead(PIN_BUTTON);
    if (newBtnState == HIGH && oldBtnState != newBtnState &&
        (now - lastTimePushed > pushInterval)) {
      DBG_PRINTLN(">>> Button pushed");
      int written = characteristic.writeValue(TV_POWER);
      if (written) {
        playTune();
      }
      lastTimePushed = now;
    }
    oldBtnState = newBtnState;
  }

  DBG_PRINTLN("Peripheral disconnected");
}

void testButton()
{
  long now = millis();
  int newBtnState = digitalRead(PIN_BUTTON);
  if (newBtnState == HIGH && oldBtnState != newBtnState &&
      (now - lastTimePushed > pushInterval)) {
    DBG_PRINTLN(">>> Button pushed");
    // characteristic.writeValue((byte)0x01);
    playTune();
    lastTimePushed = now;
  }
  oldBtnState = newBtnState;
}

void blinkStatusLED()
{
  int now = millis();
  if (now - statusLedMillis > BLINK_INTERVAL_SHORT) {
    statusLedMillis = now;
    if (statusLedState == HIGH) {
      digitalWrite(PIN_STATUS_LED, LOW);
      statusLedState = LOW;
    } else {
      digitalWrite(PIN_STATUS_LED, HIGH);
      statusLedState = HIGH;
    }
  }
}

void playTune()
{
  tone(PIN_BUZZER, 1000);
  delay(300);
  tone(PIN_BUZZER, 1200);
  delay(200);
  tone(PIN_BUZZER, 1500);
  delay(200);
  tone(PIN_BUZZER, 1000);
  delay(400);
  noTone(PIN_BUZZER);
}
