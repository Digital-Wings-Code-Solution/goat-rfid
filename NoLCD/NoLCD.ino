#include <Ethernet.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 49

MFRC522 rfid(SS_PIN, RST_PIN);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };      // Setting MAC Address

char server[] = "192.168.37.68";                          // IP address Komputer
IPAddress ip(192,168,37,177);                             // IP address Ethernet Shield
EthernetClient client; 

void setup() {
  Serial.begin(9600);
  SPI.begin();
  Serial.println("Tunggu Sejenak");
  rfid.PCD_Init();
    if (Ethernet.begin(mac) == 0) {
    Serial.println("Tempelkan Kartu");
    Ethernet.begin(mac, ip);
    }
    else{
    Serial.println("Failed to configure Ethernet using DHCP");
    }
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.print("UID tag :");
    String content = "";
    byte letter;
    for (byte i = 0; i < rfid.uid.size; i++) {
      //Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(rfid.uid.uidByte[i], HEX);
      //content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(rfid.uid.uidByte[i], HEX));
    }
  content.toUpperCase();
  delay(3000);
  Sending_To_phpmyadmindatabase(content.substring(1));    // CONNECTING WITH MYSQL
  }
}

void Sending_To_phpmyadmindatabase(String UID) 
{
  if (client.connect(server, 80)) {                        
    Serial.println();
    Serial.println("Connected");
    // Make a HTTP request:
    client.print("GET localhost/testcode/3Acc_CatatFix.php?");     // YOUR URL
    client.print("UID=");
    client.print(UID);                                    // Pembacaan Digit pertama UID
    client.print(" ");                                    // SPACE BEFORE HTTP/1.1
    client.print("HTTP/1.1");
    client.println();
    client.println("Host: 192.168.37.68");                // IP Adress Komputer
    client.println("Connection: close");
    client.println();
  } 
  else {
    Serial.println();
    Serial.println("Connection Failed");
    }
  }

//testcode/3Acc_CatatFix.php?UID=E1%20F9%20AA%20