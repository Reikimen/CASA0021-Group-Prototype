#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// ===== BLE Configuration =====
#define SERVICE_UUID        "0000ff01-0000-1000-8000-00805f9b34fb"  // Must match with the app
#define CHARACTERISTIC_WIFI "0000ff03-0000-1000-8000-00805f9b34fb"  // WiFi characteristic
#define CHARACTERISTIC_GPS  "0000ff02-0000-1000-8000-00805f9b34fb"  // GPS characteristic
#define DEVICE_NAME         "ESP32-Data-Receiver"                   // Device name

// ===== Global Variables =====
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pWifiCharacteristic;
BLECharacteristic *pGpsCharacteristic;
bool deviceConnected = false;

// ===== BLE Server Callback =====
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected!");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected!");
  }
};

// ===== BLE Callback Class =====
class DataCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String uuid = pCharacteristic->getUUID().toString().c_str();
    String value = pCharacteristic->getValue().c_str();

    // Distinguish data type based on characteristic UUID
    if(uuid == CHARACTERISTIC_WIFI) {
      Serial.printf("[WiFi] Received data (%d bytes): %s\n", value.length(), value.c_str());
      // Add WiFi data processing logic here
    } 
    else if(uuid == CHARACTERISTIC_GPS) {
      Serial.printf("[GPS] Received data (%d bytes): %s\n", value.length(), value.c_str());
      // Add GPS data processing logic here
    }
  }
};

// ===== Main Program =====
void setup() {
  Serial.begin(115200);
  
  // Initialize BLE
  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());  // Set connection callbacks

  // Create service
  pService = pServer->createService(SERVICE_UUID);

  // Configure WiFi characteristic
  pWifiCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_WIFI, 
    BLECharacteristic::PROPERTY_WRITE
  );
  pWifiCharacteristic->setCallbacks(new DataCallback());

  // Configure GPS characteristic
  pGpsCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_GPS, 
    BLECharacteristic::PROPERTY_WRITE
  );
  pGpsCharacteristic->setCallbacks(new DataCallback());

  // Start service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  
  Serial.println("BLE service started, waiting for connection...");
}

void loop() {
  // Keep an empty loop, all operations are handled by callbacks
  delay(1000);
}
