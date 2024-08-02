#include <Wire.h>                       // Library untuk komunikasi I2C
#include <LiquidCrystal_I2C.h>          // Library untuk LCD yang menggunakan I2C
#include <RTClib.h>                     // Library untuk RTC (Real-Time Clock)
#include <Adafruit_Fingerprint.h>       // Library untuk sensor sidik jari dari Adafruit
#include <SD.h>                         // Library untuk membaca dan menulis ke kartu SD
#include <SPI.h>                        // Library untuk komunikasi SPI
#include <avr/pgmspace.h>               // Library untuk menyimpan data di memori flash (PROGMEM)

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
#include <SoftwareSerial.h>             // Library untuk membuat port serial pada pin lain
SoftwareSerial mySerial(2, 3);          // Membuat port serial pada pin 2 (RX) dan 3 (TX) jika menggunakan board selain ATmega2560
#else
#define mySerial Serial1                // Mendefinisikan mySerial sebagai Serial1 jika menggunakan ATmega2560
#endif

const int cs = 10;                       // Mendefinisikan pin chip select untuk kartu SD pada pin 7
const int ledRed = 6;                   // Mendefinisikan pin untuk LED merah pada pin 6
const int ledBlue = 8;                  // Mendefinisikan pin untuk LED Biru pada pin 8
const int buzzer = 9;                  // Mendefinisikan pin untuk buzzer pada pin 8
const int button = 5;                  // Mendefinisikan pin untuk button pada pin 8

LiquidCrystal_I2C lcd(0x27, 16, 2);     // Menginisialisasi LCD I2C dengan alamat 0x27, ukuran 16x2 karakter
RTC_DS3231 rtc;                         // Membuat objek RTC dari kelas RTC_DS3231
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); // Membuat objek sensor sidik jari menggunakan port serial yang sudah didefinisikan

bool enrollMode = false;               // Deklarasi variabel boolean untuk menentukan apakah mode pendaftaran sidik jari aktif atau tidak. Default-nya adalah false.

const char namaPengguna0[] PROGMEM = "Anida Aulia Utami";        // Menyimpan nama pengguna di memori flash (PROGMEM)
const char namaPengguna1[] PROGMEM = "Rifky Hilman";      // Menyimpan nama pengguna di memori flash (PROGMEM) untuk menghemat SRAM
const char namaPengguna2[] PROGMEM = "Afif Naufal";           // Menyimpan nama pengguna di memori flash (PROGMEM)
const char namaPengguna3[] PROGMEM = "Merliyana";           // Menyimpan nama pengguna di memori flash (PROGMEM)

// Array pointer ke string yang juga disimpan di memori flash
const char* const namaPengguna[] PROGMEM = {
  namaPengguna0,
  namaPengguna1,
  namaPengguna2,
  namaPengguna3,
};

// Fungsi untuk mencetak nama pengguna berdasarkan ID
void printName(int id) {
  char buffer[20]; // Mendeklarasikan Array String dengan panjang 20 karakter
  strcpy_P(buffer, (char*)pgm_read_word(&(namaPengguna[id]))); // Membaca string dari memori flash dan menyalinnya ke buffer
  lcd.print(buffer);                                           // Mencetak nama pengguna di LCD
}

