#include <WiFi.h>
#include <PubSubClient.h>
#include "arduino_secrets.h" 

/*
  Make sure to fill in your sensitive information in the arduino_secrets.h, such as:
  #define SECRET_SSID "yours-ssid"
  #define SECRET_PASS "yours-password"
  #define SECRET_MQTTUSER "mqtt-user-name"
  #define SECRET_MQTTPASS "mqtt-password"
  #define SECRET_MQTTSERVER "yoursmqtt.org"
  #define SECRET_MQTTPORT "114514"
*/

const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = SECRET_MQTTSERVER;
const int   mqtt_port     = SECRET_MQTTPORT;

WiFiClient espClient;
PubSubClient client(espClient);

// 定义 MQTT 主题：
// Device A 用于上传时间信息；Device B 将订阅该主题
const char* mqtt_topic_A = "student/ucfnwy2/DeviceA/time";
// 同时 Device A 订阅 Device B 发布的时间信息
const char* mqtt_topic_B = "student/ucfnwy2/DeviceB/time";

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 设置设备主机名（ESP32 支持此函数）
  WiFi.setHostname("DeviceA_ESP32");
  
  connectToWiFi();
  
  client.setServer(mqtt_server, mqtt_port);
  // 设置 MQTT 回调函数，当收到订阅的消息时触发
  client.setCallback(callback);
  
  Serial.println("Setup complete");
}

void loop() {
  // 检查 MQTT 连接
  if (!client.connected()) {
    reconnectMQTT();
  }
  
  // 检查 WiFi 连接
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  
  // 保持 MQTT 客户端的后台任务
  client.loop();
  
  // 发送时间信息
  sendmqtt();
  
  Serial.println("Sent a time info message");
  delay(10000);  // 每 10 秒发送一次消息
}

void sendmqtt() {
  // 构造 JSON 格式的时间信息（此处固定为 "15mins"）
  char time_message[50];
  sprintf(time_message, "{\"time\": \"15mins\"}");
  
  // 发布消息到 Device A 的主题（供 Device B 订阅）
  if (client.publish(mqtt_topic_A, time_message)) {
    Serial.println("Time info updated successfully!");
  } else {
    Serial.println("Failed to update time info.");
  }
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  // 如果 WiFi 断开，则先重新连接 WiFi
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  
  // 循环尝试连接 MQTT Broker
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // 生成随机客户端 ID
    String clientId = "DeviceA_ESP32_";
    clientId += String(random(0xffff), HEX);
    
    // 尝试连接
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      // 连接成功后，订阅 Device B 的主题
      if (client.subscribe(mqtt_topic_B)) {
        Serial.print("Subscribed to topic: ");
        Serial.println(mqtt_topic_B);
      } else {
        Serial.print("Failed to subscribe to topic: ");
        Serial.println(mqtt_topic_B);
      }
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" - trying again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // 当接收到订阅主题的消息时，通过串口打印出来
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
