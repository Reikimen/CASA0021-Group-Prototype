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
//#include "myCompass.h"



//#define LED_PIN 15          // WS2812B 
#define BUTTON_PIN1 4       // button1
#define BUTTON_PIN2 18      // button2
#define BUTTON_PIN3 19      // button3
#define BUTTON_START 23     // Start button
#define NUMPIXELS 24 
#define LED_PIN 15


Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;
volatile bool startPressed = false;
volatile int startPressCount = 0; // record start button pressed time

// Global Variable for main program
int loactionCount = 50; // 50 means 5s for once GPS location update to MQTT
int emotionDebugCount = 300;

// ===== Main Program =====c:\Users\JiaYing\Downloads\arduino_secrets.h
void setup() {
  Serial.begin(115200);
  calibrateCompass(); // 每次启动时进行校准
  waitForNorthCalibration();  // First detect azimuth == 0 before proceeding 
  Compass_BLE_WiFi_MQTT_init();

  // pull-up resistance
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);
  pinMode(BUTTON_PIN3, INPUT_PULLUP);
  pinMode(BUTTON_START, INPUT_PULLUP);

  pixels.begin();
  pixels.setBrightness(50);
  pixels.clear();
  pixels.show();

  // 4 buttons interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN1), [](){ handleButtonPress(1); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN2), [](){ handleButtonPress(2); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN3), [](){ handleButtonPress(3); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_START), [](){ handleButtonPress(4); }, FALLING);


  Serial.println("Starting Compass Reading...");
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
    startPressCount++;
    if (startPressCount == 1){
      lightUpPixels(0, 23, 255, 255, 255);  
      Serial.println("START: Lights ON");
      waitForNorthCalibration();  // First detect azimuth == 0 before proceeding
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
        Serial.println("Indicate your emotion「LED」, 1 for happy, 2 for sad, 3 for angry");

        if (button1Pressed) {
          button1Pressed = false;
          lightUpPixels(0, 23, 255, 255, 0);  
          Serial.println("SAD");
          sendmqtt_sad();
        } else if (button2Pressed) {
          button2Pressed = false;
          //lightUpPixels(12, 16, 255, 255, 0);  
          Serial.println("HAPPY");
          sendmqtt_happy();
        } else if (button3Pressed) {
          button3Pressed = false;
          //lightUpPixels(18, 24, 255, 255, 0);  
          Serial.println("ANGRY");
          sendmqtt_angry();
        }
      }
      else {
        updateNeoPixels(azimuth, bearing);
      }
      delay(250);
    } else if (startPressCount == 2){
      Serial.println("Entering Deep Sleep...");
      delay(500); 
      enterDeepSleep(); 
    }
  }

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
    }
    else {
      updateNeoPixels(azimuth, bearing);
    }
  }

  delay(100);  // Loop every 0.1 seconds
}

// turn on the light
void updateNeoPixels(int azimuth, double target) {
    pixels.clear();  

    int ledCount;
    uint32_t color;
    if (target > 0 ){
      if (azimuth >= 0) {  
          ledCount = map(azimuth, 0, 180, 1, NUMPIXELS / 2);  
          color = pixels.Color(0, 255, 0);  // green
          pixels.setPixelColor(0, color); 
          for (int i = 1; i < ledCount; i++) {
              pixels.setPixelColor(NUMPIXELS - i, color);
          }
      } else {  
          ledCount = map(abs(azimuth), 0, 180, 1, NUMPIXELS / 2);  
          color = pixels.Color(255, 0, 0);  // red
          for (int i = 0; i < ledCount; i++) {
              pixels.setPixelColor(i, color);
          }
      }
    } else if (target < 0 ){
      if (azimuth >= 0) {  
          ledCount = map(azimuth, 0, 180, 1, NUMPIXELS / 2);  
          color = pixels.Color(255, 0, 0);  // red
          for (int i = 0; i < ledCount; i++) {
              pixels.setPixelColor(i, color);
          }
          
      } else {  
          ledCount = map(abs(azimuth), 0, 180, 1, NUMPIXELS / 2);  
          color = pixels.Color(0, 255, 0);  // green
          pixels.setPixelColor(0, color); 
          for (int i = 1; i < ledCount; i++) {
              pixels.setPixelColor(NUMPIXELS - i, color);
          }
          
      }
    }
      

    pixels.show();
}


void handleButtonPress(int buttonID) {
    static unsigned long lastInterruptTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastInterruptTime > 200) {  
        switch (buttonID) {
            case 1: button1Pressed = true; break;
            case 2: button2Pressed = true; break;
            case 3: button3Pressed = true; break;
            case 4: startPressed = true; break;
        }
        lastInterruptTime = currentTime;
    }
}


void lightUpPixels(int start, int end, int red, int green, int blue) {
    pixels.clear();
    for (int i = start; i <= end; i++) {
        pixels.setPixelColor(i, pixels.Color(red, green, blue));
    }
    pixels.show();
}

void lightUpRainbow() {
    pixels.clear();

    for (int i = 0; i < NUMPIXELS; i++) {
        int hue = map(i, 0, NUMPIXELS - 1, 0, 255); // reflect in HSV color
        pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(hue * 256))); 
    }

    pixels.show();
}

// sleep mode
void enterDeepSleep() {
    pixels.clear();
    pixels.show();

    Serial.println("ESP32 is going to sleep...");
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_START, 23); // Wake on button press (LOW)

    esp_deep_sleep_start();
}


