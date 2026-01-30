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
void initBLE();
String formatMAC(String);

uint16_t getSecurityColor(wifi_auth_mode_t);
String getEncryptionType(wifi_auth_mode_t);
uint16_t getRSSIColor(int rssi);

// TODO: Create a callback class for BLE scanning
// Will be called whenever a BLE device is found

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (bleDeviceCount < MAX_BLE_DEVICES) {
        
        if (advertisedDevice.haveName()) {
          bleDeviceNames[bleDeviceCount] = advertisedDevice.getName().c_str();
        } else {
          bleDeviceNames[bleDeviceCount] = "";
        }

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
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setCursor(40, 80);
  M5.Lcd.println("Security Scanner");

  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(40, 120);
  M5.Lcd.println("A: Scan | B/C: Scroll");
  M5.Lcd.setCursor(40, 140);
  M5.Lcd.println("Touch: Switch Wi-Fi/BLE");

  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setCursor(40, 170);
  M5.Lcd.println("Starting in Wi-Fi mode...");

  // Initialize Wi-Fi
  WiFi.mode(WIFI_STA); 
  WiFi.disconnect();

  delay(2000);

}

void loop() {
  M5.update(); // Must call this to update button states

  static unsigned long lastTouchTime = 0;
  if (M5.Touch.ispressed()) {
    if (millis() - lastTouchTime > 1000) {
      switchMode();
      lastTouchTime = millis();
    }
  }

  // Normal press: Scan in current mod
  if (M5.BtnA.wasPressed()) {
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

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    btStart();
    delay(100);
    initBLE();

  } else {
    currentMode = WIFI_MODE;

    if (pBLEScan) {
      pBLEScan->clearResults();
    }

    btStop();
    delay(100);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
  }

  scrollOffset = 0;  // Reset scroll when switching modes
  
  // Show mode indicator
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setCursor(60, 100);
  
  if (currentMode == WIFI_MODE) {
      M5.Lcd.println("Wi-Fi Mode");
  } else {
      M5.Lcd.println("BLE Mode");
  }

  M5.Axp.SetLDOEnable(3, true);
  delay(50);
  M5.Axp.SetLDOEnable(3, false);
  
  delay(800);
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

  if (networkCount == 0) {
    M5.Lcd.setCursor(60, 100);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("No devices found");
    M5.Lcd.setCursor(40, 120);
    M5.Lcd.println("Press A to scan again");
    return;
  }

  displayNetworks();
}

void performBLEScan() {
  // Reset BLE device arrays
  bleDeviceCount = 0;

  pBLEScan->clearResults();
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(60, 100);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.println("Scanning BLE...");
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(80, 130);
  M5.Lcd.println("(5 seconds)");

  scrollOffset = 0;
  
  // Start BLE scan
  int scanTime = 5;  // seconds
  pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();

  if (bleDeviceCount == 0) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(50, 100);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("No BLE devices found");
    M5.Lcd.setCursor(40, 120);
    M5.Lcd.println("Press A to scan again");
    delay(2000);
    return;
  }

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
  
  // Calculate start and end indices for scrolling
  int startIndex = scrollOffset;
  int endIndex = min(startIndex + MAX_VISIBLE_NETWORKS, bleDeviceCount);
  
  for (int i = startIndex; i < endIndex; i++) {
      String name = bleDeviceNames[i];
      String mac = bleDeviceMACs[i];
      int rssi = bleDeviceRSSI[i];

      // Choose color based on signal strength
      uint16_t color = getRSSIColor(rssi);
      
      M5.Lcd.setTextColor(color);
      M5.Lcd.setCursor(5, yPos);
      
      // Display device info
      if (name.length() > 0) {
          // Has a name
          if (name.length() > 18) {
            name = name.substring(0, 15) + "...";
          }
          M5.Lcd.printf("%-18s", name.c_str());
        } else {
          // No name, show MAC
          M5.Lcd.printf("%-18s | ", formatMAC(mac).c_str());
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
    // displayNetworks();
  } else {
    // Wrap Around: Goes back to the start of the list if at end
    scrollOffset = 0;
    // displayNetworks();
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

// Create a function to color-code by signal strength
uint16_t getRSSIColor(int rssi) {
  if (rssi > -50) return TFT_GREEN;
    else if (rssi > -70) return TFT_YELLOW;
    else if (rssi > -85) return TFT_ORANGE;
    else return TFT_RED;
}

void initBLE() {
  BLEDevice::init("M5Scanner");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

String formatMAC(String mac) {
  if (mac.length() > 8) {
    return "..." + mac.substring(mac.length() - 8);
  }
  return mac;
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