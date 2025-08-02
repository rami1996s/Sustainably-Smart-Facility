#include <WiFi.h> 
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h> // Use the ESP32Servo library
#include "DHT.h"

// Pin Definitions
const int DHT_PIN = 4;
const int MQ2_PIN = 35;
const int FAN_PIN = 27; // GPIO pin for fan
const int SERVO_PIN = 25; // GPIO pin for servo motor

Servo servoMotor; // Create Servo object

// AWS IoT Topics
#define AWS_IOT_PUBLISH_TOPIC   "sensor/data"
#define AWS_IOT_SUBSCRIBE_TOPIC "actuator/command"
#define DHTTYPE DHT11

// Sensor Data Variables
float temperature;
float humidity;
int mq2Reading;

// Delay Variables
unsigned long previousMillis = 0; // Stores the last time the function was run
const unsigned long interval = 30000; // Interval in milliseconds (e.g., 2000 ms = 2 seconds)


// AWS MQTT Client
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

DHT dht(DHT_PIN, DHTTYPE);

// Connect to Wi-Fi
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
}

// Connect to AWS IoT Core
void connectAWS() {
  net.setCACert(AWS_CERT_CA1);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IoT Core...");
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if (client.connected()) {
    Serial.println("\nAWS IoT Core Connected!");
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  } else {
    Serial.println("\nAWS IoT Connection Failed!");
  }
}

// Publish sensor data to AWS IoT Core
void publishMessage() {
  StaticJsonDocument<256> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["gas"] = mq2Reading;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.println("Sensor data published to AWS IoT Core.");
}

// Handle incoming MQTT messages
void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.print("Payload: ");
  Serial.println(message);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  if (doc.containsKey("fan")) {
    int fanState = doc["fan"];
    digitalWrite(FAN_PIN, fanState ? HIGH : LOW);
    Serial.print("Fan state set to: ");
    Serial.println(fanState ? "ON" : "OFF");
  } else {
    Serial.println("Fan key not found in the JSON payload.");
  }

  if (doc.containsKey("servo")) {
    int servoAngle = doc["servo"];
    servoMotor.write(servoAngle);
    Serial.print("Servo motor angle set to: ");
    Serial.println(servoAngle);
  } else {
    Serial.println("Servo key not found in the JSON payload.");
  }
}

// Setup function
void setup() {
  Serial.begin(115200);

  // Initialize sensors and actuators
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  servoMotor.attach(SERVO_PIN); // Attach servo motor to the specified pin
  servoMotor.write(0);          // Set initial servo angle to 0 degrees

  dht.begin();

  // Connect to Wi-Fi and AWS IoT Core
  connectWiFi();
  connectAWS();
}

// Main loop
void loop() {
  client.loop();
  
  // Read sensors
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  mq2Reading = analogRead(MQ2_PIN);
  

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Update the last time the function was run
    // Log sensor data
     Serial.println("Temperature: " + String(temperature) + "°C");
     Serial.println("Humidity: " + String(humidity) + "%");
     Serial.println("Gas Reading: " + String(mq2Reading));
     Serial.println("---");

    // Publish sensor data
    publishMessage();
  }


  //delay(30000); // Wait for 30 seconds before reading again
}
