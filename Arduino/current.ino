/*
Project Name: Arc
Project Author: Nathaniel Mugenyi
Feature: Converted to a Web Server using MDNS for easy discovery.
Device Name: "ARC Power Unit" (Hostname: arcpowerunit.local)

Endpoint: http://arcpowerunit.local/current
Response: {"current": 0.000}
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h> // Include the MDNS library

// --- CONFIGURATION ---
// 1. Enter your Wi-Fi credentials
const char* ssid = "-_-";     // <-- CHANGE THIS
const char* password = "qtrf3498"; // <-- CHANGE THIS

// 2. MDNS Hostname Configuration
// The device will be accessible via: http://arcpowerunit.local
const char* host = "arcpowerunit";

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
  
  // Take multiple samples to get a more stable average reading
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
  String html = "<html><head><title>Arc Power Unit Data</title></head><body>";
  html += "<h1>ARC Power Unit Web Server</h1>";
  html += "<p>Current Reading: <span id='currentData'>Loading...</span> A</p>";
  html += "<p>Access the API endpoint directly: <a href='/current'>/current</a></p>";
  html += "<p>Device Hostname: **http://" + String(host) + ".local/**</p>";
  html += "<p>Current IP: " + WiFi.localIP().toString() + " (Dynamic)</p>";
  
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
  Serial.print("Dynamic IP Address: ");
  Serial.println(WiFi.localIP());

  // --- MDNS SETUP START ---
  // Start MDNS service and advertise the hostname
  if (!MDNS.begin(host)) {
    Serial.println("Error starting MDNS");
    return;
  }
  Serial.print("MDNS responder started. Hostname: ");
  Serial.print(host);
  Serial.println(".local");
  // --- MDNS SETUP END ---

  // Set up server routes
  server.on("/current", handleCurrent);
  server.on("/", handleRoot); 

  // Start the server
  server.begin();
  Serial.println("HTTP server started.");
}


void loop() {
  // NOTE: MDNS.update() is NOT required for the ESP32 framework
  server.handleClient();
}
