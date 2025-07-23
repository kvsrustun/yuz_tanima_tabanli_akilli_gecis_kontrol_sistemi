#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Stepper.h>

#define SS_PIN D10
#define RST_PIN D0
MFRC522 rfid(SS_PIN, RST_PIN);

#define LED1 D14
#define LED2 D15
#define BUZZER_PIN D13

int tur = 2048;
Stepper stepper_1(tur,D1,D4,D12,D9);

String veri = "";   
String durum = ""; 


WebServer server(80);
WiFiClient streamClient;
bool streamActive = false;



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

}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String rfidTag = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      rfidTag += String(rfid.uid.uidByte[i], HEX);
    }

    rfidTag.toUpperCase();
    Serial.println("Okunan Kart ID: " + rfidTag);

    // Kart ID'sine göre geçiş kontrolü
    if (rfidTag == "B696C1F") {
      durum = "GECIS_ONAY";
      Serial.println("Geçiş Onaylandı.");
    } else {
      Serial.println("Yetkisiz Kart.");
      durum = "YETKISIZ";
    }

    delay(2000); // Aynı kartın tekrar okunmasını engellemek için bekleme
  }
  server.handleClient();

  if (Serial.available()) {
    veri = Serial.readStringUntil('\n');
    veri.trim();
  }

  if (durum == "GECIS_ONAY") {
    if (veri == "1") {
      digitalWrite(LED1, HIGH);
      delay(3000);
      digitalWrite(LED1, LOW);
      durum = "";
      veri = "";

      stepper_1.setSpeed(15);
      stepper_1.step(tur);
      delay(1000);
      stepper_1.setSpeed(15);
      stepper_1.step(-tur);
      delay(1000);
    } else if (veri == "0") {
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
  } else if (durum == "YETKISIZ") {
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

  delay(10);
}
