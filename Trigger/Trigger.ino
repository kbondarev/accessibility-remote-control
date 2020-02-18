
#include <ArduinoBLE.h>

#define BLE_UUID "19B10000-E8F2-537E-4F6C-D104768A1214"

const int buttonPin1 = 2;
const int buttonPin2 = 3;

int oldBtn1 = -1;
int oldBtn2 = -1;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);

//  Serial.println("btn 1: "+String(oldBtn1)+"\n\rbtn 2: "+oldBtn2+"\n");

  // initialize the BLE hardware
  BLE.begin();

  Serial.println("BLE Master");

  // start scanning for peripherals
  BLE.scanForUuid(BLE_UUID);
  
}

void loop() {

  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "FANCY DEVICE") {
      return;
    }

    // stop scanning
    BLE.stopScan();

    controlLed(peripheral);

    // peripheral disconnected, start scanning again
    BLE.scanForUuid("19B10001-E8F2-537E-4F6C-D104768A1214");
  } 
  
}

void controlLed(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
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
  BLECharacteristic ledCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

  if (!ledCharacteristic) {
    Serial.println("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    return;
  } else if (!ledCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable LED characteristic!");
    peripheral.disconnect();
    return;
  }

  int isConnected = -1;
  while (peripheral.connected()) {
    // while the peripheral is connected
    if (isConnected == -1){
      Serial.println("!!!!");
      isConnected = 1;
    }
    
    // put your main code here, to run repeatedly:
    int btn1 = digitalRead(buttonPin1);
    int btn2 = digitalRead(buttonPin2);
    Serial.println(String(btn1)+"\t"+String(btn2));
    
    if (oldBtn1 != btn1 || oldBtn2 != btn2) {
      Serial.println("Button toggled");
      ledCharacteristic.writeValue((byte)0x01);
    }
    
    oldBtn1 = btn1;
    oldBtn2 = btn2;
  }

  Serial.println("Peripheral disconnected");
}
