#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Preferences.h>

Preferences preferences;

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

// ===== Parsed「Wi-Fi & GPS info」 =====
String storedSSID = "";
String storedPASS = "";
float storedLAT = 0.0;
float storedLON = 0.0;

// ===== Stored function implementation =====
void saveWiFiCredentials(const String &ssid, const String &pass) {

  preferences.begin("wifi-config", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", pass);
  preferences.end();
  
  Serial.printf("Saved WiFi: %s,%s\n", ssid.c_str(), pass.c_str());
}

void saveGPSData(float lat, float lon) {

  preferences.begin("gps-data", false);
  preferences.putFloat("lat", lat);
  preferences.putFloat("lng", lon);
  preferences.putULong("timestamp", millis() / 1000); // Use the device run time as a timestamp
  preferences.end();
  
  Serial.printf("Saved GPS: %.6f,%.6f\n", lat, lon);
}

// ===== Special analytic(parse) function =====
void parseWiFiData(const String &data) {
  // Intelligently locate key fields (case compatible)
  int ssidStart = data.indexOf("SSID:");
  int passStart = data.indexOf("PASS:");

  // Added separator lookup
  int delimiterPos = data.indexOf(',', ssidStart); // Finds the first comma after the SSID

  if (ssidStart == -1 || passStart == -1 || delimiterPos == -1) {
    Serial.println("[WiFi] Data format error");
    return;
  }

  // Fixed SSID intercept range (before comma)
  storedSSID = data.substring(ssidStart + 5, delimiterPos);
  storedSSID.trim();

  // Fixed PASS intercept range (starting after PASS:)
  storedPASS = data.substring(passStart + 5);
  storedPASS.trim();

  // Debug output
  Serial.printf("Analysis results: SSID=[%s], PASS=[%s]\n", 
    storedSSID.c_str(), storedPASS.c_str());
}

void parseGPSData(const String &data) {
  // Fault tolerant location field
  int latStart = data.indexOf("LAT:");
  int lonStart = data.indexOf("LON:");
  
  if(latStart == -1 || lonStart == -1) {
    Serial.println("[GPS] Invalid data format");
    return;
  }

  // Extract the numeric part
  String latStr = data.substring(latStart + 4, lonStart);
  latStr.trim();
  storedLAT = latStr.toFloat();

  String lonStr = data.substring(lonStart + 4);
  lonStr.trim();
  
  // Handle possible trailing characters
  int endPos = lonStr.indexOf(",");
  if(endPos != -1) lonStr = lonStr.substring(0, endPos);
  
  storedLON = lonStr.toFloat();

  // Data validity verification
  if(abs(storedLAT) > 90 || abs(storedLON) > 180) {
    Serial.println("[GPS] Invalid coordinate values");
    return;
  }
}

void loadStoredWiFi() {
  preferences.begin("wifi-config", true); // Open in read-only mode
  storedSSID = preferences.getString("ssid", "");  // The second argument is the default
  storedPASS = preferences.getString("password", "");
  preferences.end();

  Serial.printf("Loaded WiFi: %s,%s\n", storedSSID.c_str(), storedPASS.c_str());
}

void WiFi_Connector(){
  int counter = 60;
  // Clear old connections
  WiFi.disconnect(true);
  delay(100);
  
  // Set STA mode and enable connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(storedSSID.c_str(), storedPASS.c_str());
  
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED && counter > 0) {
    counter--;
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\Wifi Connection Successful!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else{
    Serial.println("\nWifi Connection Failed x");
  }
}

// ===== BLE Server Callback =====
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device「BLE」connected!");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device「BLE」disconnected!");
    // Restart the broadcast after a delay of 100ms
    delay(100);
    pServer->startAdvertising();
    Serial.println("BLE re-boardcasting...");
  }
};

// ===== BLE Callback Class =====
class DataCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String uuid = pCharacteristic->getUUID().toString().c_str();
    String value = pCharacteristic->getValue().c_str();

    // Distinguish data type based on characteristic UUID
    if(uuid == CHARACTERISTIC_WIFI) {
      // Add WiFi data processing logic here
      Serial.printf("[WiFi] Received data (%d bytes): %s\n", value.length(), value.c_str());
      parseWiFiData(value);
      saveWiFiCredentials(storedSSID, storedPASS);// Store to non-volatile storage
      WiFi_Connector();
    } 
    else if(uuid == CHARACTERISTIC_GPS) {
      // Add GPS data processing logic here
      Serial.printf("[GPS] Received data (%d bytes): %s\n", value.length(), value.c_str());
      parseGPSData(value);
      saveGPSData(storedLAT, storedLON);// Store to non-volatile storage
    }
  }
};

void Compass_BLE_init(){
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
  pWifiCharacteristic->addDescriptor(new BLE2902());

  // Configure GPS characteristic
  pGpsCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_GPS, 
    BLECharacteristic::PROPERTY_WRITE
  );
  pGpsCharacteristic->setCallbacks(new DataCallback());
  pGpsCharacteristic->addDescriptor(new BLE2902());

  // Start service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  
  Serial.println("BLE service started, waiting for connection...");
}


