#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid     = "";      
const char* password = "";  

String serverUrl = "http://alamat-web-baru-kamu.com/api/cek-user?uid=";

#define SS_PIN        5   // SDA RFID  -> G5
#define RST_PIN      22   // RST RFID  -> G22
#define PIN_BUZZER    2   // Positif   -> G2


#define PIN_LED_HIJAU 14  // Sukses    -> G14
#define PIN_LED_MERAH 15  // Gagal     -> G15
#define PIN_SERVO    13   // Sinyal    -> G13

MFRC522 rfid(SS_PIN, RST_PIN);
Servo palangPintu;

// Konek Wi-Fi
void connectToWiFi() {
  Serial.print("[WIFI] Menghubungkan ke ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int timeoutCounter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    timeoutCounter++;
    if(timeoutCounter > 20) { 
      Serial.println("\n[WIFI] Gagal konek, mencoba ulang...");
      WiFi.begin(ssid, password);
      timeoutCounter = 0;
    }
  }
  Serial.println("\n[WIFI] Berhasil Terkoneksi!");
  Serial.print("[WIFI] IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_HIJAU, OUTPUT);
  pinMode(PIN_LED_MERAH, OUTPUT);
  
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED_HIJAU, LOW);
  digitalWrite(PIN_LED_MERAH, LOW);
  
  palangPintu.setPeriodHertz(50);
  palangPintu.attach(PIN_SERVO, 500, 2400);
  palangPintu.write(0); 

  connectToWiFi();

  Serial.println("Tapcard untuk akses...");
}

void loop() {
  digitalWrite(PIN_LED_HIJAU, LOW);
  digitalWrite(PIN_LED_MERAH, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  // Proteksi jika Wi-Fi putus
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  // Standby nunggu kartu ditempel
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uidString += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uidString += " ";
  }
  uidString.toUpperCase();
  
  Serial.println("------------------------------------------------");
  Serial.println("[SCAN] Kartu Terdeteksi! UID: " + uidString);

  // PROSES HTTP GET KE SERVER WEB
  HTTPClient http;
  String fullUrl = serverUrl + uidString; 
  
  Serial.println("[HTTP] Menghubungi Server...");
  http.begin(fullUrl);
  
  int httpResponseCode = http.GET(); 

  if (httpResponseCode > 0) {
    String responseBody = http.getString();
    responseBody.trim(); 
    
    Serial.println("[HTTP] Status Code: " + String(httpResponseCode));
    Serial.println("[HTTP] Balasan Web: " + responseBody);

    if (responseBody == "TERDAFTAR") {
      Serial.println("[STATUS] VALID! Akses Diterima.");
      digitalWrite(PIN_LED_HIJAU, HIGH);
      
      for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_BUZZER, HIGH); delay(120);
        digitalWrite(PIN_BUZZER, LOW);  delay(120);
      }
      
      Serial.println("[SERVO] Membuka Kunci...");
      for (int pos = 0; pos <= 90; pos += 1) { 
        palangPintu.write(pos);
        delay(20); 
      }
      
      Serial.println("[SERVO] Menahan posisi terbuka 5 detik...");
      delay(5000); 
      
      Serial.println("[SERVO] Mengunci Kembali...");
      for (int pos = 90; pos >= 0; pos -= 1) { 
        palangPintu.write(pos);
        delay(20); 
      }
      
      digitalWrite(PIN_LED_HIJAU, LOW);
    } 

    else {
      Serial.println("[STATUS] KARTU BELUM TERDAFTAR / AKSES DITOLAK!");
      digitalWrite(PIN_LED_MERAH, HIGH);
      
      digitalWrite(PIN_BUZZER, HIGH);
      delay(2000); 
      digitalWrite(PIN_BUZZER, LOW);
      
      digitalWrite(PIN_LED_MERAH, LOW);
    }
  } 
  else {
    Serial.print("[HTTP] Gagal tersambung ke web server! Error: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());
    
    // Alert error jaringan
    digitalWrite(PIN_LED_MERAH, HIGH); digitalWrite(PIN_BUZZER, HIGH);
    delay(500);
    digitalWrite(PIN_LED_MERAH, LOW);  digitalWrite(PIN_BUZZER, LOW);
  }
  
  http.end();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}