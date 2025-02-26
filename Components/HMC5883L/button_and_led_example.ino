#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 24
#define LED_PIN 15         //
#define BUTTON_PIN 4     

volatile bool buttonPressed = false; // press flag
unsigned long lastInterruptTime = 0;
int pressCount = 0;                  // count the number of press

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    Serial.begin(9600);

    pinMode(BUTTON_PIN, INPUT_PULLUP); // poll-up resistance involved
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); 

    pixels.begin();
    pixels.setBrightness(10);
    pixels.clear();
    pixels.show();

    // Interrupt
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);
}

void loop() {
    if (buttonPressed) {
        buttonPressed = false; // clear flag
        
        if (pressCount == 1) {
            lightUpPixels(0, 23, 255, 255, 255); 
        } else if (pressCount == 2) {
            lightUpPixels(4, 8, 255, 255, 0);
        } else if (pressCount == 3) {
            lightUpPixels(12, 16, 255, 255, 0); 
        } else if (pressCount == 4){
            pixels.clear();
            pixels.show();
            pressCount = 0; 
        }    
    }
}

// button processed with interrupt
void handleButtonPress() {
    unsigned long currentTime = millis();
    if (currentTime - lastInterruptTime > 200) { // debounce
        buttonPressed = true;
        pressCount++;
        lastInterruptTime = currentTime;
    }
}

// light the led
void lightUpPixels(int start, int end, int red, int green, int blue) {
    for (int i = start; i <= end; i++) {
        pixels.setPixelColor(i, pixels.Color(red, green, blue));
    }
    pixels.show();
}
