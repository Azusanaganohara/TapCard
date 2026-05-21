# Tap Card

Sistem akses kontrol berbasis RFID dan ESP32 yang melakukan validasi UID kartu melalui permintaan HTTP ke server web.

## Ringkasan

Proyek ini menggunakan:
- ESP32 (board `esp32dev`)
- Modul RFID MFRC522
- Servo untuk membuka/menutup palang pintu
- Buzzer dan LED sebagai indikator status
- Koneksi Wi-Fi untuk memanggil API server dengan UID kartu

## Fitur

- Membaca UID kartu RFID
- Mengirim UID ke server web
- Menyalakan LED hijau dan membuka servo saat kartu valid
- Menyalakan LED merah dan menyala buzzer saat kartu tidak valid
- Koneksi Wi-Fi otomatis ulang bila terputus

## Perangkat keras dan pin

ESP32 pin yang digunakan:
- `G5` (GPIO 5) sebagai `SS_PIN` untuk SDA RFID
- `G22` (GPIO 22) sebagai `RST_PIN` untuk RST RFID
- `G2` (GPIO 2) untuk `PIN_BUZZER`
- `G14` (GPIO 14) untuk `PIN_LED_HIJAU`
- `G15` (GPIO 15) untuk `PIN_LED_MERAH`
- `G13` (GPIO 13) untuk `PIN_SERVO`

## Koneksi kabel

- MFRC522 SDA -> GPIO 5
- MFRC522 RST -> GPIO 22
- MFRC522 MOSI -> GPIO 23 (default SPI)
- MFRC522 MISO -> GPIO 19 (default SPI)
- MFRC522 SCK -> GPIO 18 (default SPI)
- Servo signal -> GPIO 13
- LED hijau -> GPIO 14
- LED merah -> GPIO 15
- Buzzer -> GPIO 2
- Ground dan VCC disesuaikan dengan modul

## Konfigurasi perangkat lunak

File `platformio.ini` sudah dikonfigurasikan sebagai berikut:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_port = COM3
monitor_port = COM3

lib_deps =
    miguelbalboa/MFRC522 @ ^1.4.11
    madhephaestus/ESP32Servo @ ^3.0.5
```

## Pengaturan kode

Di `src/main.cpp`, atur:
- `ssid` dan `password` Wi-Fi Anda
- `serverUrl` ke URL API Anda, misalnya `http://alamat-web-baru-kamu.com/api/cek-user?uid=`

Server diharapkan mengembalikan teks:
- `TERDAFTAR` untuk kartu valid
- selain itu dianggap tidak valid

## Cara menggunakan

1. Buka proyek di PlatformIO.
2. Masukkan SSID dan password Wi-Fi di `src/main.cpp`.
3. Ubah `serverUrl` ke alamat server Anda.
4. Hubungkan perangkat keras sesuai pin.
5. Unggah firmware ke ESP32.
6. Buka Serial Monitor dengan baud `115200`.
7. Tempelkan kartu RFID untuk menguji.

## Alur kerja sistem

1. ESP32 terhubung ke Wi-Fi.
2. Sistem menunggu kartu RFID muncul.
3. UID kartu dibaca.
4. UID dikirim ke server menggunakan HTTP GET.
5. Server merespons status kartu.
6. ESP32 menyalakan indikator sesuai respons:
   - kartu valid → LED hijau + buzzer + servo buka lalu tutup
   - kartu tidak valid → LED merah + buzzer singkat

## Penanganan masalah

- Jika Wi-Fi terputus, perangkat otomatis mencoba kembali.
- Jika server tidak bisa dihubungi, LED merah dan buzzer menyala sebagai tanda kesalahan.

## Catatan

Pastikan API server Anda dapat menerima UID dengan format yang dibuat di `main.cpp` dan merespons sesuai dengan logika validasi.
