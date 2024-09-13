#include <Wire.h>
#include <MAX30105.h>   // Correct header for SparkFun MAX3010x library
#include <heartRate.h>  // Required for calculating heart rate
#include <spo2_algorithm.h>  // Required for calculating SpO2
#include <WiFi.h>  // For connecting the ESP32 to Wi-Fi
#include <WiFiClientSecure.h>  // For HTTPS connection
#include <ArduinoJson.h>  // For formatting sensor data as JSON

// SSL certificate and private key (replace these with your actual certificate)
const char* server_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIEGTCCAwGgAwIBAgIUfoM3BDYTmtL6SDN1sDt/U6imdlAwDQYJKoZIhvcNAQEL
BQAwgZsxCzAJBgNVBAYTAlBIMQ8wDQYDVQQIDAZNYW5pbGExFzAVBgNVBAcMDlph
bWJvYW5nYSBDaXR5MREwDwYDVQQKDAhFbGl0SW5ubzETMBEGA1UECwwKYXJ0aXNh
bmRldjETMBEGA1UEAwwKYXJ0aXNhbmRldjElMCMGCSqGSIb3DQEJARYWcnViaWNr
a2luZzA0QGdtYWlsLmNvbTAeFw0yNDA5MTIyMzU0MDFaFw0yNTA5MTIyMzU0MDFa
MIGbMQswCQYDVQQGEwJQSDEPMA0GA1UECAwGTWFuaWxhMRcwFQYDVQQHDA5aYW1i
b2FuZ2EgQ2l0eTERMA8GA1UECgwIRWxpdElubm8xEzARBgNVBAsMCmFydGlzYW5k
ZXYxEzARBgNVBAMMCmFydGlzYW5kZXYxJTAjBgkqhkiG9w0BCQEWFnJ1Ymlja2tp
bmcwNEBnbWFpbC5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCb
2sCFODJdDuANPRWHziSfIUDI2a05Fges7UPaVqaau/4vCjjRQlBZUI5l7g4S2e7a
TG0X5Xx+Z4bhArETbswsIWKCOzQucRh/q+207M1DhZurb450FVuHzo8U2x9UyHlV
HvXay5xFtq7bs9mue0nMxQaxoaTfbMN+x6KdiDEatr+Jr7gOxdUvA0MzM4OBnhfs
e/a95DHSMkrX2K+NTQb8I1RAzDP2Ro2qU8+74LLLzEoRhMlIO6rLGC92dLH+SAlc
Q9Za+su8x3lwPUN6evfTJ5sYfFLZQ49XUd6c+TBH2JaPy3OM/pX+qbbRu+mCeE2C
pZB3+IB49MdSqej3cspJAgMBAAGjUzBRMB0GA1UdDgQWBBSll96ytrqpwFbpikgm
zOoKE2BvYDAfBgNVHSMEGDAWgBSll96ytrqpwFbpikgmzOoKE2BvYDAPBgNVHRMB
Af8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQA4wXTumGTKTx9N3dvMzX/cjRVu
p34o/eJJPBnCfA843P8mZF6G3ym76Y5SLm7I2YVYUHq8gBt4SliO4jv/+Y5pYqpf
g1PA4ecaC4KDH1ex7OI0C082aN12lICJX8pnZ3/RCHRpm1GivQbmoLQ24bDuEmCA
9x6fzxm26NQ1DlibEysb3/vIAZ+Tu85HPBxCCMrkTNJ729ZvxvmUDhVtA+P3TEMN
W7E0s8gS3v2Jj0d9SdfqKEIL9TU1rP1rictGZsfkVV+f6wrzGbRHvqYn52OFPMyy
7iY3X1qLOZjLSM1KQNnvfPQjD1O0LDNTQkh7pQX4rVe4jZa8rv3YLI5MkNt/
-----END CERTIFICATE-----
)EOF";

