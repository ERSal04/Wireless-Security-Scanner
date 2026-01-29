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

// TODO: Create a callback class for BLE scanning
// Will be called whenever a BLE device is found

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // TODO: This function is called for EACH device found
    // Need to:
    // 1. Check if we have room in our arrays (bleDeviceCount < MAX_BLE_DEVICES)
      // 2. Store the device name (or MAC if no name)
      // 3. Store the MAC address
      // 4. Store the RSSI
      // 5. Increment bleDeviceCount
      
      // HINT: advertisedDevice.getName() gets the name (might be empty!)
      // HINT: advertisedDevice.getAddress().toString() gets MAC
      // HINT: advertisedDevice.getRSSI() gets signal strength
      
      // YOUR CODE HERE
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

  // TODO: Initialize BLE
  // HINT: BLEDevice::init("M5Scanner");
  // HINT: Create a scan object: pBLEScan = BLEDevice::getScan()
  // HINT: Set the callback: pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks())
  // HINT: Configure scan: pBLEScan->setActiveScan(true)
  // HINT: Set scan interval: pBLEScan->setInterval(100)
  // HINT: Set scan window: pBLEScan->setWindow(99)
  
  // YOUR CODE HERE
  
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
  // TODO: Toggle between WIFI_MODE and BLE_MODE
  // HINT: if (currentMode == WIFI_MODE) { currentMode = BLE_MODE; } else { ... }
  
  // YOUR CODE HERE
  
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
   // TODO: Reset BLE device arrays
  // HINT: Set bleDeviceCount = 0
  // This clears previous scan results
  
  // YOUR CODE HERE
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.println("Scanning BLE...");
  
  scrollOffset = 0;
  
  // TODO: Start BLE scan
  // HINT: pBLEScan->start(scanTime, false)
  // scanTime = how long to scan (try 5 seconds)
  // false = don't delete results after scan
  
  int scanTime = 5;  // seconds
  // YOUR CODE HERE
  
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
  
  // TODO: Calculate start and end indices for scrolling
  // HINT: Same logic as Wi-Fi display
  int startIndex = scrollOffset;
  int endIndex = /* YOUR CODE HERE */;
  
  for (int i = startIndex; i < endIndex; i++) {
      // TODO: Get device info from arrays
      String name = /* YOUR CODE HERE */;
      String mac = /* YOUR CODE HERE */;
      int rssi = /* YOUR CODE HERE */;

      // Choose color based on signal strength
      uint16_t color = getRSSIColor(rssi);
      
      M5.Lcd.setTextColor(color);
      M5.Lcd.setCursor(5, yPos);
      
      // TODO: Display device info
      // If device has no name, show MAC address
      // Format: "DeviceName" or "[MAC]"
      //         "RSSI: -65 dBm"
      
      if (name.length() > 0) {
          // Has a name
          // YOUR CODE HERE - truncate if > 20 chars
      } else {
          // No name, show MAC
          // YOUR CODE HERE
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
    maxOffset = ;
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
  int totalPages = (networkCount + MAX_VISIBLE_NETWORKS - 1) / MAX_VISIBLE_NETWORKS;

  String modeStr = (currentMode == WIFI_MODE) ? "WiFi" : "BLE";
  M5.Lcd.printf("%s | Page %d/%d", modeStr.c_str(), currentPage, totalPages);

}

// TODO: Create a function to color-code by signal strength
// Stronger signal = greener, weaker = redder
uint16_t getRSSIColor(int rssi) {
    // HINT: RSSI ranges typically from -30 (very close) to -90 (far away)
    // -30 to -50: GREEN (excellent)
    // -50 to -70: YELLOW (good)
    // -70 to -85: ORANGE (fair)
    // -85+: RED (poor)
    
    // YOUR CODE HERE
    
    return WHITE;  // default
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