#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

// Konfigurasi WiFi
const char* ssid = "IoT";            // Ganti dengan SSID WiFi Anda
const char* password = "percobaan";  // Ganti dengan password WiFi Anda

// URL API
const char* serverName = "http://192.168.1.201/coba/lihat.php";  // Ganti IP sesuai server lokal Anda

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung");
}

void lihat(){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);             // Masukkan URL server API
    int httpResponseCode = http.GET();  // Deklarasi dan inisialisasi httpResponseCode

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Data dari server:");
      Serial.println(payload);

      // Parsing JSON
      DynamicJsonDocument doc(256);  // Buffer size (sesuaikan dengan besar JSON)
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        // Ambil data dari JSON
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
      // Parsing JSON jika perlu
    } else {
      Serial.print("Error mengambil data, kode: ");
      Serial.println(httpResponseCode);
    }

    http.end();  // Menutup koneksi HTTP
  } else {
    Serial.println("WiFi tidak terhubung");
  }
}

void loop() {
  lihat();
  delay(10000);  // Tunggu 10 detik sebelum request berikutnya
}
