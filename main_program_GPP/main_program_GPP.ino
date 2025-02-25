#include "arduino_secrets.h" 
#include "myConfig.h"

// ===== Main Program =====
void setup() {
  Serial.begin(115200);
  Compass_init();
}

void loop() {
  // Keep an empty loop, all operations are handled by callbacks
  // Check MQTT connections
  if (!client.connected()) {
    reconnectMQTT();
  }
  
  // Keep MQTT client background tasks
  client.loop();
  
  // Send time message
  sendmqtt();
  
  Serial.println("Sent a time info message");
  delay(10000);  // Send a message every 10 seconds
}
