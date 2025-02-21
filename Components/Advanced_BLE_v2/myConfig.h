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

// ===== 添加全局变量存储解析结果 =====
String storedSSID = "";
String storedPASS = "";
float storedLAT = 0.0;
float storedLON = 0.0;

// ===== 存储函数实现 =====
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
  preferences.putULong("timestamp", millis() / 1000); // 使用设备运行时间作为时间戳
  preferences.end();
  
  Serial.printf("Saved GPS: %.6f,%.6f\n", lat, lon);
}

// ===== 专用解析函数 =====
void parseWiFiData(const String &data) {
  // 智能定位关键字段（兼容大小写）
  int ssidStart = data.indexOf("SSID:");
  int passStart = data.indexOf("PASS:");

  // 新增分隔符查找
  int delimiterPos = data.indexOf(',', ssidStart); // 查找SSID后的第一个逗号

  if (ssidStart == -1 || passStart == -1 || delimiterPos == -1) {
    Serial.println("[WiFi] 数据格式错误");
    return;
  }

  // 修正SSID截取范围（到逗号前）
  storedSSID = data.substring(ssidStart + 5, delimiterPos);
  storedSSID.trim();

  // 修正PASS截取范围（从PASS:后开始）
  storedPASS = data.substring(passStart + 5);
  storedPASS.trim();

  // 调试输出
  Serial.printf("解析结果: SSID=[%s], PASS=[%s]\n", 
    storedSSID.c_str(), storedPASS.c_str());
}

void parseGPSData(const String &data) {
  // 容错定位字段
  int latStart = data.indexOf("LAT:");
  int lonStart = data.indexOf("LON:");
  
  if(latStart == -1 || lonStart == -1) {
    Serial.println("[GPS] Invalid data format");
    return;
  }

  // 提取数值部分
  String latStr = data.substring(latStart + 4, lonStart);
  latStr.trim();
  storedLAT = latStr.toFloat();

  String lonStr = data.substring(lonStart + 4);
  lonStr.trim();
  
  // 处理可能存在的尾随字符
  int endPos = lonStr.indexOf(",");
  if(endPos != -1) lonStr = lonStr.substring(0, endPos);
  
  storedLON = lonStr.toFloat();

  // 数据有效性验证
  if(abs(storedLAT) > 90 || abs(storedLON) > 180) {
    Serial.println("[GPS] Invalid coordinate values");
    return;
  }
}

void loadStoredWiFi() {
  preferences.begin("wifi-config", true); // 只读模式打开
  storedSSID = preferences.getString("ssid", "");  // 第二个参数是默认值
  storedPASS = preferences.getString("password", "");
  preferences.end();

  Serial.printf("Loaded WiFi: %s,%s\n", storedSSID.c_str(), storedPASS.c_str());
}

void WiFi_Connector(){
  int counter = 60;
  // 清理旧连接
  WiFi.disconnect(true);
  delay(100);
  
  // 设置STA模式并启用自动重连
  WiFi.mode(WIFI_STA);
  WiFi.begin(storedSSID.c_str(), storedPASS.c_str());
  
  Serial.print("正在连接Wi-Fi");
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
    // 延迟100ms后重启广播
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
      saveWiFiCredentials(storedSSID, storedPASS);// 存储到非易失存储
      WiFi_Connector();
    } 
    else if(uuid == CHARACTERISTIC_GPS) {
      // Add GPS data processing logic here
      Serial.printf("[GPS] Received data (%d bytes): %s\n", value.length(), value.c_str());
      parseGPSData(value);
      saveGPSData(storedLAT, storedLON);// 持久化存储
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