void setup() {
  Wire.begin();                        // Memulai komunikasi I2C
  Serial.begin(9600);                  // Memulai komunikasi serial dengan baud rate 9600
  lcd.init();                          // Inisialisasi LCD
  lcd.backlight();                     // Menyalakan lampu latar LCD
  pinMode(ledRed, OUTPUT);             // Menentukan pin ledRed sebagai output
  pinMode(ledBlue, OUTPUT);             // Menentukan pin ledBlue sebagai output
  pinMode(buzzer, OUTPUT);             // Menentukan pin buzzer sebagai output
  pinMode(button, INPUT_PULLUP);    // Mengatur push button sebagai input dengan pull-up resistor internal

  if (!rtc.begin()) {                  // Memeriksa apakah RTC terhubung
    Serial.println(F("RTC tidak ditemukan!"));
    lcd.clear();                       // Membersihkan layar LCD
    lcd.print(F("RTC Error"));         // Menampilkan pesan kesalahan pada LCD
    while (1);                         // Berhenti di sini jika RTC tidak ditemukan
  }

  if (rtc.lostPower()) {               // Memeriksa apakah RTC kehilangan daya dan perlu diatur ulang
    Serial.println(F("RTC kehilangan daya, set waktu ke default!"));
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Mengatur waktu RTC ke waktu kompilasi sketsa
  }

  delay(100);                          // Memberikan jeda waktu 100 ms
  Serial.println(F("\n\nAdafruit finger detect test")); // Menampilkan pesan pada serial monitor

  finger.begin(57600);                 // Memulai komunikasi dengan sensor sidik jari pada baud rate 57600
  delay(5);                            // Memberikan jeda waktu 5 ms

  if (finger.verifyPassword()) {       // Memeriksa apakah sensor sidik jari terhubung dan berfungsi
    Serial.println(F("Found fingerprint sensor!"));
  } else {
    Serial.println(F("Did not find fingerprint sensor :("));
    while (1) { delay(1); }            // Berhenti di sini jika sensor sidik jari tidak ditemukan
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();              // Membaca parameter sensor sidik jari

  finger.getTemplateCount();           // Mendapatkan jumlah template (sidik jari) yang tersimpan di sensor

  if (finger.templateCount == 0) {     // Memeriksa apakah ada template yang tersimpan
    Serial.println(F("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example."));
  } else {
    Serial.println(F("Waiting for valid finger..."));
    Serial.print(F("Sensor contains ")); Serial.print(finger.templateCount); Serial.println(F(" templates"));
  }

  if (!SD.begin(cs)) {                 // Memulai komunikasi dengan kartu SD
    Serial.println(F("Inisialisasi SD card gagal!"));
    return;                            // Keluar dari fungsi setup() jika inisialisasi kartu SD gagal
  }
  Serial.println(F("SD card berhasil diinisialisasi."));
  
  DateTime now = rtc.now();

  if (now.day() == 1) { // Jika hari ini adalah tanggal 30
      hitungGaji(); // Panggil fungsi hitungGaji untuk menghitung gaji
  }
}


void loop() {
  DateTime now = rtc.now(); // Mendapatkan waktu dan tanggal saat ini dari RTC (Real-Time Clock)

  lcd.setCursor(0, 0);      // Mengatur kursor LCD pada baris pertama, kolom pertama
  lcd.print(F("Time: "));   // Menampilkan teks "Time: " di LCD
  if (now.hour() < 10) lcd.print('0'); // Menambahkan '0' jika jam kurang dari 10 (untuk format HH)
  lcd.print(now.hour());    // Menampilkan jam saat ini
  lcd.print(':');           // Menampilkan karakter ':'
  if (now.minute() < 10) lcd.print('0'); // Menambahkan '0' jika menit kurang dari 10 (untuk format MM)
  lcd.print(now.minute());  // Menampilkan menit saat ini
  lcd.print(':');           // Menampilkan karakter ':'
  if (now.second() < 10) lcd.print('0'); // Menambahkan '0' jika detik kurang dari 10 (untuk format SS)
  lcd.print(now.second());  // Menampilkan detik saat ini

  lcd.setCursor(0, 1);      // Mengatur kursor LCD pada baris kedua, kolom pertama
  lcd.print(F("Date: "));   // Menampilkan teks "Date: " di LCD
  if (now.day() < 10) lcd.print('0'); // Menambahkan '0' jika hari kurang dari 10 (untuk format DD)
  lcd.print(now.day());     // Menampilkan hari saat ini
  lcd.print('/');           // Menampilkan karakter '/'
  if (now.month() < 10) lcd.print('0'); // Menambahkan '0' jika bulan kurang dari 10 (untuk format MM)
  lcd.print(now.month());   // Menampilkan bulan saat ini
  lcd.print('/');           // Menampilkan karakter '/'
  lcd.print(now.year());    // Menampilkan tahun saat ini

  delay(1000);              // Memberikan jeda waktu 1 detik sebelum melanjutkan loop

  if (enrollMode) {         // Memeriksa apakah mode enroll (pendaftaran sidik jari) aktif
    enrollFingerprint();    // Jika ya, panggil fungsi enrollFingerprint()
  } else {
    getFingerprintID(now);  // Jika tidak, panggil fungsi getFingerprintID() dengan parameter waktu saat ini
  }

  if (digitalRead(button) == LOW) {  // Jika push button ditekan
    enrollMode = true;
  }

}


void writeAbsensiToSD(int id, const char* status, DateTime now) { //function menyimpan absensi terdaftar

  File dataFile = SD.open("absensi.csv", FILE_WRITE); // Membuka file "absensi.txt" dalam mode tulis

  if (dataFile) { // Memeriksa apakah file berhasil dibuka
    dataFile.print(F("ID: ")); // Menulis teks "ID: " ke file
    dataFile.print(id); // Menulis ID pengguna ke file
    dataFile.print(F(", Nama: ")); // Menulis teks ", Nama: " ke file
    char buffer[20]; // Membuat buffer untuk menyimpan nama pengguna
    strcpy_P(buffer, (char*)pgm_read_word(&(namaPengguna[id]))); // Menyalin nama pengguna dari memori PROGMEM ke buffer
    dataFile.print(buffer); // Menulis nama pengguna ke file
    dataFile.print(F(", Waktu: ")); // Menulis teks ", Waktu: " ke file
    if (now.hour() < 10) dataFile.print('0'); // Menambahkan '0' jika jam kurang dari 10
      dataFile.print(now.hour()); // Menulis jam saat ini ke file
      dataFile.print(':'); // Menulis karakter ':'
    if (now.minute() < 10) dataFile.print('0'); // Menambahkan '0' jika menit kurang dari 10
      dataFile.print(now.minute()); // Menulis menit saat ini ke file
      dataFile.print(':'); // Menulis karakter ':'
    if (now.second() < 10) dataFile.print('0'); // Menambahkan '0' jika detik kurang dari 10
      dataFile.print(now.second()); // Menulis detik saat ini ke file
      dataFile.print(F(", Tanggal: ")); // Menulis teks ", Tanggal: " ke file
    if (now.day() < 10) dataFile.print('0'); // Menambahkan '0' jika hari kurang dari 10
    dataFile.print(now.day()); // Menulis hari saat ini ke file
    dataFile.print('/'); // Menulis karakter '/'
    if (now.month() < 10) dataFile.print('0'); // Menambahkan '0' jika bulan kurang dari 10
    dataFile.print(now.month()); // Menulis bulan saat ini ke file
    dataFile.print('/'); // Menulis karakter '/'
    dataFile.print(now.year()); // Menulis tahun saat ini ke file
    dataFile.print(F(", Absensi: ")); // Menulis teks ", Absensi: " ke file
    dataFile.print(status); // Menulis status absensi ke file
    dataFile.println(); // Menulis newline ke file
    
    dataFile.close(); // Menutup file
  } else {
    Serial.println(F("Gagal membuka file absensi.txt")); // Jika file tidak bisa dibuka, cetak pesan error
  }
}

void writeAbsensiNotFound(const char* nama, DateTime now) { //Function menyimpan absensi yang tidak terdaftar
  // Membuka file "notfound.csv" dalam mode tulis. Jika file tidak ada, maka akan dibuat.
  File dataFile = SD.open("notfound.csv", FILE_WRITE);
  
  if (dataFile) { // Memeriksa apakah file berhasil dibuka
    dataFile.print(F("Nama: ")); // Menulis teks "Nama: " ke file
    dataFile.print(nama); // Menulis nama yang diberikan ke file
    dataFile.print(F(", Waktu: ")); // Menulis teks ", Waktu: " ke file
    
    // Menulis jam dengan format HH:MM:SS
    if (now.hour() < 10) dataFile.print('0'); // Menambahkan '0' jika jam kurang dari 10
    dataFile.print(now.hour()); // Menulis jam saat ini ke file
    dataFile.print(':'); // Menulis karakter ':'
    
    if (now.minute() < 10) dataFile.print('0'); // Menambahkan '0' jika menit kurang dari 10
    dataFile.print(now.minute()); // Menulis menit saat ini ke file
    dataFile.print(':'); // Menulis karakter ':'
    
    if (now.second() < 10) dataFile.print('0'); // Menambahkan '0' jika detik kurang dari 10
    dataFile.print(now.second()); // Menulis detik saat ini ke file
    
    dataFile.print(F(", Tanggal: ")); // Menulis teks ", Tanggal: " ke file
    
    // Menulis tanggal dengan format DD/MM/YYYY
    if (now.day() < 10) dataFile.print('0'); // Menambahkan '0' jika hari kurang dari 10
    dataFile.print(now.day()); // Menulis hari saat ini ke file
    dataFile.print('/'); // Menulis karakter '/'
    
    if (now.month() < 10) dataFile.print('0'); // Menambahkan '0' jika bulan kurang dari 10
    dataFile.print(now.month()); // Menulis bulan saat ini ke file
    dataFile.print('/'); // Menulis karakter '/'
    
    dataFile.print(now.year()); // Menulis tahun saat ini ke file
    
    dataFile.println(); // Menulis newline ke file
    dataFile.close(); // Menutup file
  } else {
    // Jika file tidak bisa dibuka, cetak pesan error ke Serial Monitor
    Serial.println(F("Gagal membuka file AbsenTidakTerdaftar.txt"));
  }
}




void hitungGaji() {
  int totalGaji[sizeof(namaPengguna) / sizeof(namaPengguna[0])] = {0}; // Array untuk menyimpan total gaji
  bool dataDiperbarui[sizeof(namaPengguna) / sizeof(namaPengguna[0])] = {false}; // Array untuk menandai data yang sudah diperbarui

  File absensiFile = SD.open("absensi.csv", FILE_READ); // Membuka file "absensi.txt" dalam mode baca
  if (absensiFile) { // Memeriksa apakah file berhasil dibuka
    while (absensiFile.available()) { // Membaca file sampai selesai
      String line = absensiFile.readStringUntil('\n'); // Membaca satu baris dari file
      if (line.startsWith("ID: ")) { // Jika baris dimulai dengan "ID: "
        int id = line.substring(4, line.indexOf(',')).toInt(); // Mendapatkan ID dari baris
        String status = line.substring(line.indexOf("Absensi: ") + 9); // Mendapatkan status absensi dari baris

        if (status.startsWith("Absen Masuk Tepat")) { // Jika status adalah "Absen Masuk Tepat"
          totalGaji[id] += 1; // Menambah jumlah masuk tepat waktu
        }
      }
    }
    absensiFile.close(); // Menutup file absensi

    // Membaca dan mengupdate data gaji di file gaji.txt
    File gajiFile = SD.open("gaji.csv", FILE_WRITE); // Membuka file "gaji.txt" dalam mode tulis
    if (gajiFile) { // Memeriksa apakah file berhasil dibuka
      bool isFirstLine = true; // Flag untuk baris pertama
      while (gajiFile.available()) { // Membaca file sampai selesai
        String line = gajiFile.readStringUntil('\n'); // Membaca satu baris dari file
        if (isFirstLine) { // Jika ini adalah baris pertama
          isFirstLine = false;
          continue; // Lewatkan baris pertama
        }

        int id = line.substring(line.indexOf("Nama: ") + 6, line.indexOf(", Jumlah Masuk Tepat Waktu: ")).toInt(); // Mendapatkan ID dari baris
        int jumlahMasukTepat = line.substring(line.indexOf("Jumlah Masuk Tepat Waktu: ") + 27, line.indexOf(", Jumlah Gaji: ")).toInt(); // Mendapatkan jumlah masuk tepat waktu dari baris
        int jumlahGaji = line.substring(line.indexOf("Jumlah Gaji: ") + 13).toInt(); // Mendapatkan jumlah gaji dari baris

        if (id >= 0 && id < sizeof(namaPengguna) / sizeof(namaPengguna[0])) { // Jika ID valid
          char buffer[20]; // Membuat buffer untuk menyimpan nama pengguna
          strcpy_P(buffer, (char*)pgm_read_word(&(namaPengguna[id]))); // Menyalin nama pengguna dari memori PROGMEM ke buffer
          if (!dataDiperbarui[id]) { // Jika data belum diperbarui
            totalGaji[id] += jumlahMasukTepat; // Menghitung ulang jumlah masuk tepat waktu
            jumlahGaji = totalGaji[id] * 100000; // Menghitung ulang total gaji
            dataDiperbarui[id] = true; // Menandai data sudah diperbarui
          }
          gajiFile.print(buffer); // Menulis nama pengguna ke file
          gajiFile.print(F(", Jumlah Masuk Tepat Waktu: ")); // Menulis teks ", Jumlah Masuk Tepat Waktu: " ke file
          gajiFile.print(totalGaji[id]); // Menulis jumlah masuk tepat waktu ke file
          gajiFile.print(F(", Jumlah Gaji: ")); // Menulis teks ", Jumlah Gaji: " ke file
          gajiFile.println(jumlahGaji); // Menulis jumlah gaji ke file
        }
      }

      // Menambahkan data baru jika belum ada di file gaji.txt
      for (int i = 1; i < sizeof(namaPengguna) / sizeof(namaPengguna[0]); i++) {
        if (!dataDiperbarui[i]) { // Jika data belum diperbarui
          char buffer[20]; // Membuat buffer untuk menyimpan nama pengguna
          strcpy_P(buffer, (char*)pgm_read_word(&(namaPengguna[i]))); // Menyalin nama pengguna dari memori PROGMEM ke buffer
          gajiFile.print(buffer); // Menulis nama pengguna ke file
          gajiFile.print(F(", Jumlah Masuk Tepat Waktu: ")); // Menulis teks ", Jumlah Masuk Tepat Waktu: " ke file
          gajiFile.print(totalGaji[i]); // Menulis jumlah masuk tepat waktu ke file
          gajiFile.print(F(", Jumlah Gaji: ")); // Menulis teks ", Jumlah Gaji: " ke file
          gajiFile.println(totalGaji[i] * 100000); // Menulis jumlah gaji ke file
        }
      }

      gajiFile.close(); // Menutup file gaji
      Serial.println(F("Data gaji berhasil diperbarui di gaji.txt")); // Mencetak pesan sukses
    } else {
      Serial.println(F("Gagal membuka file gaji.txt untuk mengupdate data gaji")); // Jika file tidak bisa dibuka, cetak pesan error
    }
  } else {
    Serial.println(F("Gagal membuka file absensi.txt untuk menghitung gaji")); // Jika file tidak bisa dibuka, cetak pesan error
  }
}

uint8_t getFingerprintID(DateTime now) { //Function untuk mendapatkan sidik jari absensi
  uint8_t p = finger.getImage(); // Mengambil gambar sidik jari
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image taken"));
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(F("No finger detected"));
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println(F("Imaging error"));
      return p;
    default:
      Serial.println(F("Unknown error"));
      return p;
  }

  p = finger.image2Tz(); // Mengonversi gambar ke template sidik jari
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println(F("Image too messy"));
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    default:
      Serial.println(F("Unknown error"));
      return p;
  }

  p = finger.fingerSearch(); // Mencari kecocokan sidik jari di dalam database
  if (p == FINGERPRINT_OK) {
    Serial.println(F("Found a print match!"));

    lcd.clear();
    lcd.setCursor(0, 0);
    digitalWrite(ledBlue, HIGH); // Menyalakan LED merah
    delay(1000);
    digitalWrite(ledBlue, LOW); // Mematikan LED merah

    const char* status;
    if (now.hour() >= 7 && now.hour() < 15) { // Jika waktu sekarang antara jam 8 dan 10
      if (now.hour() >= 8) { // Jika waktu sekarang lebih besar atau sama dengan jam 8
        lcd.print(F("Absen Masuk "));
        lcd.setCursor(0, 1);
        lcd.print(F("Terlambat "));
        delay(1000);
        status = "Absen Masuk Terlambat"; // Mengatur status absen terlambat
      } else {
        lcd.print(F("Absen Masuk "));
        lcd.setCursor(0, 1);
        lcd.print(F("Tepat Waktu"));
         delay(1000);
        status = "Absen Masuk Tepat"; // Mengatur status absen tepat
      }
    } else if (now.hour() >= 15 && now.hour() < 17) { // Jika waktu sekarang antara jam 11 dan 24
      lcd.print(F("Absen Pulang"));
      status = "Absen Pulang"; // Mengatur status absen pulang
    } else {
      lcd.print(F("Bukan Waktu Absensi"));
      delay(2000);
      lcd.clear();
      return p; // Mengembalikan nilai p jika bukan waktu absensi
    }

    lcd.setCursor(0, 1);
    printName(finger.fingerID); // Menampilkan nama berdasarkan ID sidik jari
    writeAbsensiToSD(finger.fingerID, status, now); // Menulis data absensi ke SD card
    delay(2000);
    lcd.clear();

  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    return p; // Mengembalikan nilai p jika terjadi kesalahan komunikasi
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println(F("Did not find a match"));
    lcd.clear();
    digitalWrite(ledRed, HIGH); // Menyalakan LED merah
    digitalWrite(buzzer, HIGH); 
    lcd.print(F("Tidak terdaftar")); 
    delay(2000);
    digitalWrite(buzzer, LOW); 
    digitalWrite(ledRed, LOW); // Mematikan LED merah
    lcd.clear();
    writeAbsensiNotFound("Tidak Terdaftar", now);

    return p; // Mengembalikan nilai p jika tidak ditemukan kecocokan sidik jari
  } else {
    Serial.println(F("Unknown error"));
    return p; // Mengembalikan nilai p jika terjadi kesalahan tidak dikenal
  }

  Serial.print(F("Found ID #"));
  Serial.print(finger.fingerID);
  Serial.print(F(" with confidence of "));
  Serial.println(finger.confidence); // Menampilkan ID dan confidence dari kecocokan sidik jari

  return finger.fingerID; // Mengembalikan ID sidik jari yang ditemukan
}


