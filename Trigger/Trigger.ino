
#include <ArduinoBLE.h>

#define BLE_PERIPHERAL_NAME "IR-Tx"
#define BLE_SERVICE_UUID "ABCD"
#define BLE_CHAR "1010"

const int buttonPin = 2;
const int buzzerPin = 9;

int oldBtnState = LOW;
long lastTimePushed = 0;
const int pushInterval = 2000; // 2 seconds

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.print(F("--- Trigger ---\n"));
  Serial.print(F("---------------\n"));

  pinMode(buttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  // initialize the BLE hardware
  BLE.begin();

  // start scanning for peripherals
  BLE.scanForUuid(BLE_SERVICE_UUID);
}

void loop() {
  
  // testButton();

  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print(F("Found "));
    Serial.print(peripheral.address());
    Serial.print(F(" '"));
    Serial.print(peripheral.localName());
    Serial.print(F("' "));
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.print(F("\n"));

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

void controlLed(BLEDevice peripheral) {
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
    if (isConnected == -1){
      isConnected = 1;
    }
    
    long now = millis();
    int newBtnState = digitalRead(buttonPin);
    if (newBtnState == HIGH && oldBtnState != newBtnState && (now - lastTimePushed > pushInterval)) {
      Serial.println(">>> Button pushed");
      characteristic.writeValue((byte)0x01);
      playTune();
      lastTimePushed = now;
    }
    oldBtnState = newBtnState;
  
    
  }

  Serial.println("Peripheral disconnected");
}


void testButton() {
  long now = millis();
    int newBtnState = digitalRead(buttonPin);
    if (newBtnState == HIGH && oldBtnState != newBtnState && (now - lastTimePushed > pushInterval)) {
      Serial.println(">>> Button pushed");
      // characteristic.writeValue((byte)0x01);
      playTune();
      lastTimePushed = now;
    }
    oldBtnState = newBtnState;
}

void playTune() {
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