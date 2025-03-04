// Keep only one cancel comment, defining the mode of the current device
#define MODE_JACK   // The device is in Jack mode
// #define MODE_ROSE  // The device is in Rose mode
#define COUPLE_NUM "002" // The couple's number

// ===== MQTT Topic definition =====
#ifdef MODE_JACK
  // Jack 模式：A 为发布主题，B为订阅的主题
  const char* mqtt_topic_A_LAT = "student/CASA0021/Group2/" COUPLE_NUM "/Jack/LAT";
  const char* mqtt_topic_A_LON = "student/CASA0021/Group2/" COUPLE_NUM "/Jack/LON";
  const char* mqtt_topic_A_MODE = "student/CASA0021/Group2/" COUPLE_NUM "/Jack/MODE"; // for emotion
  const char* mqtt_topic_A_STATUS = "student/CASA0021/Group2/" COUPLE_NUM "/Jack/STATUS"; // for message status
  const char* mqtt_topic_B_LAT = "student/CASA0021/Group2/" COUPLE_NUM "/Rose/LAT";
  const char* mqtt_topic_B_LON = "student/CASA0021/Group2/" COUPLE_NUM "/Rose/LON";
  const char* mqtt_topic_B_MODE = "student/CASA0021/Group2/" COUPLE_NUM "/Rose/MODE"; // for emotion
  const char* mqtt_topic_B_STATUS = "student/CASA0021/Group2/" COUPLE_NUM "/Rose/STATUS"; // for message status
#elif defined(MODE_ROSE)
  // Rose 模式：A 为发布主题，B为订阅的主题
  const char* mqtt_topic_B_LAT = "student/CASA0021/Group2/" COUPLE_NUM "/Jack/LAT";
  const char* mqtt_topic_B_LON = "student/CASA0021/Group2/" COUPLE_NUM "/Jack/LON";
  const char* mqtt_topic_B_MODE = "student/CASA0021/Group2/" COUPLE_NUM "/Jack/MODE"; // for emotion
  const char* mqtt_topic_B_STATUS = "student/CASA0021/Group2/" COUPLE_NUM "/Jack/STATUS"; // for message status
  const char* mqtt_topic_A_LAT = "student/CASA0021/Group2/" COUPLE_NUM "/Rose/LAT";
  const char* mqtt_topic_A_LON = "student/CASA0021/Group2/" COUPLE_NUM "/Rose/LON";
  const char* mqtt_topic_A_MODE = "student/CASA0021/Group2/" COUPLE_NUM "/Rose/MODE"; // for emotion
  const char* mqtt_topic_A_STATUS = "student/CASA0021/Group2/" COUPLE_NUM "/Rose/STATUS"; // for message status
#else
  #error "Please define MODE_JACK or MODE_ROSE!"
#endif

// ===== BLE Configuration =====
#define SERVICE_UUID        "0000ff01-0000-1000-8000-00805f9b34fb"  // Must match with the app
#define CHARACTERISTIC_WIFI "0000ff03-0000-1000-8000-00805f9b34fb"  // WiFi characteristic
#define CHARACTERISTIC_GPS  "0000ff02-0000-1000-8000-00805f9b34fb"  // GPS characteristic

#ifdef MODE_JACK
  #define DEVICE_NAME         "ESP32-" COUPLE_NUM "-Jack"                   // Device name
#elif defined(MODE_ROSE)
  #define DEVICE_NAME         "ESP32-" COUPLE_NUM "-Rose"                   // Device name
#else
  #error "Please define MODE_JACK or MODE_ROSE!"
#endif

// ===== Global Variables =====
// Reading Status
bool ReadOrNot = true; // true表示已读，也表示自己没有待处理的对方情绪事件
int pairStatus = 0; // 0 for normal, 1 for happy, 2 for sad, 3 for angry 

// BLE
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pWifiCharacteristic;
BLECharacteristic *pGpsCharacteristic;
bool deviceConnected = false;

