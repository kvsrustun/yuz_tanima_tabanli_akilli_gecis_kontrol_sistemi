import cv2
import face_recognition
import socket
import time
import os 

class TCPCommunicator:
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
        self.sock = None
        self.connect()

    def connect(self):
        try:
            print(f"{self.ip}:{self.port} adresine bağlanılıyor...")
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(2) 
            self.sock.connect((self.ip, self.port))
            print("Bağlantı başarılı!")
        except Exception as e:
            print(f"İlk bağlantı hatası: {e}")
            self.sock = None

    def send(self, data):
        if self.sock is None:
            self.connect() 
            return

        try:
            self.sock.sendall(f"{data}\n".encode())
        except Exception as e:
            print(f"Veri gönderme hatası: {e}. Bağlantı kopmuş olabilir.")
            self.sock.close()
            self.sock = None

    def close(self):
        if self.sock:
            self.sock.close()
            print("Bağlantı kapatıldı.")


ESP32_IP = "192.168.2.150"
ESP32_PORT = 1234
esp32_comm = TCPCommunicator(ESP32_IP, ESP32_PORT)

try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    image_path = os.path.join(script_dir, "kevser.jpg")
    
    print(f"Referans resim şu yoldan yükleniyor: {image_path}")

    kisi1 = face_recognition.load_image_file(image_path)
    kisi1encodings = face_recognition.face_encodings(kisi1)[0]
    encodinglist = [kisi1encodings]
    namelist = ["Kevser Ustun"]
except (FileNotFoundError, IndexError):
    print(f"HATA: Referans resim 'kevser.jpg' bulunamadı veya resimde yüz yok!")
    print(f"Lütfen 'kevser.jpg' dosyasının bu script ile aynı klasörde olduğundan emin olun.")
    exit()

url = "http://192.168.2.151/stream"
cap = cv2.VideoCapture(url)
if not cap.isOpened():
    print(f"IP Kamera açılamadı! Adres: {url}")
    exit()

last_send_time = 0
send_interval = 2 

while True:
    ret, image = cap.read()
    if not ret:
        print("Kamera görüntüsü alınamadı!")
        time.sleep(1) 
        continue
    
    image = cv2.flip(image, 1)
    rgb_img = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    facelocations = face_recognition.face_locations(rgb_img)
    faceencodings = face_recognition.face_encodings(rgb_img, facelocations)

    current_time = time.time()
    detected_name = ""

    for faceloc, faceencoding in zip(facelocations, faceencodings):
        ust, sag, alt, sol = faceloc
        matchfaces = face_recognition.compare_faces(encodinglist, faceencoding)
        name = "Bilinmeyen Kisi"

        if True in matchfaces:
            matchindex = matchfaces.index(True)
            name = namelist[matchindex]
            detected_name = name 

        cv2.rectangle(image, (sol, ust), (sag, alt), (255, 0, 0), 2)
        cv2.putText(image, name, (sol, ust - 10), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)

    if current_time - last_send_time > send_interval:
        if "Kevser" in detected_name:
            esp32_comm.send("1")
            print("1 gönderildi")
            last_send_time = current_time
        elif len(facelocations) > 0:
            esp32_comm.send("0")
            print("0 gönderildi")
            last_send_time = current_time
            
 
print("Program sonlandırılıyor.")
cap.release()
#cv2.destroyAllWindows() 
esp32_comm.close()
