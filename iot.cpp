#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Replace with your Thingspeak channel details
const char* serverName = "http://api.thingspeak.com/update";
const char* apiKey = "YOUR_WRITE_API_KEY";

// Pin assignments for IR sensors
const int sensorPin = 34;  // GPIO34

// Threshold for detection duration
const unsigned long DETECTION_THRESHOLD = 2000; // 2000 milliseconds (2 seconds)

// Timing variables
unsigned long detectionStartTime = 0;
bool sensorActive = false;

// Object count
int objectCount = 0;

void setup() {
  Serial.begin(115200);

  // Initialize the IR sensor pin
  pinMode(sensorPin, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  int sensorValue = digitalRead(sensorPin);
  unsigned long currentMillis = millis();

  if (sensorValue == HIGH) { // Object detected
    if (!sensorActive) {
      detectionStartTime = currentMillis;
      sensorActive = true;
    } else {
      unsigned long detectionDuration = currentMillis - detectionStartTime;
      if (detectionDuration >= DETECTION_THRESHOLD) {
        // Increment the object count and send data to Thingspeak
        objectCount++;
        sendToThingspeak(objectCount, detectionDuration);
        // Reset detection time
        detectionStartTime = currentMillis;
      }
    }
  } else { // Object not detected
    if (sensorActive) {
      unsigned long detectionDuration = currentMillis - detectionStartTime;
      if (detectionDuration >= DETECTION_THRESHOLD) {
        // Increment the object count and send data to Thingspeak
        objectCount++;
        sendToThingspeak(objectCount, detectionDuration);
      }
      sensorActive = false;
    }
  }

  delay(100); // Short delay for stability
}

void sendToThingspeak(int count, unsigned long duration) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName); // Start connection to Thingspeak
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Set content type for POST

    // Prepare the payload for POST
    String payload = "api_key=" + String(apiKey) + "&field1=" + String(count) + "&field2=" + String(duration);
    
    int httpResponseCode = http.POST(payload); // Send POST request with payload

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error code: " + String(httpResponseCode));
    }

    http.end(); // End the HTTP session
  } else {
    Serial.println("Not connected to WiFi");
  }
}
