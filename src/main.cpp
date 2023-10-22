#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "secrets.h"

BLEServer* pServer = NULL;
BLE2902 *pBLE2902;
BLECharacteristic *lockCharacteristic;


bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// #define CHAR2_UUID "e3223119-9445-4e96-a4a1-85358c4046a2" 
#define SERVICE_UUID        "66b0a5cd-c1e8-4358-8e33-78045552379c"
#define CHAR1_UUID  "5860118b-4b4a-475c-8f2a-4f763059ca90" //the charistic I have to read here, write from the app
// #define KEY "PutYourSecretKeyHere"
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
class CharacteristicCallBack: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override { 
    std::string pChar2_value_stdstr = pChar->getValue();
    String pChar2_value_string = String(pChar2_value_stdstr.c_str());
    int pChar2_value_int = pChar2_value_string.toInt();
    Serial.println("pChar2: " + String(pChar2_value_int)); 
  }
};
void lock(){
  digitalWrite(GPIO_NUM_10, LOW);
  delay(250);
  digitalWrite(GPIO_NUM_10, GPIO_FLOATING);
}
void unlock(){
  digitalWrite(GPIO_NUM_9, LOW);
  delay(250);
  digitalWrite(GPIO_NUM_9, GPIO_FLOATING);
}
void setup() {
  pinMode(GPIO_NUM_10, OUTPUT);
  pinMode(GPIO_NUM_9, OUTPUT);
  digitalWrite(GPIO_NUM_10, GPIO_FLOATING);
  digitalWrite(GPIO_NUM_9, GPIO_FLOATING);

  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Forester"); 
  
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
   lockCharacteristic = pService->createCharacteristic(CHAR1_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  lockCharacteristic->setCallbacks(new CharacteristicCallBack());
  // BLECharacteristic lockDescriptor(BLEUUID((uint16_t)0x2903));
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("waiting a client connection to notify...");
}


void loop() {
  // put your main code here, to run repeatedly:
  if (deviceConnected) {
    // if (event == BLECharacteristic::EVENT_WRITE) {}
    uint8_t* data = lockCharacteristic->getData();
    String dataString = String((char*)data);
    String lockString = String(KEY) + "1";
    String unlockString = String(KEY)+"2";
    //check with KEY
    if (dataString == lockString ){
            Serial.println("lock");

      lock();
      lockCharacteristic->setValue("");
    }else if (dataString == unlockString){
            Serial.println("unlock");
      unlock();
      lockCharacteristic->setValue("");

    }
  }

if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
