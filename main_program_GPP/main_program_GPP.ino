#include "arduino_secrets.h" 
#include "myConfig.h"

// ===== Main Program =====
void setup() {
  Serial.begin(115200);
  Compass_init();
}

void loop() {
  // All operations of BLE, Wi-Fi & GPS info update are handled by callbacks
  // Check MQTT connections
  if (!client.connected()) {
    reconnectMQTT();
  }
  
  // Keep MQTT client background tasks
  client.loop();
  
  // Send message
  sendmqtt(); // the function for sending MQTT message based on the mode you set in mgConfig.h

  delay(500);  // Loop every 0.5 seconds
}
