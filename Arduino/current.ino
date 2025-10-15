/*
Project Name: Arc
Project Author: Nathaniel Mugenyi
Feature: Converted to a Web Server to expose current data over Wi-Fi.

Endpoint: http://<ESP32_IP>/current
Response: {"current": 0.000}
*/

#include <WiFi.h>
#include <WebServer.h>

// --- CONFIGURATION ---
// 1. Enter your Wi-Fi credentials
const char* ssid = "arc";     // <-- CHANGE THIS
const char* password = "admin"; // <-- CHANGE THIS

// 2. Hardware Pins and Indicator
int check = 13; // Indicator LED pin (GPIO 13)
const int currentPin = 2; // Analog input pin for ACS712 (GPIO 2 - ADC1_CH2)

// Web Server instance on port 80
WebServer server(80);

// --- CURRENT MEASUREMENT FUNCTION ---
// Encapsulates the core logic to get a stable current reading
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
  // WARNING: This calculation assumes a 10-bit ADC (1024) and a 5.0V reference.
  // Standard ESP32 ADCs are typically 12-bit (4095) and 3.3V reference. 
  // If your readings are inaccurate, you may need to adjust 5.0/1024.0.
  
  // Measured_Voltage = (AvgAcs * (5.0 / 1024.0))
  // Calculation: ((Quiescent_Voltage - Measured_Voltage) / Sensitivity)
  AcsValueF = (2.5 - (AvgAcs * (5.0 / 1024.0))) / 0.066;
  
  return AcsValueF;
}


// --- WEB SERVER HANDLERS ---

// Handler for the /current endpoint
void handleCurrent() {
  // Read the current value using the dedicated function
  float current = measureCurrent();
  
  // Format the response as JSON
  String jsonResponse = "{ \"current\": " + String(current, 3) + " }";

  // Allow cross-origin requests (CORS) for web apps to access this API
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  // Send the JSON response
  server.send(200, "application/json", jsonResponse);
}

// Handler for the root path 
void handleRoot() {
  String html = "<html><head><title>Arc Sensor Data</title></head><body>";
  html += "<h1>Arc Sensor Web Server</h1>";
  html += "<p>Current Reading: <span id='currentData'>Loading...</span> A</p>";
  html += "<p>Access the API endpoint directly: <a href='/current'>/current</a></p>";
  
  // Simple JavaScript to fetch and display the data
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
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up server routes
  server.on("/current", handleCurrent);
  server.on("/", handleRoot); // Set up a simple root page for testing

  // Start the server
  server.begin();
  Serial.println("HTTP server started.");
}


// --- LOOP ---
void loop() {
  // Web Server: Handle incoming client requests
  server.handleClient();
  // Note: The measurement logic is now only called when a client requests the data.
}
