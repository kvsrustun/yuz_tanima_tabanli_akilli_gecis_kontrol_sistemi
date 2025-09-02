#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"
#include <esp_http_server.h>

// --- Wi-Fi ve Ağ Ayarları ---
const char* ssid = "EYUP DENEYAP";
const char* password = "DENEYAP-2025";

// --- SİZİN AĞINIZA ÖZEL STATİK IP AYARLARI ---
// Bu IP adresini, diğer ESP32 kartınızla çakışmaması için farklı seçtik.
IPAddress staticIP(192, 168, 2, 151);  // Kameranız için YENİ SABİT IP ADRESİ
IPAddress gateway(192, 168, 2, 1);    // Varsayılan Ağ Geçidi (modeminizin adresi)
IPAddress subnet(255, 255, 255, 0);   // Alt Ağ Maskesi
IPAddress primaryDNS(8, 8, 8, 8);     // Google DNS (genellikle iyi çalışır)

// --- Geri Kalan Değişkenler ---
WebServer server(80);
WiFiClient streamClient;
bool streamActive = false;
TaskHandle_t streamHandle = NULL;

void cameraInit();
void streamTask(void*);

//-----------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  cameraInit();

  // --- STATİK IP'Yİ ETKİNLEŞTİRME ---
  // Wi-Fi'ye bağlanmadan ÖNCE statik IP ayarlarını yüklüyoruz.
  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS)) {
    Serial.println("HATA: Statik IP ayarlanamadı!");
  }

  // Wi-Fi'ye bağlanma
  WiFi.begin(ssid, password);
  Serial.print("Wi-Fi ağına bağlanılıyor");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi bağlantısı başarılı!");
  Serial.print("Kamera yayını için sabit IP adresi: http://");
  Serial.println(WiFi.localIP()); // Bu adres artık hep 192.168.2.151 olacak
  Serial.print("Tarayıcıya tam adres olarak şunu yazın: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");

  // Web sunucusu ayarları
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "<html><body><h1>Deneyap Kamera</h1><img src=\"/stream\"/></body></html>");
  });

  server.on("/stream", HTTP_GET, []() {
    if (streamHandle == NULL) {
      streamClient = server.client();
      streamActive = true;
      xTaskCreatePinnedToCore(streamTask, "stream", 8192, NULL, 1, &streamHandle, 1);
    }
  });

  server.begin();
}

//-----------------------------------------------------------------------------------------

void loop() {
  server.handleClient();
}

//-----------------------------------------------------------------------------------------

void streamTask(void* pvParameters) {
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  streamClient.print(response);

  while (streamClient.connected() && streamActive) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) continue;

    streamClient.print("--frame\r\n");
    streamClient.print("Content-Type: image/jpeg\r\n\r\n");
    streamClient.write(fb->buf, fb->len);
    streamClient.print("\r\n");

    esp_camera_fb_return(fb);
    delay(50);
  }

  streamClient.stop();
  streamActive = false;
  streamHandle = NULL;
  vTaskDelete(NULL);
}

//-----------------------------------------------------------------------------------------
// Bu fonksiyon orijinal haliyle kalıyor, değişiklik yok.
void cameraInit() {
camera_config_t config;
config.ledc_channel = LEDC_CHANNEL_0;
config.ledc_timer = LEDC_TIMER_0;
config.pin_d0 = CAMD2;
config.pin_d1 = CAMD3;
config.pin_d2 = CAMD4;
config.pin_d3 = CAMD5;
config.pin_d4 = CAMD6;
config.pin_d5 = CAMD7;
config.pin_d6 = CAMD8;
config.pin_d7 = CAMD9;
config.pin_xclk = CAMXC;
config.pin_pclk = CAMPC;
config.pin_vsync = CAMV;
config.pin_href = CAMH;
config.pin_sscb_sda = CAMSD;
config.pin_sscb_scl = CAMSC;
config.pin_pwdn = -1;
config.pin_reset = -1;
config.xclk_freq_hz = 15000000;
config.frame_size = FRAMESIZE_QVGA;
config.pixel_format = PIXFORMAT_JPEG;
config.grab_mode = CAMERA_GRAB_LATEST;
config.fb_location = CAMERA_FB_IN_PSRAM;
config.jpeg_quality = 10;
config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera başlatılamadı, hata: 0x%x\n", err);
  }
}