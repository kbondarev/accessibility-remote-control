
#include <ArduinoBLE.h>

#define BLE_PERIPHERAL_NAME "IRTx"
#define BLE_SERVICE_UUID "5C7D66C6-FC51-4A49-9D91-8C6439AEBA56"
#define BLE_CHAR "1010"

// status LED should be flashing when not connected to peripheral device
// and on when connected
const int statusLedPin = LED_BUILTIN;
const int buttonPin = 2;
const int buzzerPin = 9;

int oldBtnState = LOW;
long lastTimePushed = 0;
const int pushInterval = 2000; // 2 seconds

int statusLedState = LOW;
long statusLedMillis = 0;
const int statusLedInterval = 500;

void setup()
{
  Serial.begin(9600);
  // while (!Serial);

  Serial.println(F("--------------- Trigger ---------------"));
  Serial.println(F("---------------------------------------"));
  Serial.println();

  pinMode(buttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(statusLedPin, OUTPUT);
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
    Serial.println(F("Found "));
    Serial.print(peripheral.address());
    Serial.print(F(" '"));
    Serial.print(peripheral.localName());
    Serial.println(F("' "));
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

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
  Serial.print(F("Connecting...\n"));

  if (peripheral.connect()) {
    Serial.print(F("Connected\n"));
  } else {
    Serial.print(F("Failed to connect!\n"));
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the LED characteristic
  BLECharacteristic characteristic = peripheral.characteristic(BLE_CHAR);

  if (!characteristic) {
    Serial.println("Peripheral does not the characteristic!");
    peripheral.disconnect();
    return;
  } else if (!characteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable characteristic!");
    peripheral.disconnect();
    return;
  }

  int isConnected = -1;
  while (peripheral.connected()) {
    // while the peripheral is connected
    if (isConnected == -1) {
      isConnected = 1;
      digitalWrite(statusLedPin, HIGH);
    }

    long now = millis();
    int newBtnState = digitalRead(buttonPin);
    if (newBtnState == HIGH && oldBtnState != newBtnState &&
        (now - lastTimePushed > pushInterval)) {
      Serial.println(">>> Button pushed");
      characteristic.writeValue((byte)0x01);
      playTune();
      lastTimePushed = now;
    }
    oldBtnState = newBtnState;
  }

  Serial.println("Peripheral disconnected");
}

void testButton()
{
  long now = millis();
  int newBtnState = digitalRead(buttonPin);
  if (newBtnState == HIGH && oldBtnState != newBtnState &&
      (now - lastTimePushed > pushInterval)) {
    Serial.println(">>> Button pushed");
    // characteristic.writeValue((byte)0x01);
    playTune();
    lastTimePushed = now;
  }
  oldBtnState = newBtnState;
}

void blinkStatusLED()
{
  int now = millis();
  if (now - statusLedMillis > statusLedInterval) {
    statusLedMillis = now;
    if (statusLedState == HIGH) {
      digitalWrite(statusLedPin, LOW);
      statusLedState = LOW;
    } else {
      digitalWrite(statusLedPin, HIGH);
      statusLedState = HIGH;
    }
  }
}

void playTune()
{
  tone(buzzerPin, 1000);
  delay(300);
  tone(buzzerPin, 1200);
  delay(200);
  tone(buzzerPin, 1500);
  delay(200);
  tone(buzzerPin, 1000);
  delay(400);
  noTone(buzzerPin);
}