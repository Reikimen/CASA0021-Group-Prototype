#include <WiFi.h>
#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Preferences.h>
#include <QMC5883LCompass.h>
#include <math.h>
#include <Adafruit_NeoPixel.h>
#include <esp_sleep.h>  // low power mode lib

#include "arduino_secrets.h"
#include "myConfig.h"
#include "myCompass.h"
#include "myLEDandBtns.h"

// Global Variable for main program
int loactionCount = 50; // 50 means 5s for once GPS location update to MQTT
int emotionDebugCount = 300;
bool foundOrNot = true;
bool sendOrNot = true;
bool RightDirOrNot = true;

// ===== Main Program =====c:\Users\JiaYing\Downloads\arduino_secrets.h
void setup() {
  Serial.begin(115200);
  calibrateCompass(); // 每次启动时进行校准
  // waitForNorthCalibration();  // First detect azimuth == 0 before proceeding 
  Compass_BLE_WiFi_MQTT_init();
  init_LEDandBtn();
}

void loop() {
  // All operations of BLE, Wi-Fi & GPS info update are handled by callbacks
  // Check MQTT connections
  reconnectMQTT();// 如果Wi-Fi没连接就先连接Wi-Fi，没连接MQTT就连接MQTT
  
  // Keep MQTT client background tasks
  client.loop();
  
  // Allways updating the GPS Info
  if (loactionCount >= 0){
    loactionCount--;
  }else{
    loactionCount = 200; // 除了第一次是开启后5s发送，之后都是20s发送一次，减少播客占用
    sendmqtt_location(); // the function for sending MQTT GPS message based on the mode you set in mgConfig.h
  }

  // 按下某个按钮触发某个心情事件，发送MQTT
  // 以下为每30s模拟按下一次按钮
  // if (emotionDebugCount >= 0){
  //   emotionDebugCount--;
  // }else{
  //   emotionDebugCount = 300;
  //   sendmqtt_angry(); // the function for sending MQTT message based on the mode you set in mgConfig.h
  // }
  // 以下为各个心情事件的函数名称
  // sendmqtt_happy();
  // sendmqtt_sad();
  // sendmqtt_angry();
  // sendmqtt_normal();
  // delay(500);

  if (startPressed){
    startPressed = false;
    foundOrNot = false;
    RightDirOrNot = false;
    startPressCount++;
    lightUpPixels(0, 23, 255, 255, 255);  
    Serial.println("START: Lights ON");
    delay(300);
  }

  if (!foundOrNot){
      // waitForNorthCalibration();  // First detect azimuth == 0 before proceeding
      if (!RightDirOrNot){
        double bearing = calculateBearing(storedLAT, storedLON, grabedLAT, grabedLON);

        compass.read();
          
        int azimuth = compass.getAzimuth();
        char myArray[3];
        compass.getDirection(myArray, azimuth);

        Serial.print("Azimuth: ");
        Serial.print(azimuth);
        Serial.print(" | Target: ");
        Serial.print(bearing);
        Serial.print(" | Direction: ");
        Serial.print(myArray[0]);
        Serial.print(myArray[1]);
        Serial.print(myArray[2]);
        Serial.println();

        // Check if azimuth is within ±5 degrees of the target
        if (isTargetAzimuthReached(azimuth, bearing)) {
          // 如果对准了做以下动作
          lightUpPixels(0, 23, 0, 255, 0);  
          delay(300);
          pixels.clear();
          pixels.show();
          // break;  // Exit loop
          Serial.println("Send your emotion「LED」, 1 for happy, 2 for sad, 3 for angry");
          RightDirOrNot = true;
          sendOrNot = false;
        }
        else {
          updateNeoPixels(azimuth, bearing);
        }
      }

      if (!sendOrNot) {
        if (button1Pressed) {
          button1Pressed = false;
          Serial.println("SAD");
          sendmqtt_sad();
          lightUpPixels(0, 23, 0, 0, 255);  
          delay(300);
          pixels.clear();
          pixels.show();
          sendOrNot = true;
          foundOrNot = true;
        } else if (button2Pressed) {
          button2Pressed = false;
          //lightUpPixels(12, 16, 255, 255, 0);  
          Serial.println("HAPPY");
          sendmqtt_happy();
          lightUpPixels(0, 23, 0, 255, 0);  
          delay(300);
          pixels.clear();
          pixels.show();
          sendOrNot = true;
          foundOrNot = true;
        } else if (button3Pressed) {
          button3Pressed = false;
          //lightUpPixels(18, 24, 255, 255, 0);  
          Serial.println("ANGRY");
          sendmqtt_angry();
          lightUpPixels(0, 23, 255, 0, 0);  
          delay(300);
          pixels.clear();
          pixels.show();
          sendOrNot = true;
          foundOrNot = true;
        }
      }
    }
    
    // else if (startPressCount == 2){
    //   Serial.println("Entering Deep Sleep...");
    //   delay(500); 
    //   enterDeepSleep(); 
    //   startPressCount = 0;
    // }
  

  if (!ReadOrNot){ // 如果还没有阅读对面的情绪信息，需要一直旋转罗盘直到对准对面
    // Serial.println("Pls use the compase to find the dir.");

    double bearing = calculateBearing(storedLAT, storedLON, grabedLAT, grabedLON);

    compass.read();
      
    int azimuth = compass.getAzimuth();
    char myArray[3];
    compass.getDirection(myArray, azimuth);

    Serial.print("Azimuth: ");
    Serial.print(azimuth);
    Serial.print(" | Target: ");
    Serial.print(bearing);
    Serial.print(" | Direction: ");
    Serial.print(myArray[0]);
    Serial.print(myArray[1]);
    Serial.print(myArray[2]);
    Serial.println();

    // Check if azimuth is within ±5 degrees of the target
    if (isTargetAzimuthReached(azimuth, bearing)) {
      // 如果对准了做以下动作
      sendmqtt_STATUS_Read(); //发布：已经收到对方情绪
      ReadOrNot = true; // 将全局变量阅读状态改为已读
      // 再根据 pairStatus for normal, 1 for happy, 2 for sad, 3 for angry，显示对应的颜色
      Serial.println("Indicate your pair's emotion「LED」, 1 for happy, 2 for sad, 3 for angry");
      if (pairStatus==1){
        lightUpPixels(0, 23, 100, 0, 100);
      }
      else if (pairStatus==2){
        lightUpPixels(0, 23, 100, 100, 0);
      }
      else if (pairStatus==3){
        lightUpPixels(0, 23, 0, 100, 100);
      }
      else{
        // 一般没有其他情况，如有，则检查MQTT部分代码
      }
      delay(3000);
      pixels.clear();
      pixels.show();
    }
    else {
      // 如果没有对准，使用灯光指示
      updateNeoPixels(azimuth, bearing);
    }
  }

  delay(100);  // Loop every 0.1 seconds
}
