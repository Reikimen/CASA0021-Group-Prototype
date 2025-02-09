#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

TinyGPSPlus gps;
HardwareSerial ss(2); // Use UART2

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  delay(3000);
  Serial.println("GPS Module Test");
}

void updateSerial(){
  delay(500);
  while (Serial.available())  {
    Serial2.write(Serial.read()); // Forward what Serial received to Software Serial Port
  }
  while (Serial2.available())  {
    Serial.write(Serial2.read()); // Forward what Software Serial received to Serial Port
  }
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid()){
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }
  Serial.print(F(" Date/Time: "));
  if (gps.date.isValid() && gps.time.isValid()) {
    Serial.print(gps.date.day());
    Serial.print("/");
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.print(gps.date.year());
    Serial.print(" ");
    Serial.print(gps.time.hour());
    Serial.print(":");
    Serial.print(gps.time.minute());
    Serial.print(":");
    Serial.print(gps.time.second());
  } else {
    Serial.print(F("INVALID"));
  }
  Serial.println();
}

void loop() {
  updateSerial();
  while (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))
      displayInfo();
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("No GPS detected: check wiring.");
    while (true);
  }
}