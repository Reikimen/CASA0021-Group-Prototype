#include "arduino_secrets.h" 
#include "myConfig.h"

// ===== Main Program =====c:\Users\JiaYing\Downloads\arduino_secrets.h
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
  
  // After some interrupt events "press button" then Send message
  // sendmqtt_location(); // the function for sending MQTT GPS message based on the mode you set in mgConfig.h
  
  sendmqtt_happy();
  delay(500);
  sendmqtt_sad();
  delay(500);
  sendmqtt_angry();
  delay(500);
  sendmqtt_normal();
  delay(500);

  delay(500);  // Loop every 0.5 seconds
}
