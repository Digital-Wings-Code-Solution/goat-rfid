#include <HTTPClient.h>
#include <WiFi.h>
#include <keypad.h>

#define ROW_NUM

// Network SSID
const char* ssid = "Digital Wings Code Solution";
const char* password = "sijitekan8";
const char* host = "192.168.1.7";

void setup() {
  Serial.begin(9600);

  WiFi.hostname("NodeMCU");
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println(".");
  }

  Serial.println("WIfi Connected");
  Serial.println("IP Address : ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:

}