const char* server_key = R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCb2sCFODJdDuAN
PRWHziSfIUDI2a05Fges7UPaVqaau/4vCjjRQlBZUI5l7g4S2e7aTG0X5Xx+Z4bh
ArETbswsIWKCOzQucRh/q+207M1DhZurb450FVuHzo8U2x9UyHlVHvXay5xFtq7b
s9mue0nMxQaxoaTfbMN+x6KdiDEatr+Jr7gOxdUvA0MzM4OBnhfse/a95DHSMkrX
2K+NTQb8I1RAzDP2Ro2qU8+74LLLzEoRhMlIO6rLGC92dLH+SAlcQ9Za+su8x3lw
PUN6evfTJ5sYfFLZQ49XUd6c+TBH2JaPy3OM/pX+qbbRu+mCeE2CpZB3+IB49MdS
qej3cspJAgMBAAECggEAFs6rxrMDF8AiBH3wZSfDDzBoERxdr5FsPR+Zgt2UUud4
2nxl9SV9qZxSwlLUZ2jR/csaWdlg3wSaZdolfKEFqt2V01YDnFo7ig9oYUYYAa4P
KkbQc2detiCBqr0oOBmAO/ckZkjOjRsfOpJ2pARxt9ALWzo67OE3OJnRT7lK6kwi
YQwr+aPIP55cx19CJX7ui21O46lXRug8oQvQ3zU7d95WC8B2ZDRhTueS6Tzavt74
721qlaa1MxmI4vriTGw55nC7q8CnVqy2pwmRta6H/918hvQO6Rx/kOCZn1zYmfM4
pZ5hIr64xSy8FqDsP1A2ZTJkPq5+m8BmytjG09yQaQKBgQDV7J6FFOBKnzCtMbIx
o0lT8wS5ztWC54CmYpGh4nXMHVcGqZDTH43wlW0KM2apz8Jn7rPA06C9jRpyo1jV
+mGVMdJ4G0Fd9T8rA12JOAfNLQ1djlHX1LC9bSeVk//O393+tNmaNT8xNa9gkyJs
4EKPYqMtnjzB+zofXc4G+2y17QKBgQC6gj1BZTxquDg4/NHMh53WNamgSUmENu3S
LQHrpXj/qZBdtgWuavDJ8fMzwP4BkjiudfuYRqiY+ivPan3NYRXJC5vY5AL3QN1N
Pn0hTS/5q3fbHE7rk/wKFIyizgzM+KxoFDNfrCDWHQHv+UApaWGlsJ/xkCktkVyT
PGvVeZ4aTQKBgAFeIOlS59gySlphHfWJSLe7rwUNGHpcxorJyIqTaxNiAKNvRBjF
c9/c5LSS/Y4KYQDPGuEZuUZc95VSDAhaMHOGvQS9j95s6fl5bQiqjgo2xoYZmdZa
asLbRsyDBLiq3714JIgC3SOQZ02jwy2VBXfrNoXdJCBNXeLDc8TBqMFFAoGAJYxW
3LWO5kVUPFTZv/2Fjzyae8SFYnLUij25Z0GXDJGMmPEuT6ED/QQS3FZ8TMCtXKIw
FsYS50lmTIm57gEfq6wEojO1TSXn58cu1ed+Qxt8M8aNNOxED5t3fMdS2Gkv/Hwk
o5w5uCF65F8fxbaLjBn0+WL2h0/+fT3qz71+UPkCgYA7za//iS/7IduyBlFtB6Ni
CQYsme1LxaaV6m2liP/Q9jd/c6irbPcwTpYxl74ZYpTjoesL/+qk6+JYmY4LvEHV
Hnid98bTviOWLQ98Edi9BjY4r0MiMs9ZGUoCnihNvzPEGqSug5hTq/bCFg99oFb8
z8XGMtjM6HM0thaPZA7nkQ==
-----END PRIVATE KEY-----
)EOF";

// Create an instance of the sensor
MAX30105 pulseSensor;

// Variables for heart rate and SpO2 calculations
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;

#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];  // IR LED readings buffer
uint32_t redBuffer[BUFFER_SIZE];  // Red LED readings buffer

int32_t bufferLength;  // Number of samples
int32_t spo2;
int8_t validSPO2;
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

WiFiServerSecure server(443);  // HTTPS server on port 443

// Handle requests and send sensor data in JSON format
void handleSensorData(WiFiClientSecure client) {
  DynamicJsonDocument json(1024);
  long distanceFront = measureDistance(trigPinFront, echoPinFront);
  long distanceSide = measureDistance(trigPinSide, echoPinSide);
  int waterDetected = digitalRead(waterSensorPin);

  json["distanceFront"] = distanceFront;
  json["distanceSide"] = distanceSide;
  json["waterDetected"] = waterDetected;
  json["bpm"] = beatsPerMinute;
  json["avgBpm"] = beatAvg;
  json["spo2"] = spo2;

  String response;
  serializeJson(json, response);

  // Send the JSON response to the client
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.println(response);
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  delay(1000);

  // Initialize the MAX30102 sensor
  if (!pulseSensor.begin()) {
    Serial.println("MAX30102 sensor initialization failed.");
    while (1);  // Stop if initialization fails
  }
  Serial.println("MAX30102 sensor initialized.");

  // Initialize Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);  // Faster Wi-Fi retry
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.println(WiFi.localIP());

  // Configure SSL for server
  server.setServerKeyAndCert(server_key, server_cert);

  // Start the HTTPS server
  server.begin();
  Serial.println("HTTPS Server started on port 443");

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
  WiFiClientSecure client = server.available();  // Wait for a new client to connect

  if (client) {
    Serial.println("New HTTPS client connected");

    // Read the request
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("GET /sensor-data") != -1) {
      handleSensorData(client);  // Send sensor data
    } else {
      // Default 404 response for unknown routes
      client.println("HTTP/1.1 404 Not Found");
      client.println("Connection: close");
      client.println();
    }

    delay(1);
    client.stop();  // Disconnect the client
    Serial.println("HTTPS client disconnected");
  }

  handleSensorProcessing();  // Process sensor data in the background
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
  long irValue = pulseSensor.getIR();
  long redValue = pulseSensor.getRed();

  // Check if a heartbeat is detected
  if (checkForBeat(irValue)) {
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

  if (bufferLength < BUFFER_SIZE) {
    redBuffer[bufferLength] = redValue;
    irBuffer[bufferLength] = irValue;
    bufferLength++;
  } else {
    maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
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
