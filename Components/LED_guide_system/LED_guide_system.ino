//**************************************************//
// Compass-guided LED Direction Indicator
// Function: Uses QMC5883L compass to control NeoPixel LEDs
//          for direction indication
//
// Known Issues:
// 1. Compass calibration incomplete
// 2. LED indices and RGB values need adjustment
// 3. Compass sensor and LED strip must remain stationary
//    relative to each other during testing
//**************************************************//

#include <QMC5883LCompass.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     2   // Control pin for NeoPixel on ESP32
#define NUMPIXELS   17  // Number of LEDs in the strip

QMC5883LCompass compass;
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Define indices for the 4 direction indicator LEDs
const int LED_INDICES[] = {0, 6, 11, 16}; 

// Blinking and angle threshold constants
#define FAST_BLINK_DELAY 100    // Fast blink delay (ms)
#define SLOW_BLINK_DELAY 500    // Slow blink delay (ms)
#define ANGLE_THRESHOLD_NEAR 30  // Threshold for "near target" angle
#define ANGLE_THRESHOLD_REACHED 5  // Threshold for "reached target" angle

void setup() {
  Serial.begin(9600);
  
  // Initialize compass
  compass.init();
  compass.setCalibrationOffsets(-708.00, -1408.00, 895.00);
  compass.setCalibrationScales(1.33, 0.73, 1.14);
  
  // Initialize LED strip
  pixels.begin();
  pixels.setBrightness(50);

  Serial.println("Starting Compass Reading...");
}

void loop() {
  waitForNorthCalibration();

  int targetAngle = 150; // Target angle
  Serial.print("Target Angle: ");
  Serial.println(targetAngle);

  while (true) {
    compass.read();
    int currentAzimuth = compass.getAzimuth();
    
    handleDirectionIndication(currentAzimuth, targetAngle);
    
    if (abs(currentAzimuth - targetAngle) <= ANGLE_THRESHOLD_REACHED) {
      break;  // Exit loop when target angle is reached
    }

    delay(10);
  }

  while (true); // Program end
}

// Control LED blinking pattern
void blinkLEDs(int delayTime, int activeLED) {
  // Clear all LEDs
  pixels.clear();
  // Turn on specified LED
  pixels.setPixelColor(activeLED, pixels.Color(0, 255, 0));
  pixels.show();
  delay(delayTime);
  
  // Turn off specified LED
  pixels.clear();
  pixels.show();
  delay(delayTime);
}

// Check if target azimuth is reached
bool isTargetAzimuthReached(int azimuth, int angle) {
  if (abs(azimuth - angle) <= 5) {
    Serial.println("Target azimuth reached! Exiting loop.");
    return true;
  }
  return false;
}

// Wait for North calibration
void waitForNorthCalibration() {
  Serial.println("Waiting for azimuth to be 0 (North Calibration)...");

  while (true) {
    compass.read();
    int azimuth = compass.getAzimuth();

    Serial.print("Current Azimuth: ");
    Serial.println(azimuth);

    if (azimuth == 30) {
      Serial.println("Calibration in progress, compass points to North");
      break;
    }

    delay(250);
  }
}

// Determine which LED should be active based on current and target angles
int getActiveLED(int currentAzimuth, int targetAngle) {
  // Calculate angle difference
  int angleDiff = targetAngle - currentAzimuth;
  
  // Normalize angle difference to -180 to 180 degrees
  if (angleDiff > 180) angleDiff -= 360;
  if (angleDiff < -180) angleDiff += 360;
  
  // Determine active LED based on angle difference
  // LED_INDICES[0] = Front (0 degrees)
  // LED_INDICES[1] = Right (90 degrees)
  // LED_INDICES[2] = Back (180 degrees)
  // LED_INDICES[3] = Left (-90 degrees)
  
  if (angleDiff >= -45 && angleDiff < 45) {
    return LED_INDICES[0];      // Front LED
  } else if (angleDiff >= 45 && angleDiff < 135) {
    return LED_INDICES[1];      // Right LED
  } else if (angleDiff >= 135 || angleDiff < -135) {
    return LED_INDICES[2];      // Back LED
  } else {
    return LED_INDICES[3];      // Left LED
  }
}

// Core logic for direction indication
void handleDirectionIndication(int currentAzimuth, int targetAngle) {
  // Calculate angle difference
  int angleDiff = abs(currentAzimuth - targetAngle);
  if (angleDiff > 180) angleDiff = 360 - angleDiff;
  
  // Get active LED
  int activeLED = getActiveLED(currentAzimuth, targetAngle);
  
  // Control LED state based on angle difference
  if (angleDiff <= ANGLE_THRESHOLD_REACHED) {
    // Target reached, LED stays on
    pixels.clear();
    pixels.setPixelColor(activeLED, pixels.Color(0, 255, 0));
    pixels.show();
    return;
  } else if (angleDiff <= ANGLE_THRESHOLD_NEAR) {
    // Near target, slow blink
    blinkLEDs(SLOW_BLINK_DELAY, activeLED);
  } else {
    // Far from target, fast blink
    blinkLEDs(FAST_BLINK_DELAY, activeLED);
  }

  // Print debug information
  Serial.print("Azimuth: ");
  Serial.print(currentAzimuth);
  Serial.print(" | Target: ");
  Serial.print(targetAngle);
  Serial.print(" | Active LED: ");
  Serial.println(activeLED);
}

