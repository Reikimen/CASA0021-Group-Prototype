#include <math.h>

// used to convert degree to radius
#define DEG_TO_RAD (M_PI / 180.0)

void setup() {
    Serial.begin(115200);
    double lat1 = 51.5074;  
    double lon1 = 0.0;      
    double lat2 = 40.7128;  
    double lon2 = -74.0059; 

    // calculate the bearing 
    double bearing = calculateBearing(lat1, lon1, lat2, lon2);

    Serial.print("Bearing: ");
    Serial.println(bearing, 1);
}

void loop() {
   
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