#include <QMC5883LCompass.h>
#include <math.h>

// used to convert degree to radius
#define DEG_TO_RAD (M_PI / 180.0)
QMC5883LCompass compass;

//  calibration
void calibrateCompass() {
  Serial.println("Starting Calibration. Move the sensor in all directions...");
  delay(5000);
  Serial.println("Calibrating...");

  compass.calibrate();
  Serial.println("Calibration Done.");

  // obtain calibration parameters
  float offsetX = compass.getCalibrationOffset(0);
  float offsetY = compass.getCalibrationOffset(1);
  float offsetZ = compass.getCalibrationOffset(2);
  float scaleX = compass.getCalibrationScale(0);
  float scaleY = compass.getCalibrationScale(1);
  float scaleZ = compass.getCalibrationScale(2);

  compass.setCalibrationOffsets(offsetX, offsetY, offsetZ);
  compass.setCalibrationScales(scaleX, scaleY, scaleZ);

  Serial.print("Applied Calibration Offsets: ");
  Serial.print(offsetX); Serial.print(", ");
  Serial.print(offsetY); Serial.print(", ");
  Serial.println(offsetZ);

  Serial.print("Applied Calibration Scales: ");
  Serial.print(scaleX); Serial.print(", ");
  Serial.print(scaleY); Serial.print(", ");
  Serial.println(scaleZ);
}

// give +- 5 degree error
bool isTargetAzimuthReached(int azimuth, int angle) {
  if (abs(azimuth - angle) <= 5) {
    Serial.println("Target azimuth reached! Exiting loop.");
    return true; // exit
  }
  return false;
}

// wait to calibrate to north
void waitForNorthCalibration() {
  Serial.println("Waiting for azimuth to be 0 (North Calibration)...");

  while (true) {
    compass.read();
    int azimuth = compass.getAzimuth();

    Serial.print("Current Azimuth: ");
    Serial.println(azimuth);

    if (azimuth == 0) {
      Serial.println("Calibration in progress, compass points to North");
      break;
    }

    delay(250);
  }
}



// calculate the coordinate angle
double calculateBearing(double lat1, double lon1, double lat2, double lon2) {
    double phi1 = lat1 * DEG_TO_RAD;
    double lambda1 = lon1 * DEG_TO_RAD;
    double phi2 = lat2 * DEG_TO_RAD;
    double lambda2 = lon2 * DEG_TO_RAD;

    // Calculate the difference in longitude
    double deltaLambda = lambda2 - lambda1;

    // Calculate azimuth
    double y = sin(deltaLambda) * cos(phi2);
    double x = cos(phi1) * sin(phi2) - sin(phi1) * cos(phi2) * cos(deltaLambda);
    double theta = atan2(y, x);

    // convert radius to degree
    theta = theta * 180.0 / M_PI;

    // reflect between 0 - 360
    if (theta < 0) {
        theta += 360.0;
    }

    return theta;
}
