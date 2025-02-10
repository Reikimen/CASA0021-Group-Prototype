#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "myConfig.h"

// ===== Main Program =====
void setup() {
  Serial.begin(115200);
  
  Compass_BLE_init();
}

void loop() {
  // Keep an empty loop, all operations are handled by callbacks
  delay(1000);
}
