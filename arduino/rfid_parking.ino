/*
RFID Attendance System with IR Obstacle Detection
Author: NOUREDDINE AKOUCHAH
GitHub:https://github.com/noureddine-akouchah
Version: 1.0
Description: RFID-based attendance system with IR obstacle detection
             and relay control for security applications.
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin Definitions
#define SS_PIN 10
#define RST_PIN 9
#define RED_LED 6
#define GREEN_LED 5
#define BUZZER 8
#define RELAY 7
#define IR_SENSOR 2

// LCD Configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Use 0x3F if 0x27 doesn't work

// RFID Object
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ============================================================================
// CARD CONFIGURATION - REPLACE WITH YOUR CARD UIDs
// ============================================================================
// To find your card UID, upload a simple RFID reader sketch first
// Example: File -> Examples -> MFRC522 -> DumpInfo

byte authorizedCards[][4] = {
  {0x00, 0x00, 0x00, 0x00},  // Card 1 - Replace with your UID
  {0x00, 0x00, 0x00, 0x00},  // Card 2 - Replace with your UID
  {0x00, 0x00, 0x00, 0x00},  // Card 3 - Replace with your UID
  {0x00, 0x00, 0x00, 0x00}   // Card 4 - Replace with your UID
};

// User Names for each card
String userNames[] = {
  "Person A",
  "Person B",
  "Person C",
  "Person D"
};

// User IDs for each card
int userIDs[] = {1001, 1002, 1003, 1004};

// ============================================================================
// SYSTEM VARIABLES - DO NOT MODIFY
// ============================================================================
bool registeredCards[4] = {false};
bool obstacleDetected = false;
unsigned long relayStartTime = 0;
const unsigned long RELAY_DURATION = 1500;  // Relay active duration in milliseconds

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
bool compareUID(byte *a, byte *b);
void processValidCard(int index);
void processInvalidCard();
void updateDisplay();

// ============================================================================
// SETUP FUNCTION
// ============================================================================
void setup() {
  // Initialize Serial Communication
  Serial.begin(9600);
  
  // Initialize SPI and RFID
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.print("RFID System v1.0");
  delay(1500);
  lcd.clear();
  lcd.print("Ready...");
  
  // Configure Pins
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(IR_SENSOR, INPUT);
  
  // Set Initial States
  digitalWrite(RELAY, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  
  // Initialize Data Logging Header
  Serial.println("CLEARSHEET");
  Serial.println("LABEL,Date,Time,Name,ID,Status");
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  // ========== IR Obstacle Detection ==========
  int irValue = digitalRead(IR_SENSOR);
  
  // Activate relay when obstacle detected
  if (irValue == LOW && !obstacleDetected) {
    obstacleDetected = true;
    relayStartTime = millis();
    digitalWrite(RELAY, HIGH);
  }
  
  // Deactivate relay after specified duration
  if (obstacleDetected && (millis() - relayStartTime >= RELAY_DURATION)) {
    digitalWrite(RELAY, LOW);
    obstacleDetected = false;
  }
  
  // ========== RFID Processing ==========
  updateDisplay();
  
  // Check for new card present
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    int cardIndex = -1;
    
    // Search for matching card
    for (int i = 0; i < 4; i++) {
      if (compareUID(mfrc522.uid.uidByte, authorizedCards[i])) {
        cardIndex = i;
        break;
      }
    }
    
    // Process card based on authorization
    if (cardIndex != -1) {
      processValidCard(cardIndex);
    } else {
      processInvalidCard();
    }
    
    mfrc522.PICC_HaltA();
  }
  
  delay(100);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Compare two UIDs byte by byte
bool compareUID(byte *a, byte *b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

// Process authorized card
void processValidCard(int index) {
  lcd.clear();
  
  if (!registeredCards[index]) {
    // First time registration
    registeredCards[index] = true;
    
    lcd.print("Welcome:");
    lcd.setCursor(0, 1);
    lcd.print(userNames[index]);
    
    // Log attendance data to serial
    Serial.print("DATA,DATE,TIME,");
    Serial.print(userNames[index]);
    Serial.print(",");
    Serial.print(userIDs[index]);
    Serial.println(",REGISTERED");
    
    // Success feedback
    digitalWrite(GREEN_LED, HIGH);
    tone(BUZZER, 2000, 200);
    delay(2000);
    digitalWrite(GREEN_LED, LOW);
  } else {
    // Already registered
    lcd.print("Already Registered");
    lcd.setCursor(0, 1);
    lcd.print(userNames[index]);
    
    // Warning feedback
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 1000, 500);
    delay(2000);
    digitalWrite(RED_LED, LOW);
  }
}

// Process unauthorized card
void processInvalidCard() {
  lcd.clear();
  lcd.print("Invalid Card!");
  
  // Error feedback
  digitalWrite(RED_LED, HIGH);
  tone(BUZZER, 1000, 1000);
  delay(2000);
  digitalWrite(RED_LED, LOW);
}

// Update LCD display periodically
void updateDisplay() {
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > 2000) {
    lcd.clear();
    lcd.print("Scan Your Card");
    lastUpdate = millis();
  }
}
