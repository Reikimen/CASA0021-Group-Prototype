#include <QMC5883LCompass.h>

QMC5883LCompass compass;

void setup() {
  Serial.begin(9600);
  compass.init();
  compass.setCalibrationOffsets(-708.00, -1408.00, 895.00);
  compass.setCalibrationScales(1.33, 0.73, 1.14);

  Serial.println("Starting Compass Reading...");
}

void loop() {
  waitForNorthCalibration();  // First detect azimuth == 0 before proceeding

  int angle = random(-180, 180); // Generate a random target angle
  Serial.print("Target Angle: ");
  Serial.println(angle);

  while (true) { // Infinite loop until azimuth is within range of target angle
    compass.read();
    
    int azimuth = compass.getAzimuth();
    char myArray[3];
    compass.getDirection(myArray, azimuth);

    Serial.print("Azimuth: ");
    Serial.print(azimuth);
    Serial.print(" | Target: ");
    Serial.print(angle);
    Serial.print(" | Direction: ");
    Serial.print(myArray[0]);
    Serial.print(myArray[1]);
    Serial.print(myArray[2]);
    Serial.println();

    // Check if azimuth is within ±5 degrees of the target
    if (isTargetAzimuthReached(azimuth, angle)) {
      break;  // Exit loop
    }

    delay(250);
  }

  while (true); // Stop the program
}

// Function to check if the target azimuth is reached (within ±5 degrees tolerance)
bool isTargetAzimuthReached(int azimuth, int angle) {
  if (abs(azimuth - angle) <= 5) {
    Serial.println("Target azimuth reached! Exiting loop.");
    return true; // Signal to break the loop
  }
  return false;
}

// Function to wait until azimuth is detected as 0 (North Calibration)
void waitForNorthCalibration() {
  Serial.println("Waiting for azimuth to be 0 (North Calibration)...");

  while (true) {
    compass.read();
    int azimuth = compass.getAzimuth();

    Serial.print("Current Azimuth: ");
    Serial.println(azimuth);

    if (azimuth == 0) {
      Serial.println("Calibration in progress, compass points to North");
      /* Light up to illustrate calibration to north */

      break; // Exit loop once azimuth == 0 is detected
    }

    delay(250);
  }
}