#include <SPI.h>
#include <MFRC522.h>

#include <HTTPClient.h>
#include <WiFi.h>

long previousMillis = 0;
long interval = 1000;

// Network SSID
const char* ssid = "IoT";
const char* password = "percobaan";

const char* host = "192.168.1.201";  //IP Komputer / server (disesuaikan)

#define SDA_PIN 5  //D4
#define RST_PIN 2  //D3
#define LED_PIN 15 //D8
#define BTN_PIN 0  //D1
#define LED_PIN2 4  //D2

MFRC522 mfrc522(SDA_PIN, RST_PIN) ;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(BTN_PIN, OUTPUT);
  WiFi.hostname("NodeMCU");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN2, LOW);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Dekatkan Kartu RFID Anda ke Reader");
  Serial.println();

  digitalWrite(LED_PIN2, HIGH);
}

void loop() {
  WiFiClient client;
  if (digitalRead(BTN_PIN) == 1)
  {
    while (digitalRead(BTN_PIN) == 1);
    digitalWrite(LED_PIN, HIGH);
    String getData, Link;
    HTTPClient http; 
    Link = "http://" + String(host) + "/absensi/ubahmode.php";
    http.begin(client, Link);

    int httpCode = http.GET(); 
    String payload = http.getString(); 

    Serial.println(payload); 
    http.end(); 

  }

  digitalWrite(LED_PIN, LOW);
  if ( ! mfrc522.PICC_IsNewCardPresent() )
    return;

  if ( ! mfrc522.PICC_ReadCardSerial() )
    return;

  String IDTAG = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    IDTAG += mfrc522.uid.uidByte[i];
  }

  digitalWrite(LED_PIN, HIGH);
  Serial.println( IDTAG );

  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  //Serial.println("connected");

  String getData, Link;
  HTTPClient http;
  //GET Data
  Link = "http://"+String(host)+"/absensi/kirimkartu.php?nokartu=" + IDTAG;
  http.begin(client, Link);

  int httpCode = http.GET();
  String payload = http.getString();

  Serial.println(payload);
  http.end();

  delay(2000);
}
