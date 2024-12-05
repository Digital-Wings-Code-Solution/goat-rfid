//----------------------------------
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>  

LiquidCrystal_I2C lcd(0x27, 16, 2);          
//----------------------------------
int Li          = 17;
int Lii         = 1; 
int Ri          = -1;
int Rii         = -1;
//----------------------------------
void setup(){
  lcd.begin();                          
}
//---------------------------------- 
void loop(){
  for (int i = 0; i <= 12; i++) {
    lcd.setCursor(0, 0);
    lcd.print(Scroll_LCD_Right("IOT Kece"));
    delay(350);
  }
  for (int i = 0; i <= 50; i++) {                                                                         
    lcd.setCursor(0, 1);                                                
    lcd.print(Scroll_LCD_Left("Cara Membuat Text Berjalan........"));   
    delay(350);
  }
  Clear_Scroll_LCD_Left();                   
  for (int i = 16; i <= 30; i++) {
    lcd.setCursor(0, 0);
    lcd.print(Scroll_LCD_Right("IOT Kece"));
    delay(350);
  }
  Clear_Scroll_LCD_Right();
  for (int i = 0; i <= 12; i++) {
    lcd.setCursor(0, 0);
    lcd.print(Scroll_LCD_Right("IOT Kece"));
    delay(350);
  } 
  for (int i = 0; i <= 73; i++) {
    lcd.setCursor(0, 1);
    lcd.print(Scroll_LCD_Left("Selamat Datang DI IOTkece.com!!! Jangan Lupa Praktek...."));
    delay(350);
  }
  Clear_Scroll_LCD_Left();
  for (int i = 13; i <= 24; i++) {
    lcd.setCursor(0, 0);
    lcd.print(Scroll_LCD_Right("IOT Kece"));
    delay(350);
  }
  Clear_Scroll_LCD_Right();
}
//----------------------------------
String Scroll_LCD_Left(String StrDisplay){
  String result;
  String StrProcess = "                " + StrDisplay + "                ";
  result = StrProcess.substring(Li,Lii);
  Li++;
  Lii++;
  if (Li>StrProcess.length()){
    Li=16;
    Lii=0;
  }
  return result;
}

void Clear_Scroll_LCD_Left(){
  Li=16;
  Lii=0;
}
//----------------------------------
String Scroll_LCD_Right(String StrDisplay){
  String result;
  String StrProcess = "                " + StrDisplay + "                ";
  if (Rii<1){
    Ri  = StrProcess.length();
    Rii = Ri-16;
  }
  result = StrProcess.substring(Rii,Ri);
  Ri--;
  Rii--;
  return result;
}

void Clear_Scroll_LCD_Right(){
  Ri=-1;
  Rii=-1;
}