void enrollFingerprint() { //function mendaftarkan absensi
  int id = finger.templateCount + 1; // Menentukan ID baru untuk sidik jari yang akan didaftarkan
  lcd.clear();
  lcd.print(F("Masukan Sidik Jari"));
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage(); // Mengambil gambar sidik jari
    if (p == FINGERPRINT_NOFINGER) continue; // Jika tidak ada jari, lanjutkan loop
    if (p == FINGERPRINT_IMAGEFAIL) {
      lcd.clear();
      lcd.print(F("Gagal ambil gambar"));
      delay(2000);
      return; // Jika gagal mengambil gambar, keluar dari fungsi
    }
  }
  p = finger.image2Tz(1); // Mengonversi gambar ke template sidik jari
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print(F("Gagal konversi"));
    delay(2000);
    return; // Jika gagal mengonversi gambar, keluar dari fungsi
  }

  lcd.clear();
  lcd.print(F("Lepaskan jari"));
  delay(2000);
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage(); // Menunggu jari dilepaskan
  }

  lcd.clear();
  lcd.print(F("Masukkan jari"));
  delay(2000);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage(); // Mengambil gambar sidik jari lagi
    if (p == FINGERPRINT_NOFINGER) continue; // Jika tidak ada jari, lanjutkan loop
    if (p == FINGERPRINT_IMAGEFAIL) {
      lcd.clear();
      lcd.print(F("Gagal ambil gambar"));
      delay(2000);
      return; // Jika gagal mengambil gambar, keluar dari fungsi
    }
  }
  p = finger.image2Tz(2); // Mengonversi gambar ke template sidik jari lagi
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print(F("Gagal konversi"));
    delay(2000);
    return; // Jika gagal mengonversi gambar, keluar dari fungsi
  }

  p = finger.createModel(); // Membuat model sidik jari
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print(F("Gagal membuat model"));
    delay(2000);
    return; // Jika gagal membuat model, keluar dari fungsi
  }

  p = finger.storeModel(id); // Menyimpan model sidik jari ke database
  if (p == FINGERPRINT_OK) {
    lcd.clear();
    lcd.print(F("tersimpan"));
    delay(2000);
    lcd.clear();
  } else {
    lcd.clear();
    lcd.print(F("Gagal menyimpan"));
    delay(2000);
    lcd.clear(); // Jika gagal menyimpan model, tampilkan pesan error
  }

  enrollMode = false; // Mengatur mode pendaftaran ke false
}


