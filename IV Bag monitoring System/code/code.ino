// ----------------------------
// Blynk Configuration
// ----------------------------
#define BLYNK_TEMPLATE_ID "TMPL3N8d1FwIb"
#define BLYNK_TEMPLATE_NAME "IV BAG"
#define BLYNK_AUTH_TOKEN "yiVCO85Z3QvP3pYJQBKpovLUA1Z8w4Bp"

// ----------------------------
// Libraries
// ----------------------------
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

// ----------------------------
// WiFi Credentials (Mobile Hotspot)
// ----------------------------
const char* ssid = "sharu";
const char* password = "12345678";

// ----------------------------
// Pins & Global Variables
// ----------------------------
#define LOADCELL_DOUT 4
#define LOADCELL_SCK  5

#define ALERT_PIN 2
#define LED_PIN   13

HX711 scale;
LiquidCrystal_I2C lcd(0x27, 20, 4);
BlynkTimer timer;

float scaleFactor = 414.598;   // Change after calibration

// ----------------------------
// Setup
// ----------------------------
void setup() {
  Serial.begin(115200);

  // LCD Initialization
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("IOT BASED IV BAG");
  lcd.setCursor(0, 1);
  lcd.print("MONITORING SYSTEM");
  delay(2500);
  lcd.clear();

  // Load Cell Initialization
  scale.begin(LOADCELL_DOUT, LOADCELL_SCK);
  scale.power_down();
  delay(100);
  scale.power_up();
  scale.set_scale(scaleFactor);
  scale.set_gain(128);
  delay(1500);
  scale.tare();
  Serial.println("Scale tare completed.");

  // Alert pins
  pinMode(ALERT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(ALERT_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // WiFi Connection
  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;

    if (attempts > 40) {  // 20 seconds
      Serial.println("\nWiFi Connection Failed!");
      return;
    }
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start Blynk AFTER WiFi is ready
  Blynk.config(BLYNK_AUTH_TOKEN);

  // Timer – runs every 3 seconds
  timer.setInterval(3000L, sendDataToBlynk);
}

// ----------------------------
// Loop
// ----------------------------
void loop() {
  Blynk.run();
  timer.run();
}

// ----------------------------
// Main Function to Send Data
// ----------------------------
void sendDataToBlynk() {
  float ml = scale.get_units(10);

  if (ml < 0) ml = 0;  // avoid negative noise

  float percentage = (ml / 500.0) * 100.0;
  if (percentage > 100) percentage = 100.0;

  // Serial Debug
  Serial.print("IV Level: ");
  Serial.print(ml, 2);
  Serial.print(" ml | ");
  Serial.print(percentage, 1);
  Serial.println("%");

  // LCD Display
  lcd.setCursor(0, 0);
  lcd.print("IV Bag Level:      ");

  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 1);
  lcd.print(ml, 2);
  lcd.print(" ml");

  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 2);
  lcd.print(percentage, 1);
  lcd.print("%");

  // Alert Condition
  if (percentage < 2.0 || ml < 5) {
    lcd.setCursor(0, 3);
    lcd.print("Refill Needed!");

    digitalWrite(ALERT_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);

    Blynk.virtualWrite(V1, "Refill Needed!");
    Blynk.logEvent("refill_alert", "Refill Needed! - PATIENT 1");
  } 
  else {
    lcd.setCursor(0, 3);
    lcd.print("                ");

    digitalWrite(ALERT_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    Blynk.virtualWrite(V1, "IV Bag OK");
  }

  // Send Data to Blynk
  Blynk.virtualWrite(V0, ml);
  Blynk.virtualWrite(V3, percentage);
}