/*
Project Name: Arc
Project Author: Nathaniel Mugenyi
Feature: Converted to a Web Server to expose current data over Wi-Fi with a STATIC IP.

Endpoint: http://<ESP32_STATIC_IP>/current
Response: {"current": 0.000}
*/

#include <WiFi.h>
#include <WebServer.h>

// --- CONFIGURATION ---
// 1. Enter your Wi-Fi credentials
const char* ssid = "arc";     // <-- CHANGE THIS
const char* password = "admin"; // <-- CHANGE THIS

// 2. STATIC IP CONFIGURATION (CRITICAL)
// You MUST change these values to match your home network:
// IMPORTANT: Choose an IP that is not already in use on your network!
IPAddress local_IP(192, 168, 1, 200); // <-- Preferred Static IP for ESP32
IPAddress gateway(192, 168, 1, 1);    // <-- Your Router's IP Address
IPAddress subnet(255, 255, 255, 0);   // <-- Standard Subnet Mask

// 3. Hardware Pins and Indicator
int check = 13; // Indicator LED pin (GPIO 13)
const int currentPin = 2; // Analog input pin for ACS712 (GPIO 2 - ADC1_CH2)

// Web Server instance on port 80
WebServer server(80);

// --- CURRENT MEASUREMENT FUNCTION (Unchanged) ---
float measureCurrent() {
  float AcsValue = 0.0;
  float Samples = 0.0;
  float AvgAcs = 0.0;
  float AcsValueF = 0.0;
  const int numSamples = 150;
  
  for(int x = 0; x < numSamples; x++){
    AcsValue = analogRead(currentPin);
    Samples = Samples + AcsValue;
    delay(3); 
  }
  
  // 1. Calculate the average raw ADC reading
  AvgAcs = Samples / numSamples;
  
  // 2. Convert ADC reading to current (Amperes)
  // Assumes 10-bit ADC (1024) and a 5.0V reference.
  AcsValueF = (2.5 - (AvgAcs * (5.0 / 1024.0))) / 0.066;
  
  return AcsValueF;
}


// --- WEB SERVER HANDLERS (Unchanged) ---

void handleCurrent() {
  float current = measureCurrent();
  String jsonResponse = "{ \"current\": " + String(current, 3) + " }";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", jsonResponse);
}

void handleRoot() {
  String html = "<html><head><title>Arc Sensor Data</title></head><body>";
  html += "<h1>Arc Sensor Web Server (Static IP)</h1>";
  html += "<p>Current Reading: <span id='currentData'>Loading...</span> A</p>";
  html += "<p>Access the API endpoint directly: <a href='/current'>/current</a></p>";
  html += "<p>Device IP: " + WiFi.localIP().toString() + "</p>";
  
  html += "<script>";
  html += "function fetchData() { fetch('/current').then(res => res.json()).then(data => { document.getElementById('currentData').innerText = data.current.toFixed(3); }); }";
  html += "setInterval(fetchData, 1000); fetchData();";
  html += "</script>";
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}



void setup() {
  
  Serial.begin(115200);
  pinMode(check, OUTPUT);
  pinMode(currentPin, INPUT); 

  // *** STATIC IP CONFIGURATION START ***
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Static IP configuration failed!");
  }
  // *** STATIC IP CONFIGURATION END ***

  // Connect to Wi-Fi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Wait for connection and print status
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("Static IP Address: ");
  Serial.println(WiFi.localIP()); // This will now print your set static IP

  // Set up server routes
  server.on("/current", handleCurrent);
  server.on("/", handleRoot); 

  // Start the server
  server.begin();
  Serial.println("HTTP server started.");
}


void loop() {
  server.handleClient();
}
