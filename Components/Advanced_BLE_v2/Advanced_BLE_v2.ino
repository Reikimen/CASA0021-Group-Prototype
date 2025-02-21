#include "myConfig.h"

// ===== Main Program =====
void setup() {
  Serial.begin(115200);
  Compass_BLE_init();

  loadStoredWiFi();
  WiFi_Connector();
}

void loop() {
  // Keep an empty loop, all operations are handled by callbacks
  delay(1000);
}