// ===== Parsed「Wi-Fi & GPS info」 =====
String storedSSID = "";
String storedPASS = "";
float storedLAT = 0.0; // stored is for the device owner's location
float storedLON = 0.0;
// The pair's location
float grabedLAT = 0.0; // grabed is for the pair's location
float grabedLON = 0.0;

// ===== Secreted「MQTT info」 =====
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = SECRET_MQTTSERVER;
const int   mqtt_port     = SECRET_MQTTPORT;

Preferences preferences;

WiFiClient espClient;
PubSubClient client(espClient);

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

// ===== Load storaged Info function =====
void loadStoredWiFi() {
  preferences.begin("wifi-config", true); // Open in read-only mode
  storedSSID = preferences.getString("ssid", "");  // The second argument is the default
  storedPASS = preferences.getString("password", "");
  preferences.end();

  Serial.printf("Loaded WiFi: %s,%s\n", storedSSID.c_str(), storedPASS.c_str());
}
void loadStoredGPS() {
  preferences.begin("gps-data", true); // Open in read-only mode
  storedLAT = preferences.getFloat("lat", 0.0);   // The second argument is the default
  storedLON = preferences.getFloat("lng", 0.0);
  preferences.end(); // close Preferences

  Serial.printf("Loaded GPS: Latitude=%.6f, Longitude=%.6f\n", storedLAT, storedLON);
}

// ===== WiFi connecting function =====
void WiFi_Connector(){
  int counter = 20;
  // Clear old connections
  WiFi.disconnect(true);
  yield();
  delay(100);
  
  // Set STA mode and enable connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(storedSSID.c_str(), storedPASS.c_str());
  
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED && counter > 0) {
    counter--;
    delay(250);
    Serial.print(".");
    yield(); // 喂狗并处理后台任务
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\Wifi Connection Successful!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
  } else{
    Serial.println("\nWifi Connection Failed x");
  }
}

// ===== MQTT things =====
// Function for Connect to MQTT and Sub MQTT Topics
void reconnectMQTT() {
  // If the WiFi is disconnected, reconnect the WiFi first
  if (WiFi.status() != WL_CONNECTED) {
    WiFi_Connector();
  }
  
  // Loop attempts to connect to MQTT Broker
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Generate a random client ID
    String clientId = "Device_ESP32_";
    clientId += String(random(0xffff), HEX);
    
    // Attempt connection
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      // After the connection is successful, subscribe to the theme of Device B
      if (client.subscribe(mqtt_topic_B_LAT)) {
        Serial.print("Subscribed to topic: ");
        Serial.println(mqtt_topic_B_LAT);
      } else {
        Serial.print("Failed to subscribe to topic: ");
        Serial.println(mqtt_topic_B_LAT);
      }
      if (client.subscribe(mqtt_topic_B_LON)) {
        Serial.print("Subscribed to topic: ");
        Serial.println(mqtt_topic_B_LON);
      } else {
        Serial.print("Failed to subscribe to topic: ");
        Serial.println(mqtt_topic_B_LON);
      }
      if (client.subscribe(mqtt_topic_B_MODE)) {
        Serial.print("Subscribed to topic: ");
        Serial.println(mqtt_topic_B_MODE);
      } else {
        Serial.print("Failed to subscribe to topic: ");
        Serial.println(mqtt_topic_B_MODE);
      }
      if (client.subscribe(mqtt_topic_B_STATUS)) {
        Serial.print("Subscribed to topic: ");
        Serial.println(mqtt_topic_B_STATUS);
      } else {
        Serial.print("Failed to subscribe to topic: ");
        Serial.println(mqtt_topic_B_STATUS);
      }
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" - trying again in 5 seconds");
      // 分段延迟并喂狗
      unsigned long startTime = millis();
      while (millis() - startTime < 5000) {
        delay(500);  // 每次延迟500ms
        yield();      // 处理后台任务并喂狗
      }
    }
  }
}

