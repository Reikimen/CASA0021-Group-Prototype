#include "arduino_secrets.h" 
#include "myConfig.h"

// ===== Main Program =====
void setup() {
  Serial.begin(115200);
  Compass_init();
}

void loop() {
  // Keep an empty loop, all operations are handled by callbacks
    // 检查 MQTT 连接
  if (!client.connected()) {
    reconnectMQTT();
  }
  
  // 保持 MQTT 客户端的后台任务
  client.loop();
  
  // 发送时间信息
  sendmqtt();
  
  Serial.println("Sent a time info message");
  delay(10000);  // 每 10 秒发送一次消息
}
