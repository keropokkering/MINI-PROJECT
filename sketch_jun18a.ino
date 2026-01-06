/* ESP32 -> BMP280 + HC-SR04 -> Google Apps Script (Google Sheets)
   Interval: 30 seconds
   Format (Sheet columns): Timestamp | Temp (°C) | Pressure (hPa) | Distance (cm) | Device Name | RSSI
*/

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SEALEVELPRESSURE_HPA (1013.25)

// WiFi credentials
const char* WIFI_SSID = "Redmi Note 10 Pro";
const char* WIFI_PASS = "0123456789";

// Apps Script Webhook URL
const char* APPS_SCRIPT_URL = "https://docs.google.com/spreadsheets/d/1-ZHL-A83jwOc8eZQNqDxxeXMed-PkwuPRZIWnquzfCM/edit?gid=0#gid=0"; // <-- replace

// Device info
const char* DEVICE_NAME = "ESP32_Node_01";

// BMP280
Adafruit_BMP280 bmp; // I2C

// Ultrasonic pins
const int TRIG_PIN = 26;
const int ECHO_PIN = 27;

// Timing
const unsigned long SEND_INTERVAL_MS = 30000UL; // 30 seconds
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Setup pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // Start WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed (timeout).");
  }

  // Initialize BMP280 (I2C default address 0x76)
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not detected. Check wiring!");
  } else {
    Serial.println("BMP280 detected.");
  }

  lastSend = millis() - SEND_INTERVAL_MS; // immediate first send
}

float readDistanceCM() {
  // Trigger 10us pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo pulse (timeout 38ms -> ~653 cm)
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 38000UL);
  if (duration == 0) return -1.0; // timeout

  float distance_cm = (duration / 2.0) * 0.0343;
  return distance_cm;
}

String makeJsonPayload(String timestamp, float temp, float pres, float dist, int rssi) {
  String payload = "{";
  payload += "\"timestamp\":\"" + timestamp + "\",";
  payload += "\"temperature\":" + String(temp, 2) + ",";
  payload += "\"pressure\":" + String(pres, 2) + ",";
  payload += "\"distance\":" + String(dist, 2) + ",";
  payload += "\"device\":\"" + String(DEVICE_NAME) + "\",";
  payload += "\"rssi\":" + String(rssi);
  payload += "}";
  return payload;
}

void sendToAppsScript(String jsonPayload) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping send.");
    return;
  }

  HTTPClient http;
  http.begin(APPS_SCRIPT_URL);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonPayload);
  if (httpResponseCode > 0) {
    String resp = http.getString();
    Serial.print("HTTP response: ");
    Serial.print(httpResponseCode);
    Serial.print(" Body: ");
    Serial.println(resp);
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

String getTimestampISO() {
  // Simple local timestamp based on millis; for accurate time use NTP
  time_t nowSec = time(NULL);
  struct tm * timeinfo = localtime(&nowSec);
  char buf[32];
  if (timeinfo != NULL) {
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", timeinfo);
    return String(buf);
  } else {
    return String(millis());
  }
}

void loop() {
  if (millis() - lastSend >= SEND_INTERVAL_MS) {
    lastSend = millis();

    // Read sensors
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F; // hPa
    float distance = readDistanceCM(); // -1 if timeout
    int rssi = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
    String ts = getTimestampISO();

    // Serial print
    Serial.println("---------");
    Serial.println("Timestamp: " + ts);
    Serial.println("Temp (C): " + String(temperature, 2));
    Serial.println("Pressure hPa: " + String(pressure, 2));
    if (distance < 0) Serial.println("Distance: no echo");
    else Serial.println("Distance cm: " + String(distance, 2));
    Serial.println("RSSI: " + String(rssi));

    // Build JSON and send
    String payload = makeJsonPayload(ts, temperature, pressure, distance, rssi);
    sendToAppsScript(payload);
  }

  delay(10);
}
