#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFi.h>

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
char keys[ROWS][COLS] = {{'1', '2', '3', 'A'}, {'4', '5', '6', 'B'}, {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {26, 25, 33, 32};
byte colPins[COLS] = {13, 12, 14, 27};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String uid = "";

// Cek koneksi Wi-Fi
void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to Wi-Fi...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("\nWi-Fi connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
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
  return true;
}

String bacaRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return ""; // Jika tidak ada kartu baru yang terdeteksi, kembalikan string kosong
  }

  String teks = ""; // Variabel untuk menyimpan UID
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) teks += "0"; // Tambahkan leading zero jika nilai byte kurang dari 16
    teks += String(rfid.uid.uidByte[i], HEX);   // Konversi byte ke format heksadesimal
  }
  teks.toUpperCase(); // Ubah UID menjadi huruf kapital
  Serial.println("UID: " + teks); // Tampilkan UID ke serial monitor
  return teks; // Kembalikan UID sebagai hasil
}

// Cek apakah kartu sudah terdaftar
bool checkCardStatus(const String& uid) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, registerUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String postData = "uid=" + uid + "&check=true";
  int httpCode = http.POST(postData);
  String response = http.getString();
  http.end();

  if (httpCode == HTTP_CODE_OK) {
    return response.indexOf("exists") >= 0;  // Respon berisi "exists" jika terdaftar
  }
  return false;
}

// Daftarkan kartu baru
void registerCard(const String& uid) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, registerUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String postData = "uid=" + uid;
  int httpCode = http.POST(postData);
  String response = http.getString();
  http.end();

  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Server Response: " + response);
  } else {
    Serial.println("Error registering card: " + String(httpCode));
  }
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
  } else {
    Serial.println("Error sending data: " + String(httpCode));
  }
}

// Fungsi lihat data
void lihatData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;

    String lihat_uid = bacaRFID(); // Membaca UID dari kartu RFID

    if (lihat_uid != "") { // Periksa apakah UID terbaca
      // Buat URL dengan UID yang dibaca
      String lihatUrl = String(lihatDataUrl) + "?uid=" + lihat_uid;
      Serial.print("UID yang dikirim: ");
      Serial.println(lihat_uid); // Tampilkan UID ke Serial Monitor
      Serial.print("URL yang dikirim: ");
      Serial.println(lihatUrl); // Tampilkan URL ke Serial Monitor

      http.begin(client, lihatUrl);  // Gunakan URL lengkap dengan UID
      int httpResponseCode = http.GET(); // Kirimkan permintaan GET ke server

      if (httpResponseCode > 0) { // Jika HTTP response berhasil
        String payload = http.getString(); // Ambil respon dari server
        Serial.println("Data dari server:");
        Serial.println(payload);

        // Parsing JSON
        DynamicJsonDocument doc(256); // Buffer size (sesuaikan dengan besar JSON)
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) { // Jika parsing JSON berhasil
          const char* created_at = doc["created_at"];
          float berat = doc["weight"];
          float tinggi = doc["height"];

          Serial.print("Created At: ");
          Serial.println(created_at);
          Serial.print("Berat: ");
          Serial.println(berat);
          Serial.print("Tinggi: ");
          Serial.println(tinggi);
        } else {
          Serial.print("Gagal mem-parsing JSON: ");
          Serial.println(error.c_str());
        }
      } else { // Jika HTTP response gagal
        Serial.print("Error mengambil data, kode: ");
        Serial.println(httpResponseCode);
      }

      http.end(); // Menutup koneksi HTTP
    } else {
      Serial.println("RFID tidak terbaca");
    }
  } else {
    Serial.println("WiFi tidak terhubung");
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  checkWiFiConnection();
  Serial.println("System Ready.");
}

void loop() {
  checkWiFiConnection();
  if (scanRFID()) {
    Serial.println("Kartu berhasil discan.");
    Serial.println("UID: " + uid);
    Serial.println("Pilih opsi:");
    Serial.println("Tekan 'B' untuk tambah data.");
    Serial.println("Tekan 'C' untuk lihat data.");
    Serial.println("Tekan 'D' untuk cancel.");

    while (true) {
      char key = keypad.getKey();
      if (key == 'B') {
        if (checkCardStatus(uid)) {
          Serial.println("Kartu terdaftar. Silakan masukkan berat dan tinggi.");
          String height = readFromKeypad("Input height (end with #):");
          String weight = readFromKeypad("Input weight (end with #):");
          sendData(height, weight);
        } else {
          Serial.println("Kartu belum terdaftar. Hubungi admin untuk pendaftaran kartu.");
        }
        break;  // Keluar dari loop opsi
      } else if (key == 'C') {
        lihatData();  // Memanggil fungsi lihat data
        break;        // Keluar dari loop opsi
      } else if (key == 'D') {
        Serial.println("Proses dibatalkan. Siap untuk scan kartu berikutnya.");
        break;  // Keluar dari loop opsi
      }
    }

    Serial.println("Tekan 'A' untuk mengulangi proses dari awal.");
    while (true) {
      char key = keypad.getKey();
      if (key == 'A') {
        Serial.println("Mengulangi proses dari awal...");
        break;  // Keluar dari loop dan kembali ke awal
      }
    }
  }
  delay(1000);
}
