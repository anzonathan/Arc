/*
Project Name: Arc
Project Author: Nathaniel Mugenyi
Fix: Changed Serial Baud Rate to 115200 (recommended for ESP32) 
to eliminate "weird characters" caused by baud rate mismatch.
*/

int check = 13; // indicator (Note: GPIO 13 on ESP32 is fine)
const int currentPin = 2; // Analog input pin for ACS712 (Note: GPIO 2 is an analog pin on some ESP32 boards)

void setup() {
  
  // *** FIXED: Increased baud rate to 115200 for faster, more stable communication ***
  Serial.begin(115200);

  pinMode(check, OUTPUT);
  // Note: For ESP32 ADC pins, simply calling analogRead() is often enough, 
  // but pinMode(2, INPUT) doesn't hurt.
  pinMode(currentPin, INPUT); 
}

void loop() {
  
  // Variables for current calculation
  float AcsValue = 0.0;
  float Samples = 0.0;
  float AvgAcs = 0.0;
  float AcsValueF = 0.0;
  const int numSamples = 150;
  
  // Take multiple samples to get a more stable average reading
  for(int x = 0; x < numSamples; x++){
    // Read the analog value from the sensor pin
    AcsValue = analogRead(currentPin);
    Samples = Samples + AcsValue;
    // Delay slightly between readings
    delay(3); 
  }
  
  // 1. Calculate the average raw ADC reading
  AvgAcs = Samples / numSamples;
  
  // 2. Convert ADC reading to current (Amperes)
  // Assumes: 
  // - 10-bit ADC (1024 divisions)
  // - Reference Voltage (VCC) is 5.0V for the ACS712 (even if ESP32 ADC is 3.3V)
  // - Sensitivity is 0.066V/A (66 mV/A, typical of a +/-30A ACS712)
  
  // Calculation: ((Quiescent_Voltage - Measured_Voltage) / Sensitivity)
  // Measured_Voltage = (AvgAcs * (5.0 / 1024.0))
  // Quiescent_Voltage is 2.5V
  AcsValueF = (2.5 - (AvgAcs * (5.0 / 1024.0))) / 0.066;

  // Print the calculated current value
  Serial.println(AcsValueF);
  
  // Short delay before the next loop iteration
  delay(50);
}
