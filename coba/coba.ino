#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

// Konfigurasi Wi-Fi
#define WIFI_SSID "IoT"
#define WIFI_PASSWORD "percobaan"

// URL server
const char* registerUrl = "http://192.168.1.201/coba/register_rfid.php";
const char* sendDataUrl = "http://192.168.1.201/coba/send_data.php";
const char* lihatDataUrl = "http://192.168.1.201/coba/lihat.php";

// Konfigurasi RFID
#define RST_PIN 2
#define SS_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

// Konfigurasi keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {26, 25, 33, 32};
byte colPins[COLS] = {13, 12, 14, 27};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Konfigurasi LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // Alamat I2C bisa 0x27 atau 0x3F
String uid = "";

// Cek koneksi Wi-Fi
void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to Wi-Fi...");
    lcd.clear();
    lcd.print("Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
      lcd.print(".");
    }
    Serial.println("\nWi-Fi connected.");
    lcd.clear();
    lcd.print("WiFi Connected!");
  }
}

// Scan RFID dan mendapatkan UID
bool scanRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return false;
  }
  uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  Serial.println("UID: " + uid);
  lcd.clear();
  lcd.print("UID: ");
  lcd.setCursor(0, 1);
  lcd.print(uid);
  return true;
}

// Input data dari keypad
String readFromKeypad(const String& prompt) {
  Serial.println(prompt);
  String input = "";
  char key;
  while (true) {
    key = keypad.getKey();
    if (key) {
      if (key == '#') break;  // Selesai
      if (key == '*')
        input = "";  // Hapus input
      else
        input += key;
      Serial.print("Input: ");
      Serial.println(input);
    }
  }
  return input;
}

// Kirim data berat dan tinggi
void sendData(const String& height, const String& weight) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, sendDataUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String postData = "uid=" + uid + "&height=" + height + "&weight=" + weight;
  int httpCode = http.POST(postData);
  String response = http.getString();
  http.end();

  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Server Response: " + response);
    lcd.clear();
    lcd.print("Data Sent!");
    delay(2000);
  } else {
    Serial.println("Error sending data: " + String(httpCode));
    lcd.clear();
    lcd.print("Send Error!");
    delay(2000);
  }
}

// Fungsi untuk melihat data
void lihatData() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    String lihatUrl = String(lihatDataUrl) + "?uid=" + uid;
    http.begin(client, lihatUrl);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Data dari server:");
      Serial.println(payload);

      // Parsing JSON
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        const char* created_at = doc["created_at"];
        float berat = doc["weight"];
        float tinggi = doc["height"];

        Serial.println("Detail Data:");
        Serial.print("Tanggal: ");
        Serial.println(created_at);
        Serial.print("Berat: ");
        Serial.print(berat);
        Serial.println(" kg");
        Serial.print("Tinggi: ");
        Serial.print(tinggi);
        Serial.println(" cm");

        lcd.clear();
        lcd.print("Berat:");
        lcd.print(berat);
        lcd.setCursor(0, 1);
        lcd.print("Tinggi:");
        lcd.print(tinggi);
        delay(3000);
      } else {
        Serial.println("Gagal mem-parsing JSON.");
        lcd.clear();
        lcd.print("JSON Error!");
        delay(2000);
      }
    } else {
      Serial.print("Error mengambil data, kode: ");
      Serial.println(httpResponseCode);
      lcd.clear();
      lcd.print("Error GET!");
      delay(2000);
    }

    http.end();
  } else {
    Serial.println("WiFi tidak terhubung.");
    lcd.clear();
    lcd.print("No WiFi!");
    delay(2000);
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  lcd.begin();
  lcd.backlight();
  lcd.print("Initializing...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  checkWiFiConnection();
  Serial.println("System Ready.");
  lcd.clear();
  lcd.print("Ready!");
}

void loop() {
  checkWiFiConnection();
  if (scanRFID()) {
    Serial.println("Kartu berhasil discan.");
    Serial.println("Pilih opsi:");
    Serial.println("B: Tambah data");
    Serial.println("C: Lihat data");
    Serial.println("D: Cancel");

    lcd.clear();
    lcd.print("Pilih: B, C, D");

    while (true) {
      char key = keypad.getKey();
      if (key == 'B') {
        lcd.clear();
        lcd.print("Input Data...");
        String height = readFromKeypad("Input height (end with #):");
        String weight = readFromKeypad("Input weight (end with #):");
        sendData(height, weight);
        break;
      } else if (key == 'C') {
        lihatData();
        break;
      } else if (key == 'D') {
        Serial.println("Proses dibatalkan.");
        lcd.clear();
        lcd.print("Canceled!");
        delay(2000);
        break;
      }
    }

    Serial.println("Siap untuk scan kartu berikutnya.");
    lcd.clear();
    lcd.print("Scan next card");
  }
  delay(1000);
}
