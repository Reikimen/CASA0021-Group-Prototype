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

void init_LEDandBtn(){
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
}
