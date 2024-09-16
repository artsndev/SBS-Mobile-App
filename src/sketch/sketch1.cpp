#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h" // Required for calculating heart rate
#include <WiFi.h>  // For connecting the ESP32 to Wi-Fi
#include <WebServer.h>  // Synchronous web server library
#include <ArduinoJson.h>  // For formatting sensor data as JSON

// Create an instance of the sensor
MAX30105 particleSensor;

// Variables for heart rate calculations
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;

#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];  // IR LED readings buffer

int32_t bufferLength;  // Number of samples
int32_t heartRate;
int8_t validHeartRate;

// Define pin for the buzzer
const int buzzerPin = 17;

// Define pin for the vibration module
const int vibrationPin = 18;

// Define pins for the ultrasonic sensors
const int trigPinFront = 5;
const int echoPinFront = 4;
const int trigPinSide = 15;
const int echoPinSide = 16;

// Define pin for the water sensor
const int waterSensorPin = 36;

// Define the distance threshold (in cm)
const int distanceThreshold = 50;
const int minimumDistance = 1;

// Wi-Fi credentials
const char* ssid = "PLDTHOMEFIBRRcAMz";  // Replace with your Wi-Fi SSID
const char* password = "F@yeed22beats";  // Replace with your Wi-Fi password

// Create a web server on port 80
WebServer server(8000);

// Handle requests and send sensor data in JSON format
void handleSensorData() {
  Serial.println("Received request for sensor data");  // Debug print

  // Add CORS headers
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  DynamicJsonDocument json(1024);

  long distanceFront = measureDistance(trigPinFront, echoPinFront);
  long distanceSide = measureDistance(trigPinSide, echoPinSide);
  int waterDetected = digitalRead(waterSensorPin);

  json["distanceFront"] = distanceFront;
  json["distanceSide"] = distanceSide;
  json["waterDetected"] = waterDetected;
  json["bpm"] = beatsPerMinute;
  json["avgBpm"] = beatAvg;

  String response;
  serializeJson(json, response);

  server.send(200, "application/json", response);
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);

  // Initialize the MAX30102 sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 sensor initialization failed.");
    while (1);  // Stop if initialization fails
  }
  Serial.println("MAX30102 sensor initialized.");
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  // Initialize Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);  // Faster Wi-Fi retry
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.println(WiFi.localIP());

  // Set up the sensor data route
  server.on("/sensor-data", handleSensorData);

  // Start the web server
  server.begin();
  Serial.println("Server started");

  // Set ultrasonic sensor pins as outputs and inputs
  pinMode(trigPinFront, OUTPUT);
  pinMode(echoPinFront, INPUT);
  pinMode(trigPinSide, OUTPUT);
  pinMode(echoPinSide, INPUT);

  // Set the water sensor pin as input
  pinMode(waterSensorPin, INPUT);

  // Set the buzzer and vibration module pins as outputs
  pinMode(buzzerPin, OUTPUT);
  pinMode(vibrationPin, OUTPUT);

  Serial.println("Setup completed.");
}

void loop() {
  // Non-blocking loop for sensor and server operations
  handleSensorProcessing();  // Separate sensor logic to avoid delays
  server.handleClient();  // Handle web server client requests
}

// Function to measure distance using ultrasonic sensor
long measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2;

  if (duration == 0 || distance > 400 || distance < minimumDistance) {
    distance = 0;
  }

  return distance;
}

void handleSensorProcessing() {
  // Process sensor data, e.g., distance and heart rate
  long irValue = particleSensor.getIR();

  // Check if a heartbeat is detected
  if (checkForBeat(irValue == true)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte i = 0; i < RATE_SIZE; i++) {
        beatAvg += rates[i];
      }
      beatAvg /= RATE_SIZE;
    }
  }
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.println(beatAvg);

  if (bufferLength < BUFFER_SIZE) {
    irBuffer[bufferLength] = irValue;
    bufferLength++;
  } else {
    bufferLength = 0;
  }

  // Measure distances and detect water
  long distanceFront = measureDistance(trigPinFront, echoPinFront);
  long distanceSide = measureDistance(trigPinSide, echoPinSide);
  int waterDetected = digitalRead(waterSensorPin);

  // Activate buzzer and vibration module based on sensor data
  if (
    (distanceFront > minimumDistance && distanceFront <= distanceThreshold) ||
    (distanceSide > minimumDistance && distanceSide <= distanceThreshold) ||
    waterDetected == HIGH
  ) {
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(vibrationPin, HIGH);
  } else {
    digitalWrite(buzzerPin, LOW);
    digitalWrite(vibrationPin, LOW);
  }

  delay(100);  // Add small delay to avoid overloading the loop
}