// Fuction for Sending location info through 
void sendmqtt_location() {
  // Constructing time information in JSON format (fixed here as "15mins")
  char LAT_message[50];
  sprintf(LAT_message, "%.5f", storedLAT);
  
  // Post A message to the topic of Device A (for Device B subscription)
  if (client.publish(mqtt_topic_A_LAT, LAT_message)) {
    Serial.println("Message upload successfully!「MQTT-LAT」");
  } else {
    Serial.println("Failed to update info.「MQTT-LAT」");
  }
  char LON_message[50];
  sprintf(LON_message, "%.5f", storedLON);
  
  // Post A message to the topic of Device A (for Device B subscription)
  if (client.publish(mqtt_topic_A_LON, LON_message)) {
    Serial.println("Message upload successfully!「MQTT-LON」");
  } else {
    Serial.println("Failed to update info.「MQTT-LON」");
  }
}

// Functions for sending emotions
void sendmqtt_happy() {
  // Constructing time information in JSON format (fixed here as "15mins")
  char emotion_message[10];
  sprintf(emotion_message, "happy");
  
  // Post A message to the topic of Device A (for Device B subscription)
  if (client.publish(mqtt_topic_A_MODE, emotion_message)) {
    Serial.println("Message upload successfully!「MQTT-happy」");
  } else {
    Serial.println("Failed to update info.「MQTT-happy」");
  }
}
void sendmqtt_sad() {
  // Constructing time information in JSON format (fixed here as "15mins")
  char emotion_message[10];
  sprintf(emotion_message, "sad");
  
  // Post A message to the topic of Device A (for Device B subscription)
  if (client.publish(mqtt_topic_A_MODE, emotion_message)) {
    Serial.println("Message upload successfully!「MQTT-sad」");
  } else {
    Serial.println("Failed to update info.「MQTT-sad」");
  }
}
void sendmqtt_angry() {
  // Constructing time information in JSON format (fixed here as "15mins")
  char emotion_message[10];
  sprintf(emotion_message, "angry");
  
  // Post A message to the topic of Device A (for Device B subscription)
  if (client.publish(mqtt_topic_A_MODE, emotion_message)) {
    Serial.println("Message upload successfully!「MQTT-angry」");
  } else {
    Serial.println("Failed to update info.「MQTT-angry」");
  }
}
void sendmqtt_normal() {
  // Constructing time information in JSON format (fixed here as "15mins")
  char emotion_message[10];
  sprintf(emotion_message, "normal");
  
  // Post A message to the topic of Device A (for Device B subscription)
  if (client.publish(mqtt_topic_A_MODE, emotion_message)) {
    Serial.println("Message upload successfully!「MQTT-normal」");
  } else {
    Serial.println("Failed to update info.「MQTT-normal」");
  }
}
// For Reading STATUS
void sendmqtt_STATUS_NoEvent() {
  // Constructing time information in JSON format (fixed here as "15mins")
  char STATUS_message[10];
  sprintf(STATUS_message, "NoEvent");
  
  // Post A message to the topic of Device A (for Device B subscription)
  if (client.publish(mqtt_topic_A_STATUS, STATUS_message)) {
    Serial.println("Message upload successfully!「MQTT-STATUS-NoEvent」");
  } else {
    Serial.println("Failed to update info.「MQTT-STATUS-NoEvent」");
  }
}
void sendmqtt_STATUS_Read() {
  // Constructing time information in JSON format (fixed here as "15mins")
  char STATUS_message[10];
  sprintf(STATUS_message, "Read");
  
  // Post A message to the topic of Device A (for Device B subscription)
  if (client.publish(mqtt_topic_A_STATUS, STATUS_message)) {
    Serial.println("Message upload successfully!「MQTT-STATUS-READ」");
  } else {
    Serial.println("Failed to update info.「MQTT-STATUS-READ」");
  }
}
void sendmqtt_STATUS_UnRead() {
  // Constructing time information in JSON format (fixed here as "15mins")
  char STATUS_message[10];
  sprintf(STATUS_message, "UnRead");
  
  // Post A message to the topic of Device A (for Device B subscription)
  if (client.publish(mqtt_topic_A_STATUS, STATUS_message)) {
    Serial.println("Message upload successfully!「MQTT-STATUS-UnREAD」");
  } else {
    Serial.println("Failed to update info.「MQTT-STATUS-UnREAD」");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // 将 payload 转换为字符串（注意添加终止符）
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0'; // 确保字符串正确终止

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // 处理 LAT 主题
  if (strcmp(topic, mqtt_topic_B_LAT) == 0) {
    grabedLAT = atof(message); // 字符串转浮点数
    Serial.printf("Updated pair LAT: %.6f\n", grabedLAT);
  }
  // 处理 LON 主题
  if (strcmp(topic, mqtt_topic_B_LON) == 0) {
    grabedLON = atof(message); // 字符串转浮点数
    Serial.printf("Updated pair LON: %.6f\n", grabedLON);
  }
  // 处理 MODE 主题
  if (strcmp(topic, mqtt_topic_B_MODE) == 0) {
    // 判断情绪类型并触发对应动作
    if (strcmp(message, "happy") == 0) {
      Serial.println("「Emotion」Pair is HAPPY! Trigger rainbow effect");
      // 首先发送"UnRead「Status」"信息
      ReadOrNot = false;
      pairStatus = 1;
      sendmqtt_STATUS_UnRead();
      // 振动提醒用户，进行“Aligned”的相关操作
    } else if (strcmp(message, "sad") == 0) {
      Serial.println("「Emotion」Pair is SAD. Show blue slow pulse");
      // 首先发送"UnRead「Status」"信息
      ReadOrNot = false;
      pairStatus = 2;
      sendmqtt_STATUS_UnRead();
      // 振动提醒用户，进行“Aligned”的相关操作
    } else if (strcmp(message, "angry") == 0) {
      Serial.println("「Emotion」Pair is ANGRY! Red alert");
      // 首先发送"UnRead「Status」"信息
      ReadOrNot = false;
      pairStatus = 3;
      sendmqtt_STATUS_UnRead();
      // 振动提醒用户，进行“Aligned”的相关操作
    } else if (strcmp(message, "normal") == 0) {
      Serial.println("「Emotion」Pair back to NORMAL");
      // 保持"NoEvent「Status」"信息
      sendmqtt_STATUS_NoEvent();
    } else {
      Serial.println("Unknown emotion received");
    }
  }
  // 处理 STATUS 状态
  else if (strcmp(topic, mqtt_topic_B_STATUS) == 0) {
    // 判断Event Status并触发对应动作
    if (strcmp(message, "NoEvent") == 0) {
      Serial.println("「STATUS」There is no Event right now.");
      // 这里可以添加 LED 控制逻辑
    } else if (strcmp(message, "Read") == 0) {
      Serial.println("「STATUS」Your Pair have found your Emotion!");
      // 这里可以添加 LED 控制逻辑，振动等，表明对方收到了
      // 既然对方已经收到了，将自己的情绪转换为“normal”
      sendmqtt_normal();
    } else if (strcmp(message, "UnRead") == 0) {
      Serial.println("「STATUS」You've sent your emotion, please wait!");
      // 此处可以没有操作，也可以添加LED等待的显示效果,例如呼吸灯
    } else {
      Serial.println("Unknown Status received");
    }
  }

  // 添加数值有效性检查
  if (isnan(grabedLAT) || isnan(grabedLON)) {
    Serial.println("Warning: Received invalid GPS coordinates!");
    grabedLAT = 0.0;
    grabedLON = 0.0;
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
      sendmqtt_location();
    }
  }
};

// ===== Compass init pls put at the end of the myConfig.h =====
void Compass_BLE_WiFi_MQTT_init(){
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

  loadStoredWiFi();
  WiFi_Connector();

  // If the wifi is successfully connected, setup the MQTT
  client.setServer(mqtt_server, mqtt_port);
  // Set the MQTT callback function to be triggered when a subscription message is received
  client.setCallback(callback);

  if (!client.connected()) {
    reconnectMQTT();
  }
  delay(500);

  // Load the GPS info and upload to the MQTT
  loadStoredGPS();
  sendmqtt_location();
}

