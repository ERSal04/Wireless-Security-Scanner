#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>

// Define the constants for the display screen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define LINE_HEIGHT 20

int networkCount = 0;
int scrollOffset = 0; // Will be used for scrolling

void performWifiScan();
void displayNetworks();
uint16_t getSecurityColor(String);
String getEncryptionType(wifi_auth_mode_t);

void setup() {
  M5.begin(); // Initialize M5 Stack

  // Resets the screen and text
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 10);

  // Default display showing device ready
  M5.Lcd.println("Wi-Fi Scanner Ready");

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.println("Press Button A to scan");

  // Sets the mode to Station mode for scanning only
  WiFi.mode(WIFI_STA); 
  WiFi.disconnect();
}

void loop() {
  M5.update(); // Must call this to update button states

  // Check if button A was pressed
  if (M5.BtnA.wasPressed()) {

    performWifiScan();
  }

  // Delays the start of the new loop by 1 second
  delay(100);
}

void performWifiScan() {
  // Resets the screen
  M5.Lcd.clear();
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextSize(2);
   M5.Lcd.setTextColor(WHITE);
  M5.Lcd.println("Scanning...");

  // Gets the count of available Networks
  networkCount = WiFi.scanNetworks();

  displayNetworks();
}

void displayNetworks() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);

  // Header
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.printf("Found %d networks\n", networkCount);
  M5.Lcd.drawLine(0, 15, SCREEN_WIDTH, 15, WHITE);

  int yPos = 20; // Start position after the header

  // TODO: Loop through the Networks
  // Display up to 10 networks (Scolling will be implemented later)
  int maxDisplay = min(networkCount, 10);

  // Loops through the discpvered networks and displays the SSID and RSSI
  for (int i = 0; i < maxDisplay; i++) {
    String ssid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);
    wifi_auth_mode_t security = WiFi.encryptionType(i);

    // Get security type as a string
    String securityType = getEncryptionType(security);

    // Choose a color based on the encryption type
    uint16_t color = getSecurityColor(securityType);

    // M5.Lcd.setTextColor(color);
    M5.Lcd.setCursor(5, yPos);

    // Truncate long SSIDs to fit on screen
    if (ssid.length() > 15) {
      ssid = ssid.substring(0, 12) + "...";
    }

    M5.Lcd.setTextColor(color);
    M5.Lcd.printf("%-15s %4ddBm %s\n", ssid.c_str(), rssi, securityType.c_str());

    yPos += LINE_HEIGHT;

  }

  // Clean up after scanning
  WiFi.scanDelete();
}


uint16_t getSecurityColor(String encryptionType) {
  if (encryptionType == "OPEN") {
    return TFT_RED;
  } else if (encryptionType == "WEP") {
    return TFT_ORANGE;
  } else if (encryptionType == "WPA") {
    return TFT_YELLOW;
  } else if (encryptionType == "WPA2" || encryptionType == "WPA3") {
    return TFT_GREEN;
  } else {
    // Default/Enterprise
    return TFT_WHITE;
  }
}

String getEncryptionType(wifi_auth_mode_t encryptionType) {
  
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      return "OPEN";

    case WIFI_AUTH_WEP:
      return "WEP";

    case WIFI_AUTH_WPA_PSK:
      return "WPA";

    case WIFI_AUTH_WPA2_PSK:
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA2";

    case WIFI_AUTH_WPA3_PSK:
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA3";

    case WIFI_AUTH_ENTERPRISE:
    case WIFI_AUTH_WPA3_ENT_192:
      return "Enterprise";
  
    default:
      return "Unknown";
  }
}