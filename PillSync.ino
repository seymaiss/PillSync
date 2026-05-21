#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- KULLANICI AYARLARI ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
String GAS_URL = "https://script.google.com/macros/s/AKfycbzyE6ChxEBalqOjNHStWKt5OGTyYHACWRln27wkQ1wp3Pidd-IJ8zrSnGhiukM0Ox2UDw/exec";

// --- PİN TANIMLAMALARI ---
#define BUTTON_PIN 4
#define BUZZER_PIN 18 
#define SWITCH_PIN 5 

// RGB LED Pinleri
#define RED_PIN 26    
#define GREEN_PIN 25  
#define BLUE_PIN 27

// --- EKRAN AYARLARI ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- ZAMANLAMA VE ALARM AYARLARI ---
unsigned long sonAlarmZamani = 0;
const unsigned long ALARM_SURESI = 60000; 

unsigned long sonBuzzerZamani = 0;
const unsigned long BUZZER_TEKRAR_SURESI = 3000; 

bool alarmAktif = true; 
bool sonKutuDurumu = HIGH; 

volatile bool butonaBasildi = false; 

void IRAM_ATTR butonKesmesi() {
  if (alarmAktif) {
    butonaBasildi = true;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SWITCH_PIN, INPUT_PULLUP); 
  pinMode(BUZZER_PIN, OUTPUT); 
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), butonKesmesi, FALLING);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED basarisiz oldu!"));
    for(;;);
  }
  
  renkAyarla(0, 0, 255); // Başlangıçta Mavi
  ekranaOrtaliYaz("Wi-Fi...", "- PillSync -");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n[Sistem] Baglanti basarili.");
  ekranaOrtaliYaz("BAGLANDI", "Sistem Hazir");
  delay(2000); 
  
  ekranaOrtaliYaz("ILAC VAKTI", "- PillSync -");
  sesCikarVeFlasYAP(150, 3, 255, 255, 0); 
  renkAyarla(255, 255, 0); 
  sonBuzzerZamani = millis(); 
  
  sonKutuDurumu = digitalRead(SWITCH_PIN); 
}

