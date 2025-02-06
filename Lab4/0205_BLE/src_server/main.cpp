#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdlib.h>
 
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000;
 
// TODO: add new global variables for your sensor readings and processed data
// HC-SR04 Pins
const int trigPin = D9;
const int echoPin = D10;
 
const float soundSpeed = 0.0343; // sound speed in cm/usec
 
float measureDistance();
float processSensorData(float rawDistance);
 
// TODO: Change the UUID to your own (any specific one works, but make sure they're different from others'). You can generate one here: https://www.uuidgenerator.net/
#define SERVICE_UUID        "1074f039-4c1f-4977-8282-5b18926081eb"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
 
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };
 
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};
 
// TODO: add DSP algorithm functions here
 
 
void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");
 
    // TODO: add codes for handling your sensor setup (pinMode, etc.)
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
 
    // TODO: name your device to avoid conflictions
    BLEDevice::init("XIAO_ESP32S3_Lily");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("Hello World");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}
 
void loop() {
    // TODO: add codes for handling your sensor readings (analogRead, etc.)
    float distance = measureDistance();
 
    // TODO: use your defined DSP algorithm to process the readings
    float processedData = processSensorData(distance);
 
   
    if (deviceConnected) {
        // Send new readings to database
        // TODO: change the following code to send your own readings and processed data
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        distance = measureDistance();
        processedData = processSensorData(distance);
 
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Distance: %.2f cm", processedData);
        pCharacteristic->setValue(buffer);
        pCharacteristic->notify();
 
        Serial.println(buffer);
        }
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising();  // advertise again
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    delay(1000);
}
 
float measureDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * soundSpeed / 2;
 
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  return distance;
}
 
float processSensorData(float rawDistance) {
    if (rawDistance < 0 || rawDistance > 400) {
        Serial.println("Invalid or out-of-range reading.");
        return 0;
    }
    return rawDistance;
}