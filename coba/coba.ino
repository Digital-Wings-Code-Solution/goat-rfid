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
const char *registerUrl = "http://192.168.1.201/coba/register_rfid.php";
const char *sendDataUrl = "http://192.168.1.201/coba/send_data.php";
const char *lihatDataUrl = "http://192.168.1.201/coba/lihat.php";

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
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {26, 25, 33, 32};
byte colPins[COLS] = {13, 12, 14, 27};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Konfigurasi LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // Alamat I2C bisa 0x27 atau 0x3F
String uid = "";

/**
 * Cek koneksi Wi-Fi
 */
void checkWiFiConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Reconnecting to Wi-Fi...");
    lcd.clear();
    lcd.print("Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      Serial.print(".");
      lcd.print(".");
    }
    Serial.println("\nWi-Fi connected.");
    lcd.clear();
    lcd.print("WiFi Connected!");
  }
}

/**
 * Scan RFID dan mendapatkan UID
 * @return true jika kartu baru terdeteksi, false jika tidak
 */
bool scanRFID()
{
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
  {
    return false;
  }
  uid = "";
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    if (rfid.uid.uidByte[i] < 0x10)
      uid += "0";
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

/**
 * Cek apakah kartu sudah terdaftar
 * @param uid UID dari kartu RFID
 * @return true jika kartu terdaftar, false jika tidak
 */
bool checkCardStatus(const String &uid)
{
  WiFiClient client;
  HTTPClient http;
  http.begin(client, registerUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String postData = "uid=" + uid + "&check=true";
  int httpCode = http.POST(postData);
  String response = http.getString();
  http.end();

  if (httpCode == HTTP_CODE_OK)
  {
    return response.indexOf("exists") >= 0; // Respon berisi "exists" jika terdaftar
  }
  return false;
}

/**
 * Input data dari keypad
 * @param prompt pesan yang ditampilkan untuk meminta input
 * @return input dari keypad
 */
String readFromKeypad(const String &prompt)
{
  Serial.println(prompt);
  String input = "";
  char key;
  while (true)
  {
    key = keypad.getKey();
    if (key)
    {
      if (key == '#')
        break; // Selesai
      if (key == '*')
        input = ""; // Hapus input
      else
        input += key;
      Serial.print("Input: ");
      Serial.println(input);
    }
  }
  return input;
}

/**
 * Kirim data berat dan tinggi ke server
 * @param height tinggi badan
 * @param weight berat badan
 */
void sendData(const String &height, const String &weight)
{
  WiFiClient client;
  HTTPClient http;
  http.begin(client, sendDataUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String postData = "uid=" + uid + "&height=" + height + "&weight=" + weight;
  int httpCode = http.POST(postData);
  String response = http.getString();
  http.end();

  if (httpCode == HTTP_CODE_OK)
  {
    Serial.println("Server Response: " + response);
    lcd.clear();
    lcd.print("Data Sent!");
    delay(2000);
  }
  else
  {
    Serial.println("Error sending data: " + String(httpCode));
    lcd.clear();
    lcd.print("Send Error!");
    delay(2000);
  }
}

/**
 * Fungsi untuk melihat data dari server
 */
void lihatData()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClient client;
    HTTPClient http;
    String lihatUrl = String(lihatDataUrl) + "?uid=" + uid;
    http.begin(client, lihatUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
      String payload = http.getString();
      Serial.println("Data dari server:");
      Serial.println(payload);

      // Parsing JSON
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error)
      {
        if (doc.containsKey("error"))
        {
          // Jika ada error dari server
          Serial.println("Error: " + String(doc["error"].as<const char *>()));
          lcd.clear();
          lcd.print("Data: Not Found");
        }
        else
        {
          const char *created_at = doc["created_at"];
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
          lcd.print("Berat:  " + String(berat) + " kg");
          lcd.setCursor(0, 1);
          lcd.print("Tinggi: " + String(tinggi) + " cm");
          delay(3000);
        }
      }
      else
      {
        Serial.println("Gagal mem-parsing JSON.");
        lcd.clear();
        lcd.print("JSON Error!");
        delay(2000);
      }
    }
    else
    {
      Serial.print("Error mengambil data, kode: ");
      Serial.println(httpResponseCode);
      lcd.clear();
      lcd.print("Error GET!");
      delay(2000);
    }

    http.end();
  }
  else
  {
    Serial.println("WiFi tidak terhubung.");
    lcd.clear();
    lcd.print("No WiFi!");
    delay(2000);
  }

  // Tunggu hingga ada tombol yang ditekan untuk kembali ke menu utama
  lcd.clear();
  lcd.print("Press any key");

  while (keypad.getKey() == NO_KEY)
  {
    delay(100);
  }
  lcd.clear();
}

/**
 * Fungsi setup untuk inisialisasi
 */
void setup()
{
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

/**
 * Fungsi loop utama
 */
void loop()
{
  checkWiFiConnection();
  if (scanRFID())
  {
    Serial.println("Kartu berhasil discan.");
    Serial.println("Pilih opsi:");
    Serial.println("B: Tambah data");
    Serial.println("C: Lihat data");
    Serial.println("D: Cancel");

    lcd.clear();
    lcd.print("Pilih: B, C, D");

    while (true)
    {
      char key = keypad.getKey();
      if (key == 'B')
      {
        if (checkCardStatus(uid))
        {
          Serial.println("Kartu terdaftar. Silakan masukkan berat dan tinggi.");
          String height = readFromKeypad("Input height (end with #):");
          String weight = readFromKeypad("Input weight (end with #):");
          sendData(height, weight);
        }
        else
        {
          Serial.println("Kartu belum terdaftar. Hubungi admin untuk pendaftaran kartu.");
        }
        break; // Keluar dari loop opsi
      }
      else if (key == 'C')
      {
        lihatData(); // Memanggil fungsi lihat data
        break;       // Keluar dari loop opsi
      }
      else if (key == 'D')
      {
        Serial.println("Proses dibatalkan. Siap untuk scan kartu berikutnya.");
        break; // Keluar dari loop opsi
      }
    }
  }
  delay(1000);
}