void loop() {
  // --- 1. KISIM: KESİNTİSİZ GÜVENLİK KİLİDİ ---
  if (!alarmAktif && digitalRead(SWITCH_PIN) == HIGH) {
    ekranaOrtaliYaz("KAPAT!", "Kutuyu Kapatiniz");
    
    while(digitalRead(SWITCH_PIN) == HIGH) {
      renkAyarla(255, 0, 0); 
      digitalWrite(BUZZER_PIN, HIGH); 
      delay(100); 
      renkAyarla(0, 0, 0); 
      digitalWrite(BUZZER_PIN, LOW);  
      delay(50);
    }
    
    digitalWrite(BUZZER_PIN, LOW); 
    renkAyarla(0, 0, 255); 
    ekranaOrtaliYaz("UYKU MODU", "Bekleniyor...");
    sonKutuDurumu = LOW; 
  }

  // --- 2. KISIM: NORMAL ALARM ANINDA KAPAK GEÇİŞLERİ ---
  bool kutuAcik = digitalRead(SWITCH_PIN); 
  if (alarmAktif && (kutuAcik != sonKutuDurumu)) { 
    if (kutuAcik == HIGH) { 
      ekranaOrtaliYaz("KAPAK ACIK", "Ilaci Aliniz");
      renkAyarla(255, 255, 255); 
    } else {
      ekranaOrtaliYaz("ILAC VAKTI", "- PillSync -"); 
      renkAyarla(255, 255, 0); 
    }
    sonKutuDurumu = kutuAcik; 
  }

  // --- 3. KISIM: GENEL ZAMAN KONTROLÜ ---
  if (!alarmAktif && (millis() - sonAlarmZamani >= ALARM_SURESI)) {
    alarmAktif = true; 
    butonaBasildi = false; 
    ekranaOrtaliYaz("ILAC VAKTI", "- PillSync -"); 
    sesCikarVeFlasYAP(150, 3, 255, 255, 0); 
    renkAyarla(255, 255, 0); 
    sonBuzzerZamani = millis(); 
  }

  // --- 4. KISIM: ISRARCI ALARM SES TEKRARI ---
  if (alarmAktif && (millis() - sonBuzzerZamani >= BUZZER_TEKRAR_SURESI)) {
    sonBuzzerZamani = millis(); 
    if (digitalRead(SWITCH_PIN) == HIGH) {
      sesCikarVeFlasYAP(150, 3, 255, 255, 255); 
      renkAyarla(255, 255, 255); 
    } else {
      sesCikarVeFlasYAP(150, 3, 255, 255, 0); 
      renkAyarla(255, 255, 0); 
    }
  }

  // --- 5. KISIM: BUTON KONTROLÜ VE DOĞRULAMA (DÜZELTİLDİ!) ---
  if (alarmAktif && butonaBasildi) {
    if (digitalRead(SWITCH_PIN) == LOW) { 
      // ÇÖZÜM: Bayrağı hemen burada sıfırlıyoruz ki aşağıdaki flaş fonksiyonu çalışabilsin!
      butonaBasildi = false; 

      Serial.println("[Hata] Kapak kapaliyken buton reddedildi!");
      ekranaOrtaliYaz("HATA!", "Kutuyu Ac!");
      
      // Şimdi aslanlar gibi 3 kere Kırmızı yanıp sönecek ve ötecek!
      sesCikarVeFlasYAP(200, 3, 255, 0, 0); 
      
      renkAyarla(255, 255, 0); // Tekrar alarm sarısına geri dön
      delay(1500); 
      ekranaOrtaliYaz("ILAC VAKTI", "- PillSync -"); 
    } 
    else {
      alarmAktif = false;        
      butonaBasildi = false; 
      
      sesCikarVeFlasYAP(100, 1, 0, 255, 0); // Başarılı, Yeşil yan!
      renkAyarla(0, 255, 0); 
      
      ekranaOrtaliYaz("SIFA OLSUN", "- PillSync -");
      delay(2000); 
      
      ekranaOrtaliYaz("YUKLENIYOR", "- PillSync -");
      veriGonder("Ilac_Alindi"); 
      
      ekranaOrtaliYaz("TAMAMLANDI", "- PillSync -");
      delay(3000);
      
      renkAyarla(0, 0, 255); 
      ekranaOrtaliYaz("UYKU MODU", "Bekleniyor...");
      sonAlarmZamani = millis(); 
    }
  }
}

void ekranaOrtaliYaz(String anaMetin, String altMetin) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  int xAna = (SCREEN_WIDTH - (anaMetin.length() * 12)) / 2;
  if(xAna < 0) xAna = 0; 
  display.setCursor(xAna, 18); 
  display.println(anaMetin);

  display.setTextSize(1);
  int xAlt = (SCREEN_WIDTH - (altMetin.length() * 6)) / 2;
  if(xAlt < 0) xAlt = 0;
  display.setCursor(xAlt, 45); 
  display.println(altMetin);

  display.display();
}

void renkAyarla(int r, int g, int b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void sesCikarVeFlasYAP(int sure, int tekrar, int r, int g, int b) {
  for(int i = 0; i < tekrar; i++) {
    if (butonaBasildi) break; 
    renkAyarla(r, g, b); 
    digitalWrite(BUZZER_PIN, HIGH); 
    delay(sure);                    
    renkAyarla(0, 0, 0); 
    digitalWrite(BUZZER_PIN, LOW);  
    if (tekrar > 1) {
      delay(100); 
    }
  }
}

void veriGonder(String mesaj) {
  WiFiClientSecure client;
  client.setInsecure(); 
  HTTPClient http;
  String tamYol = GAS_URL + "?durum=" + mesaj;
  
  if (http.begin(client, tamYol)) {
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); 
    int httpCode = http.GET(); 
    http.end();
  }
}