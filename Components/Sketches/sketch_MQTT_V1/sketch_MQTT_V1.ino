#include <WiFi.h>
#include <PubSubClient.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

// WiFi and MQTT settings
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
const char* mqtt_server = "your_MQTT_BROKER_ADDRESS";

WiFiClient espClient;
PubSubClient client(espClient);
TinyGPSPlus gps;
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (!mag.begin()) {
    Serial.println("Could not find a valid HMC5883 sensor, check wiring!");
    while (1);
  }

  // Initialize LEDs and vibrator pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(VIBRATOR_PIN, OUTPUT);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  // Vibrate and turn LED white on any message
  digitalWrite(VIBRATOR_PIN, HIGH);
  delay(500);
  digitalWrite(VIBRATOR_PIN, LOW);
  setLEDColor(255, 255, 255); // White

  // Process emotional states and direction
  sensors_event_t event;
  mag.getEvent(&event);
  float heading = atan2(event.magnetic.y, event.magnetic.x) * 180 / PI;
  if (heading < 0) heading += 360;

  if (message == "button1(happy)" && isDirectionSynced(heading)) {
    setLEDColor(255, 255, 0); // Yellow
  } else if (message == "button1(sad)" && isDirectionSynced(heading)) {
    setLEDColor(0, 0, 255); // Blue
  }
}

bool isDirectionSynced(float heading) {
  // Implement your direction sync logic here
  return true; // Placeholder
}

void setLEDColor(int red, int green, int blue) {
  // Implement your LED control logic here
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read GPS data and publish location
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
    if (gps.location.isUpdated()) {
      String location = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
      client.publish("gps/location", location.c_str());
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("your_topic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}