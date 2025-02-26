#include <Adafruit_NeoPixel.h>
#include <esp_sleep.h>  // low power mode lib
#include <QMC5883LCompass.h>

#define NUMPIXELS 24  
#define LED_PIN 15          // WS2812B 
#define BUTTON_PIN1 4       // button1
#define BUTTON_PIN2 18      // button2
#define BUTTON_PIN3 19      // button3
#define BUTTON_START 23     // Start button

volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;
volatile bool startPressed = false;
volatile int startPressCount = 0; // record start button pressed time

QMC5883LCompass compass;
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
int targetAngle; 

void setup() {
    Serial.begin(9600);

    compass.init();
    compass.setCalibrationOffsets(-708.00, -1408.00, 895.00);
    compass.setCalibrationScales(1.33, 0.73, 1.14);

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

    // random angle
    targetAngle = random(-180, 181);
    Serial.print("Target Angle: ");
    Serial.println(targetAngle);

    Serial.println("Starting Compass Reading...");
}

void loop() {
    compass.read();
    int azimuth = compass.getAzimuth();

    Serial.print("Azimuth: ");
    Serial.print(azimuth);
    Serial.print(" | Target: ");
    Serial.println(targetAngle);

    if (isTargetAzimuthReached(azimuth, targetAngle)) {
        lightUpPixels(0, NUMPIXELS - 1, 255, 255, 255); // turn white light
        enterDeepSleep();
    } else {
        updateNeoPixels(azimuth);
    }

    // button pressed
    if (button1Pressed) {
        button1Pressed = false;
        lightUpPixels(4, 8, 255, 255, 0);  
        Serial.println("SAD");
    } 
    if (button2Pressed) {
        button2Pressed = false;
        lightUpPixels(12, 16, 255, 255, 0);  
        Serial.println("HAPPY");
    } 
    if (button3Pressed) {
        button3Pressed = false;
        lightUpPixels(18, 24, 255, 255, 0);  
        Serial.println("ANGRY");
    }
    
    if (startPressed) {
        startPressed = false;
        startPressCount++;

        if (startPressCount == 1) {
            lightUpPixels(0, 23, 255, 255, 255);  
            Serial.println("START: Lights ON");
        } else if (startPressCount == 2) {
            Serial.println("Entering Deep Sleep...");
            delay(500); 
            enterDeepSleep();  
        }
    }

    delay(250);  
}

// check angle reached
bool isTargetAzimuthReached(int azimuth, int angle) {
    if (abs(azimuth - angle) <= 5) {
        Serial.println("Target azimuth reached! Exiting loop.");
        return true; 
    }
    return false;
}

// turn on the light
void updateNeoPixels(int azimuth) {
    pixels.clear();  

    int ledCount;
    uint32_t color;

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

// sleep mode
void enterDeepSleep() {
    pixels.clear();
    pixels.show();
    
    Serial.println("ESP32 is going to sleep...");
    esp_deep_sleep_start();  
}
