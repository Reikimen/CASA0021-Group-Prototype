#include <Wire.h>
#include <QMC5883LCompass.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     2   // Control pin for NeoPixel on ESP32
#define NUMPIXELS   12  // Number of LEDs in the strip
#define DELAYVAL 500

QMC5883LCompass compass;
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

const int targetAngle = 150; // Target angle
#define SLOW_BLINK_DELAY 100    // Slow blink delay (milliseconds)
#define FAST_BLINK_DELAY 50     // Fast blink delay (milliseconds)

/************ Angle Calculation ************/
int getCurrentAzimuth() {
  compass.read(); // Read compass data
  return compass.getAzimuth(); // Return current azimuth
}

int calculateAngleDiff(int currentAzimuth) {
  return abs(currentAzimuth - targetAngle); // Calculate the difference from the target angle
}

void controlLED(int angleDiff) {
  if (angleDiff > 50) {
    // Far from target, red slow blink
    pixels.setPixelColor(9, pixels.Color(255, 0, 0)); // Red
    pixels.show();
    delay(SLOW_BLINK_DELAY);
    pixels.clear();
    pixels.show();
    delay(SLOW_BLINK_DELAY);
  } else if (angleDiff >= 10 && angleDiff <= 30) {
    // Approaching target, yellow fast blink
    pixels.setPixelColor(9, pixels.Color(255, 255, 0)); // Yellow
    pixels.show();
    delay(FAST_BLINK_DELAY);
    pixels.clear();
    pixels.show();
    delay(FAST_BLINK_DELAY);
  } else if (angleDiff < 10) {
    // Reached target, green steady on
    pixels.setPixelColor(9, pixels.Color(0, 255, 0)); // Green
    pixels.show();
  }
}
/************ Angle Calculation ************/

/************ LED State ************/


void setHappyState() {
  // Clear current pixels
  pixels.clear();
  
  // Set LED strip to green with a fading effect
  // Gradually increase brightness
  for (int brightness = 0; brightness <= 255; brightness += 5) { // Gradually increase brightness
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, brightness, 0)); // Green
    }
    pixels.show();
    delay(30); // Control gradual change speed
  }
  
  for (int brightness = 255; brightness >= 0; brightness -= 5) { // Gradually decrease brightness
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, brightness, 0)); // Green
    }
    pixels.show();
    delay(30); // Control gradual change speed
  }
}

void setSadState() {
  pixels.clear();
  // Set LED strip to blue for sad state with fading effect
  // Gradually increase brightness
  for (int brightness = 0; brightness <= 255; brightness += 5) { // Gradually increase brightness
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, brightness)); // Blue
    }
    pixels.show();
    delay(30); // Control gradual change speed
  }
  
  for (int brightness = 255; brightness >= 0; brightness -= 5) { // Gradually decrease brightness
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, brightness)); // Blue
    }
    pixels.show();
    delay(30); // Control gradual change speed
  }
}

void setAngryState() {
  pixels.clear();
  // Set LED strip to red for angry state with fading effect
  // Gradually increase brightness
  for (int brightness = 0; brightness <= 255; brightness += 5) { // Gradually increase brightness
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(brightness, 0, 0)); // Red
    }
    pixels.show();
    delay(30); // Control gradual change speed
  }
  
  for (int brightness = 255; brightness >= 0; brightness -= 5) { // Gradually decrease brightness
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(brightness, 0, 0)); // Red
    }
    pixels.show();
    delay(30); // Control gradual change speed
  }
}
/************ LED State ************/

void setup() {
  Serial.begin(9600);
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize compass
  compass.init(); 
  Serial.println("QMC5883L Initialized");

  // Initialize LED strip
  pixels.begin();
  pixels.setBrightness(50);

  // Initialize LED strip, set the 10th LED (index 9) to red
  pixels.setPixelColor(9, pixels.Color(255, 0, 0)); // Red
  pixels.show(); // Update LED state

  // Print debug information
  Serial.println("LED initialized. Index 9 is set to red.");
}


void loop() {
  compass.read(); // Read compass data
  int currentAzimuth = compass.getAzimuth(); // Get current azimuth
  int angleDiff = calculateAngleDiff(currentAzimuth); // Calculate angle difference
  
  // Print debug information
  Serial.print("Current Azimuth: ");
  Serial.println(currentAzimuth);
  // Serial.print("Target Angle: ");
  // Serial.println(targetAngle);
  Serial.print("Angle Difference: ");
  Serial.println(angleDiff);
  controlLED(angleDiff);
  delay(1000);

  // setHappyState();
  // delay(3000); // Update frequency
  // setSadState();
  // delay(3000); // Update frequency
  // setAngryState();
  // delay(3000); // Update frequency
}