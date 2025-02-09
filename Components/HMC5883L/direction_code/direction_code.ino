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
  int angle = random(-180, 180); // Generate a random angle between 0 and 359
  Serial.print("Target Angle: ");
  Serial.println(angle);

  while (true) { // Infinite loop until the azimuth equals angle
    compass.read();

    int x = compass.getX();
    int y = compass.getY();
    int z = compass.getZ();
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

    if (abs(azimuth - angle) <= 5) {
      Serial.println("Target azimuth reached! Exiting loop.");
      break; // Exit the loop when azimuth equals angle
    }

    delay(250);
  }

  while (true); // Stop the program
}
