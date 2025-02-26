#include <WiFi.h>
#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Preferences.h>
#include <QMC5883LCompass.h>
#include <math.h>

#include "arduino_secrets.h"
#include "myConfig.h"
#include "myCompass.h"

// Global Variable for main program
int loactionCount = 50; // 50 means 5s for once GPS location update to MQTT
int emotionDebugCount = 300;

// ===== Main Program =====c:\Users\JiaYing\Downloads\arduino_secrets.h
void setup() {
  Serial.begin(115200);
  Compass_init();
  calibrateCompass(); // 每次启动时进行校准
  Serial.println("Starting Compass Reading...");
}

void loop() {
  // All operations of BLE, Wi-Fi & GPS info update are handled by callbacks
  // Check MQTT connections
  if (!client.connected()) {
    reconnectMQTT();
  }
  
  // Keep MQTT client background tasks
  client.loop();
  
  // Allways updating the GPS Info
  if (loactionCount >= 0){
    loactionCount--;
  }else{
    loactionCount = 50;
    sendmqtt_location(); // the function for sending MQTT GPS message based on the mode you set in mgConfig.h
  }

  // 按下某个按钮触发某个心情事件，发送MQTT
  // 以下为每30s模拟按下一次按钮
  if (emotionDebugCount >= 0){
    emotionDebugCount--;
  }else{
    emotionDebugCount = 300;
    sendmqtt_happy(); // the function for sending MQTT message based on the mode you set in mgConfig.h
  }
  // 以下为各个心情事件的函数名称
  // sendmqtt_happy();
  // sendmqtt_sad();
  // sendmqtt_angry();
  // sendmqtt_normal();
  // delay(500);

  if (!ReadOrNot){ // 如果还没有阅读对面的情绪信息，需要一直旋转罗盘直到对准对面
    delay(8000); // 本应是一个关于旋转角的if嵌套，此处先用delay模拟
    // 如果没有对准，使用灯光指示
    // 如果对准了做以下动作
    sendmqtt_STATUS_Read(); //发布：已经收到对方情绪
    ReadOrNot = true; // 将阅读状态改为已读
    // 再根据 pairStatus for normal, 1 for happy, 2 for sad, 3 for angry，显示对应的颜色
  }

  delay(100);  // Loop every 0.1 seconds
}
