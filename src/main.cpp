#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>

// Define the constants for the display screen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define LINE_HEIGHT 20
#define MAX_VISIBLE_NETWORKS 9

int networkCount = 0;
int scrollOffset = 0; // Index of the first network to display


void performWifiScan();
void displayNetworks();

void scrollDown();
void scrollUp();
void drawScrollIndicator();

uint16_t getSecurityColor(wifi_auth_mode_t);
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

  // Check if button B was pressed (Scroll Down)
  if (M5.BtnB.wasPressed()) {
    scrollDown();
  } 

  // Check if button A was pressed (Scroll Up)
  if (M5.BtnC.wasPressed()) {
    scrollUp();
  }

  delay(100);
}

void performWifiScan() {
  // Clean up old scanning results (Frees up RAM)
  WiFi.scanDelete();

  // Resets the screen
  M5.Lcd.clear();
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.println("Scanning...");

  scrollOffset = 0;

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

  int startIndex = scrollOffset;
  int endIndex = min(startIndex + MAX_VISIBLE_NETWORKS, networkCount);

  // Loops through the discpvered networks and displays the SSID and RSSI
  for (int i = startIndex; i < endIndex; i++) {
    String ssid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);
    wifi_auth_mode_t security = WiFi.encryptionType(i);

    // Get security type as a string
    String securityType = getEncryptionType(security);

    // Choose a color based on the encryption type
    uint16_t color = getSecurityColor(security);

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

  drawScrollIndicator();
}


void scrollDown() {
  // Sets max scroll offset
  int maxOffset = max(0, networkCount - MAX_VISIBLE_NETWORKS);

  // TODO: Find a way to prevent scrollOffset from becoming negative
  // Long Press: Hold button to scroll faster??? 

  if (scrollOffset < maxOffset) {
    // Increase the scrollOffset
    scrollOffset += MAX_VISIBLE_NETWORKS;
    displayNetworks();
  } else {
    // Wrap Around: Goes back to the start of the list if at end
    scrollOffset = 0;
    displayNetworks();
  }

}

void scrollUp() {
  if (scrollOffset > 0) {
    // Decrease the scrollOffset
    // Prevents scrollOffset from becoming negative
    scrollOffset = max(0, scrollOffset - MAX_VISIBLE_NETWORKS);
    displayNetworks();
  }
}

// Displays the page number
void drawScrollIndicator() {
  M5.Lcd.drawLine(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, SCREEN_HEIGHT - 20, WHITE);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setCursor(10, SCREEN_HEIGHT - 15);

  int currentPage = scrollOffset / MAX_VISIBLE_NETWORKS + 1;
  int totalPages = (networkCount + MAX_VISIBLE_NETWORKS - 1) / MAX_VISIBLE_NETWORKS;

  M5.Lcd.printf("Page %d/%d", currentPage, totalPages);

}

// Security Colors for wifi encryption types
uint16_t getSecurityColor(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      return TFT_RED;
    case WIFI_AUTH_WEP:
      return TFT_ORANGE;
    case WIFI_AUTH_WPA_PSK:
      return TFT_YELLOW;
    case WIFI_AUTH_WPA2_PSK:
    case WIFI_AUTH_WPA_WPA2_PSK:
    case WIFI_AUTH_WPA3_PSK:
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return TFT_GREEN;  
    default:
      return TFT_WHITE;
  }
}

// Returns the ecryption type
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