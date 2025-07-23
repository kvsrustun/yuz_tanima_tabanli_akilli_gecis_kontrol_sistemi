import cv2
import face_recognition
import serial
import time

ser = serial.Serial("COM67", 115200)
time.sleep(2)

kisi1 = face_recognition.load_image_file("C:/Users/ustun/Desktop/jhk/kevser.jpg")
kisi1encodings = face_recognition.face_encodings(kisi1)[0]


if len(kisi1encodings) == 0:
    print("Kişinin yüzü görüntüden alınamadı!")
    exit()


encodinglist = [kisi1encodings]
namelist = ["Kevser Ustun"]

url = "http://192.168.2.85/stream"

cap = cv2.VideoCapture(url)
if not cap.isOpened():
    print("IP Kamera açılamadı!")
    exit()

while True:
    ret, image = cap.read()
    if not ret:
        print("Kamera görüntüsü alınamadı!")
        continue

    image = cv2.flip(image, 1)  

    rgb_img = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    facelocations = face_recognition.face_locations(rgb_img)
    faceencodings = face_recognition.face_encodings(rgb_img, facelocations)

    for faceloc, faceencoding in zip(facelocations, faceencodings):
        ust, sag, alt, sol = faceloc
        matchfaces = face_recognition.compare_faces(encodinglist, faceencoding)

        name = "Bilinmeyen Kisi"

        if True in matchfaces:
            matchindex = matchfaces.index(True)
            name = namelist[matchindex]

            ser.write(b'1\n') 
            print("1 değerini gönderdi") 
        else:
            ser.write(b'0\n') 
            print("0 değerini gönderdi") 

        cv2.rectangle(image, (sol, ust), (sag, alt), (255, 0, 0), 2)
        cv2.putText(image, name, (sol, ust - 10), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)

    cv2.imshow("kisi", image)

    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

print("Son tanınan kişi:", name)

cap.release()
cv2.destroyAllWindows()
ser.close()
