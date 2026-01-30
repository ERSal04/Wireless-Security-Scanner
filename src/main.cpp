#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>

// Bluetooth Libraries
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// Define the constants for the display screen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define LINE_HEIGHT 20

#define MAX_VISIBLE_NETWORKS 9
#define MAX_BLE_DEVICES 50

// Wi-Fi variables
int networkCount = 0;
int scrollOffset = 0; // Index of the first network to display

// BLE variables
int bleDeviceCount = 0;
String bleDeviceNames[MAX_BLE_DEVICES];
String bleDeviceMACs[MAX_BLE_DEVICES];
int bleDeviceRSSI[MAX_BLE_DEVICES];

// Mode tracking
enum ScanMode { WIFI_MODE, BLE_MODE };
ScanMode currentMode = WIFI_MODE;

// BLE scanner
BLEScan* pBLEScan;

// Forward declarations
void performWifiScan();
void performBLEScan();
void displayNetworks();
void displayBLEDevices();
void scrollDown();
void scrollUp();
void drawScrollIndicator();
void switchMode();

uint16_t getSecurityColor(wifi_auth_mode_t);
String getEncryptionType(wifi_auth_mode_t);
uint16_t getRSSIColor(int rssi);


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (bleDeviceCount < MAX_BLE_DEVICES) {
      String name = advertisedDevice.getName().c_str();

      bleDeviceNames[bleDeviceCount] = advertisedDevice.getName().c_str();
      bleDeviceMACs[bleDeviceCount] = advertisedDevice.getAddress().toString().c_str();
      bleDeviceRSSI[bleDeviceCount] = advertisedDevice.getRSSI();

      bleDeviceCount++;
    }
  }
};

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

  // Initialize Wi-Fi
  WiFi.mode(WIFI_STA); 
  WiFi.disconnect();

  BLEDevice::init("M5Scanner");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
    
}

void loop() {
  M5.update(); // Must call this to update button states

  // Check for mode switch (Long press Button A)
  if (M5.BtnA.pressedFor(1000)) {
    switchMode();
    delay(500); //Debounce
  }
  // Normal press: Scan in current mod
  else if (M5.BtnA.wasPressed()) {
    if (currentMode == WIFI_MODE) {
      performWifiScan();
    } else {
      performBLEScan();
    }
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

void switchMode() {
  // Toggle between WIFI_MODE and BLE_MODE
  if (currentMode == WIFI_MODE) {
    currentMode = BLE_MODE;
  } else {
    currentMode = WIFI_MODE;
  }

  scrollOffset = 0;  // Reset scroll when switching modes
  
  // Show mode indicator
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setCursor(60, 100);
  
  if (currentMode == WIFI_MODE) {
      M5.Lcd.println("Wi-Fi Mode");
  } else {
      M5.Lcd.println("BLE Mode");
  }
  
  delay(1000);
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

void performBLEScan() {
  bleDeviceCount = 0;
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.println("Scanning BLE...");
  
  scrollOffset = 0;
  
  int scanTime = 5;
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  
  displayBLEDevices();
}

void displayBLEDevices() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  
  // Header
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.printf("Found %d BLE devices\n", bleDeviceCount);
  M5.Lcd.drawLine(0, 15, SCREEN_WIDTH, 15, WHITE);
  
  int yPos = 20;
  
  int startIndex = scrollOffset;
  int endIndex = min(startIndex + MAX_VISIBLE_NETWORKS, bleDeviceCount);
  
  for (int i = startIndex; i < endIndex; i++) {
      String name = bleDeviceNames[i];
      String mac = bleDeviceMACs[i];
      int rssi = bleDeviceRSSI[i];

      uint16_t color = getRSSIColor(rssi);
      
      M5.Lcd.setTextColor(color);
      M5.Lcd.setCursor(5, yPos);
      
      // Display device info
      if (name.length() > 0) {
          // Has a name
          M5.Lcd.printf("%-20s  | ", name);
      } else {
          // No name, show MAC
          M5.Lcd.printf("%s | ", mac);
      }
      
      M5.Lcd.printf(" %ddBm\n", rssi);
      
      yPos += LINE_HEIGHT;
  }

  drawScrollIndicator();
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
  int maxOffset;

  if (currentMode == WIFI_MODE) {
    maxOffset = max(0, networkCount - MAX_VISIBLE_NETWORKS);  
  } else {
    // TODO: Calculate maxOffset for BLE mode
    maxOffset = max(0, bleDeviceCount - MAX_VISIBLE_NETWORKS);
  }

  if (scrollOffset < maxOffset) {
    // Increase the scrollOffset
    scrollOffset += MAX_VISIBLE_NETWORKS;
  } else {
    // Wrap Around: Goes back to the start of the list if at end
    scrollOffset = 0;
  }
  
  if (currentMode == WIFI_MODE) {
    displayNetworks();
  } else {
    displayBLEDevices();
  }

}

void scrollUp() {
  if (scrollOffset > 0) {
    // Decrease the scrollOffset
    scrollOffset = max(0, scrollOffset - MAX_VISIBLE_NETWORKS);
    // displayNetworks();

    if (currentMode == WIFI_MODE) {
      displayNetworks();
    } else {
      displayBLEDevices();
    }
  }
}

// Displays the page number
void drawScrollIndicator() {
  M5.Lcd.drawLine(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, SCREEN_HEIGHT - 20, WHITE);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setCursor(10, SCREEN_HEIGHT - 15);

  int totalItems = (currentMode == WIFI_MODE) ? networkCount : bleDeviceCount;
  int currentPage = scrollOffset / MAX_VISIBLE_NETWORKS + 1;
  int totalPages = (totalItems + MAX_VISIBLE_NETWORKS - 1) / MAX_VISIBLE_NETWORKS;

  String modeStr = (currentMode == WIFI_MODE) ? "WiFi" : "BLE";
  M5.Lcd.printf("%s | Page %d/%d", modeStr.c_str(), currentPage, totalPages);

}

uint16_t getRSSIColor(int rssi) {
    if (rssi > -50) return TFT_GREEN;      
    else if (rssi > -70) return TFT_YELLOW; 
    else if (rssi > -85) return TFT_ORANGE; 
    else return TFT_RED;                    
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