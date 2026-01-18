#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>

int networkCount = 0;

void performWifiScan();

void setup() {
  M5.begin(); // Initialize M5 Stack

  // Todo: Initialize the display
  // Hint: Look at M5.Lcd functions
  // What should I display when the device first starts?
  M5.Lcd.clear(BLACK);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);


  M5.Lcd.println("Wi-Fi Scanner Ready");

  // Todo: Set Wifi mode
  // Hint: Should it ne in Station mode, AP mode, or something else?
  // Look up Wifi.mode() documentation

  WiFi.mode(WIFI_STA); // Sets the mode to Station mode for scanning only

}

void loop() {
  M5.update(); // Must call this to update button states

  // Todo: Check if button A (Left button) is pressed
  // Hint: M5.BtnA.wasPressed() returns true when pressed

  if (M5.BtnA.wasPressed()) {
    performWifiScan();
  }

  delay(100);

}

void performWifiScan() {
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Scanning...");

  // Todo: Perform the actual Wi-Fi scan
  // Hint: Wifi.scanNetworks() returns the number of networks found
  // Store this in network count

  networkCount = WiFi.scanNetworks();

  // Todo: display the number of networks found
  M5.Lcd.printf("Found %d networks\n", networkCount);

  // Todo: Loop through each network and display its information
  // Hint: Use a for loop from 0 to the networkCount
  // For each network, I can get:
  //  - WiFi.SSD(i) - Network name
  //  - WiFi.RSSI(i) - Network strength
  //  - Wifi.encryptionType(i) - Security type

  for (int i = 0; i < networkCount; i++) {
    // Code here
    // Need to display each network's information
    String ssid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);
    wifi_auth_mode_t security = WiFi.encryptionType(i);

    M5.Lcd.printf("%s: (%d dBm)\n", ssid.c_str(), rssi);

  }

  // Clean up after scanning
  WiFi.scanDelete();
}
