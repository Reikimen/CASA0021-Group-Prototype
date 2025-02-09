// // BLE 服务配置 (必须与 Flutter 应用中的 UUID 一致)
// #define SERVICE_UUID        "0000ff01-0000-1000-8000-00805f9b34fb"
// #define CHARACTERISTIC_WIFI "0000ff03-0000-1000-8000-00805f9b34fb"
// #define CHARACTERISTIC_GPS  "0000ff02-0000-1000-8000-00805f9b34fb"

// // 硬件配置
// #define LED_PIN 2  // ESP32 内置LED引脚

// // 全局变量
// BLEServer *pServer;
// BLEService *pService;
// BLECharacteristic *pWifiCharacteristic;
// BLECharacteristic *pGpsCharacteristic;

// bool wifiCredentialsReceived = false;
// String ssid = "";
// String password = "";

// void parseWifiCredentials(String data) {
//   int ssidStart = data.indexOf("SSID:") + 5;
//   int ssidEnd = data.indexOf(",PASS:");
//   int passStart = ssidEnd + 6;
  
//   ssid = data.substring(ssidStart, ssidEnd);
//   password = data.substring(passStart);
  
//   Serial.println("Received WiFi Credentials:");
//   Serial.println("SSID: " + ssid);
//   Serial.println("Password: " + password);
  
//   wifiCredentialsReceived = true;
// }

// void attemptWifiConnection() {
//   Serial.println("Attempting WiFi connection...");
//   digitalWrite(LED_PIN, LOW);
  
//   WiFi.begin(ssid.c_str(), password.c_str());
  
//   int attempts = 0;
//   while (WiFi.status() != WL_CONNECTED && attempts < 10) {
//     delay(500);
//     Serial.print(".");
//     attempts++;
//   }
  
//   if (WiFi.status() == WL_CONNECTED) {
//     Serial.println("\nWiFi Connected!");
//     Serial.println("IP Address: " + WiFi.localIP().toString());
//     digitalWrite(LED_PIN, HIGH);
//   } else {
//     Serial.println("\nConnection Failed");
//     digitalWrite(LED_PIN, LOW);
//   }
// }

// void parseGpsData(String data) {
//   int latStart = data.indexOf("LAT:") + 4;
//   int latEnd = data.indexOf(",LON:");
//   int lonStart = latEnd + 5;
  
//   String latitude = data.substring(latStart, latEnd);
//   String longitude = data.substring(lonStart);
  
//   Serial.println("Received GPS Data:");
//   Serial.println("Latitude: " + latitude);
//   Serial.println("Longitude: " + longitude);
  
//   // 这里可以添加GPS数据处理逻辑（例如存储或转发）
// }

// // 修改回调函数处理方式
// class WifiDataCallback: public BLECharacteristicCallbacks {
//   void onWrite(BLECharacteristic *pCharacteristic) {
//     String value = pCharacteristic->getValue().c_str(); // 转换为Arduino String
//     if (value.length() > 0) {
//       parseWifiCredentials(value);
//       attemptWifiConnection();
//     }
//   }
// };

// class GpsDataCallback: public BLECharacteristicCallbacks {
//   void onWrite(BLECharacteristic *pCharacteristic) {
//     String value = pCharacteristic->getValue().c_str(); // 转换为Arduino String
//     if (value.length() > 0) {
//       parseGpsData(value);
//     }
//   }
// };

// void setupBLE() {
//   BLEDevice::init("ESP32-BLE-Server");
//   pServer = BLEDevice::createServer();
  
//   pService = pServer->createService(SERVICE_UUID);
  
//   // WiFi 特征配置
//   pWifiCharacteristic = pService->createCharacteristic(
//     CHARACTERISTIC_WIFI,
//     BLECharacteristic::PROPERTY_WRITE
//   );
//   pWifiCharacteristic->setCallbacks(new WifiDataCallback());
  
//   // GPS 特征配置
//   pGpsCharacteristic = pService->createCharacteristic(
//     CHARACTERISTIC_GPS,
//     BLECharacteristic::PROPERTY_WRITE
//   );
//   pGpsCharacteristic->setCallbacks(new GpsDataCallback());
  
//   pService->start();
  
//   BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//   pAdvertising->addServiceUUID(SERVICE_UUID);
//   pAdvertising->setScanResponse(true);
//   pAdvertising->start();
  
//   Serial.println("BLE Server Ready");
// }

