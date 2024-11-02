#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

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
uint32_t irBuffer[BUFFER_SIZE];

int32_t bufferLength;
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

// Servo setup
Servo myservo; // Create a servo object
const int servoPin = 19; // Set the pin for the MG996R servo

// Wi-Fi credentials
const char* ssid = "PLDTHOMEFIBRRcAMz";
const char* password = "F@yeed22beats";

// Create a web server on port 80
WebServer server(8000);

// Handle requests and send sensor data in JSON format
void handleSensorData() {
  Serial.println("Received request for sensor data");

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
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.println(WiFi.localIP());

  server.on("/sensor-data", handleSensorData);
  server.begin();
  Serial.println("Server started");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 sensor initialization failed.");
    while (1);
  }
  Serial.println("MAX30102 sensor initialized.");
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  pinMode(trigPinFront, OUTPUT);
  pinMode(echoPinFront, INPUT);
  pinMode(trigPinSide, OUTPUT);
  pinMode(echoPinSide, INPUT);
  pinMode(waterSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(vibrationPin, OUTPUT);

  myservo.attach(servoPin); // Attach the servo
  Serial.println("Setup completed.");
}

void loop() {
  handleSensorProcessing();
  server.handleClient();
}

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
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  long distanceFront = measureDistance(trigPinFront, echoPinFront);
  long distanceSide = measureDistance(trigPinSide, echoPinSide);
  int waterDetected = digitalRead(waterSensorPin);

  if ((distanceFront > minimumDistance && distanceFront <= distanceThreshold) ||
      (distanceSide > minimumDistance && distanceSide <= distanceThreshold) ||
      waterDetected == HIGH) {
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(vibrationPin, HIGH);

    // Servo rotates to detect direction
    if (distanceFront <= distanceThreshold) {
      myservo.write(0);  // Rotate to left
      delay(500);
      long leftDistance = measureDistance(trigPinFront, echoPinFront);

      myservo.write(180);  // Rotate to right
      delay(500);
      long rightDistance = measureDistance(trigPinFront, echoPinFront);

      if (leftDistance < rightDistance) {
        Serial.println("Object detected on the left side.");
      } else {
        Serial.println("Object detected on the right side.");
      }
    }
  } else {
    digitalWrite(buzzerPin, LOW);
    digitalWrite(vibrationPin, LOW);
  }

  delay(100);
}