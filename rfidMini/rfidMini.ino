#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

// WiFi credentials
const char* ssid = "IoT";
const char* password = "percobaan";

// Server PHP URL
const char* serverName = "http://192.168.1.201/coba/rfid_data.php"; // Ganti dengan IP server Anda

// RFID pins
#define RST_PIN 2
#define SS_PIN 5

MFRC522 rfid(SS_PIN, RST_PIN);

void handleRFID() {
  // Check for new RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Get UID
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      uid += "0"; // Add leading zero if needed
    }
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  Serial.println("UID: " + uid);

  // Send UID to server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "uid=" + uid;
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      Serial.println("Server Response: " + http.getString());
    } else {
      Serial.println("Error in sending POST: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected!");
  }  
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  handleRFID();

  // Halt for a moment
  delay(1000);
}
