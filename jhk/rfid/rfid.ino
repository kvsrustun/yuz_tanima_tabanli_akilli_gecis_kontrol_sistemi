// Gerekli Kütüphaneler
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <AccelStepper.h> // DONMAYI ÖNLEYEN, GÜNCEL STEP MOTOR KÜTÜPHANESİ

// --- Pin Tanımlamaları ---
// RFID Pinleri
#define SS_PIN D10
#define RST_PIN D0
// LED ve Buzzer Pinleri
#define LED1 D14 // Başarılı geçiş ledi
#define LED2 D15 // Başarısız geçiş ledi
#define BUZZER_PIN D13
// Step Motor Pinleri (ULN2003 sürücü için pin sırası: IN1, IN3, IN2, IN4)
#define MOTOR_PIN_1 D1
#define MOTOR_PIN_2 D4
#define MOTOR_PIN_3 D12
#define MOTOR_PIN_4 D9

// --- Cihaz Kurulumları ---
MFRC522 rfid(SS_PIN, RST_PIN);
AccelStepper stepper_1(AccelStepper::FULL4WIRE, MOTOR_PIN_1, MOTOR_PIN_3, MOTOR_PIN_2, MOTOR_PIN_4);

// --- Wi-Fi ve Ağ Ayarları ---
const char* ssid = "EYUP DENEYAP";
const char* password = "DENEYAP-2025";

IPAddress staticIP(192, 168, 2, 150);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);

WiFiServer tcpServer(1234);
WiFiClient tcpClient;

// --- Global Değişkenler ---
String veri = "";
String durum = "";
unsigned long durumBaslamaZamani = 0;
bool motorGeriDonmeli = false;

//-----------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID Okuyucu Hazır.");

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  stepper_1.setMaxSpeed(1000.0);     // Hız artırıldı
  stepper_1.setAcceleration(600.0);  // Hızlı ve akıcı
  stepper_1.setCurrentPosition(0);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS)) {
    Serial.println("HATA: Statik IP ayarlanamadı!");
  }

  WiFi.begin(ssid, password);
  Serial.print("WiFi'ye bağlanıyor");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Bağlandı!");
  Serial.print("Atanan Sabit IP Adresi: ");
  Serial.println(WiFi.localIP());

  tcpServer.begin();
}

//-----------------------------------------------------------------------------------------

void loop() {
  // --- AĞ VE SENSÖR OKUMA BÖLÜMÜ ---
  if (!tcpClient || !tcpClient.connected()) {
    tcpClient = tcpServer.available();
  }
  if (tcpClient && tcpClient.connected() && tcpClient.available()) {
    veri = tcpClient.readStringUntil('\n');
    veri.trim();
    Serial.println("Gelen veri: " + veri);
  }

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String rfidTag = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      rfidTag += String(rfid.uid.uidByte[i], HEX);
    }
    rfidTag.toUpperCase();
    Serial.println("Okunan Kart ID: " + rfidTag);
    if (rfidTag == "B696C1F") {
      durum = "GECIS_ONAY";
      Serial.println("Geçiş Onaylandı.");
    } else {
      durum = "YETKISIZ";
      Serial.println("Yetkisiz Kart.");
    }
  }

  // --- ANA MANTIK VE KONTROL BÖLÜMÜ ---
  unsigned long simdikiZaman = millis();

  // Durum "Geçiş Onaylı" ise
  if (durum == "GECIS_ONAY") {
    // ve "1" geldiyse motoru çalıştır
    if (veri == "1" && !motorGeriDonmeli && stepper_1.distanceToGo() == 0) {
      digitalWrite(LED1, HIGH);
      delay(500);
      digitalWrite(LED1, LOW);
      stepper_1.moveTo(2048); // ileri hareket
      Serial.println("Motor ileri hareketi için ayarlandı.");
      motorGeriDonmeli = true;   // geri dönüş için beklemede
      durumBaslamaZamani = simdikiZaman;
      veri = "";
    }
    // ve "0" geldiyse hata ver
    else if (veri == "0") {
      Serial.println("Hata: Tanınmayan yüz algılandı.");
      digitalWrite(LED2, HIGH);
      for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
      }
      digitalWrite(LED2, LOW);
      durum = "";
      veri = "";
    }
  }

  // Yetkisiz kart okutulduysa
  if (durum == "YETKISIZ") {
    Serial.println("Hata: Yetkisiz kart okutuldu.");
    digitalWrite(LED2, HIGH);
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
      delay(200);
    }
    digitalWrite(LED2, LOW);
    durum = "";
  }

  // Motor ileri hareketini tamamladıysa ve geri dönmesi gerekiyorsa
  if (motorGeriDonmeli && stepper_1.distanceToGo() == 0 && (simdikiZaman - durumBaslamaZamani > 1000)) {
    stepper_1.moveTo(0);  // geri dönüş başlat
    Serial.println("Motor geri dönüş için ayarlandı.");
    motorGeriDonmeli = false; // geri dönüş başlatıldıktan sonra false
    durum = "";
  }

  // Motorun hedefine doğru bir adım atmasını sağlar
  stepper_1.run();
}