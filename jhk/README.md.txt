# 📷 Yüz Tanıma Tabanlı Akıllı Geçiş Kontrol Sistemi  
**(Deneyap Kart 1A ile Tam Entegre Uygulama)**

## 🔍 Proje Açıklaması

Bu proje, **Deneyap Kart 1A** kullanılarak geliştirilmiş bir yüz tanıma destekli akıllı geçiş kontrol sistemidir. Kamera üzerinden alınan görüntüler Python ile analiz edilir, tanınan kişi bilgisi ESP32’ye seri port üzerinden iletilir. Aynı zamanda RFID kart doğrulaması yapılır ve geçiş izni durumuna göre step motor, LED ve buzzer kontrol edilir. Tüm işlemler tek kart üzerinde entegre edilmiştir.

---

## 🧰 Kullanılan Donanım ve Modüller

- 📷 **Deneyap Kart 1A** (ESP32-CAM destekli)
- 📇 MFRC522 RFID Okuyucu
- 💡 2x LED (Geçiş durumu göstergesi)
- 🔊 Buzzer
- 🔁 **Stepper Motor** (2048 adım – harici **5V pil** ile beslenmiştir)
- 🔋 **5V harici pil** (step motor için ayrı güç kaynağı)
- 🧠 Bilgisayar (Python 3.10.10 ile yüz tanıma için)

---

## ⚠️ Güç Besleme Notu

> Step motor, Deneyap Kart’ın pinlerinden yeterli akım çekemeyeceği için **harici 5V pil** ile beslenmiştir.  
> Bu sayede motor stabil çalışır, kart zarar görmez.

- **Ortak GND bağlantısı mutlaka yapılmalıdır.**  
  Deneyap Kart ve motor devresinin GND uçları birleştirilmelidir.

---

## 🧪 Kullanılan Yazılım ve Kütüphaneler

### Arduino (Deneyap Kart 1A):
- `WiFi.h`
- `Wire.h`
- `esp_camera.h`
- `WebServer.h`
- `esp_http_server.h`
- `SPI.h`
- `MFRC522.h`
- `Stepper.h`

### Python (Sürüm: 3.10.10):

pip install opencv-python face_recognition pyserial numpy

## ⚙️ Sistem Çalışma Mantığı

- **Deneyap Kart 1A (ESP32-CAM)** IP yayını başlatır.  
- **Python betiği**, bu yayını alıp yüz tanıma işlemini gerçekleştirir.  
- Tanınan kişi bilgisi **seri port** üzerinden karta gönderilir.  
- Aynı kart üzerindeki **RFID okuyucu**, kullanıcı kimliğini kontrol eder.  
- Tanınan ve yetkili kişi için:  
  - Step motor döner ve geçiş sağlanır.  
  - LED yanar.  
- Yetkisiz girişler için:  
  - Buzzer ve kırmızı LED ile uyarı verilir.  

---

## 🚀 Kurulum ve Kullanım

1. **Deneyap Kart 1A – Kamera Kodunu Yükle**  
   - `kamera.ino` dosyasını Deneyap Kart 1A’ya yükleyin.  
   - Seri port üzerinden şu şekilde bir çıktı alınır:  
     ```
     Kamera yayını için: http://192.168.2.xx/stream
     ```

2. **Deneyap Kart 1A – RFID & Motor Kontrol Kodunu Yükle**  
   - `rfid_led_buzzer_step.ino` dosyasını aynı karta yükleyin.  
   - RFID kart ID’si seri porttan okunabilir.  
   - Örnek yetkili kart: `"B696C1F"`

3. **Python Betiğini Çalıştırma**  
   - `kisi_tanima.py` içinde IP adresini güncelleyin:  
     ```python
     url = "http://192.168.2.xx/stream"
     ```  
   - `kevser.jpg` dosyasını aynı klasöre koyun.  
   - Terminalden çalıştırın:  
     ```bash
     python kisi_tanima.py
     ```

---

## 📂 Dosya Yapısı

proje/
│
├── kamera.ino # IP kamera yayını
├── rfid_led_buzzer_step.ino # RFID, motor, LED, buzzer kontrolü
├── kisi_tanima.py # Python yüz tanıma betiği
├── kevser.jpg # Tanınacak kişinin yüz görseli
├── dlib-19.22.99-cp310-cp310-win_amd64.whl # Python dlib kütüphanesi tekerleği (Windows, Python 3.10)
└── README.md # Proje dokümantasyonu


### Bu .whl dosyasını yüklemek için kullanıcılar şunu çalıştırabilirler:
pip install dlib-19.22.99-cp310-cp310-win_amd64.whl


## 👤 Kişi (Yüz) Ekleme

- Yeni bir kişi eklemek için `kisi_tanima.py` dosyasındaki aşağıdaki adımları izleyin:  
  1. Yüzü tanınacak kişinin fotoğrafını `.jpg` formatında kaydedin.  
  2. Fotoğraf dosyasını Python dosyası ile aynı klasöre koyun.  
  3. `kisi_tanima.py` dosyasında, aşağıdaki kısmı kopyalayıp çoğaltarak yeni kişi ve yüz kodlamasını ekleyin:

python
yeni_kisi = face_recognition.load_image_file("yeni_kisi.jpg")
yeni_kisi_encodings = face_recognition.face_encodings(yeni_kisi)[0]

encodinglist = [kisi1encodings, yeni_kisi_encodings]  # Önceki ve yeni kişi kodlamaları listesi
namelist = ["Kevser Ustun", "Yeni Kisi Adı"]          # İsimler listesi



## 🔐 RFID Kart Tanımlama

- Geçerli kart ID’leri `rfid_led_buzzer_step.ino` dosyasında tanımlıdır.  

---

## 👩‍💻 Geliştirici

**Kevser ÜSTÜN**  
Giresun Üniversitesi – Bilgisayar Mühendisliği  
