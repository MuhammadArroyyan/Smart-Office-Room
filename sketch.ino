#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "Arroyyan"
#define WIFI_PASSWORD "MuhammadArroyyan#"

#define API_KEY "AIzaSyCL01ZczH3DV1pDL0drOTmnKgukk-YvJuk"
#define DATABASE_URL "https://smart-office-room-23-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// === Perangkat Keras ===
// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definisi Pin
#define TRIG_PIN    5   // Ultrasonik Trigger
#define ECHO_PIN    18  // Ultrasonik Echo
#define DHT_PIN     15  // DHT22 Data Pin
#define LDR_PIN     34  // LDR Analog Pin
#define LED_KUNING  2   // LED untuk Indikator Cahaya
#define LED_MERAH   4   // LED untuk Indikator Kelembapan

// Sensor DHT
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// === Threshold (Ambang Batas) ===
#define LDR_THRESHOLD       1000  // Nilai LDR saat dianggap gelap (0-4095, perlu dikalibrasi)
#define HUMIDITY_THRESHOLD  60.0  // Kelembapan dalam %

void setup() {
  Serial.begin(115200);

  // Inisialisasi Perangkat
  lcd.init();
  lcd.backlight();
  dht.begin();

  // Konfigurasi Mode Pin
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LDR_PIN, INPUT); // Pin ADC tidak perlu di-set mode-nya, tapi ini untuk kejelasan
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);

  // --- Koneksi WiFi ---
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    lcd.setCursor(0, 1);
    lcd.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");

  // --- Inisialisasi Firebase ---
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  // Menyiapkan token (Penting untuk Firebase)
  config.token_status_callback = tokenStatusCallback; // Lihat fungsi di bawah

  lcd.setCursor(0, 1);
  lcd.print("Firebase Ready!");
  delay(2000);
  lcd.clear();
}

void loop() {
  // --- 1. Membaca Semua Sensor ---

  // Baca Sensor DHT22
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // PERBAIKAN: Cek jika pembacaan DHT gagal (akan menghasilkan NaN)
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    lcd.setCursor(0, 0);
    lcd.print("DHT Error!");
    delay(1000);
    return; // Lewati sisa loop jika sensor error
  }

  // Baca Sensor Ultrasonik
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  // Baca Sensor LDR
  int ldrValue = analogRead(LDR_PIN);

  // --- 2. Logika Kontrol LED ---

  // PERBAIKAN: Logika LED Kuning (LDR)
  if (ldrValue < LDR_THRESHOLD) {
    digitalWrite(LED_KUNING, HIGH); // Ruangan gelap, nyalakan lampu (LED Kuning)
  } else {
    digitalWrite(LED_KUNING, LOW);
  }

  // PERBAIKAN: Logika LED Merah (Kelembapan)
  if (humidity > HUMIDITY_THRESHOLD) {
    digitalWrite(LED_MERAH, HIGH); // Kelembapan tinggi, nyalakan LED Merah
  } else {
    digitalWrite(LED_MERAH, LOW);
  }

  // --- 3. Menampilkan Data ke LCD ---
  
  lcd.clear();
  // Baris 1: Suhu & Kelembapan
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print((char)223); // Simbol derajat
  lcd.print("C H:");
  lcd.print(humidity, 0);
  lcd.print("%");
  
  // Baris 2: Jarak & Cahaya
  lcd.setCursor(0, 1);
  lcd.print("Dist:");
  lcd.print(distance, 0);
  lcd.print(" LDR:");
  lcd.print(ldrValue);


  // --- 4. Mengirim Data ke Firebase ---
  
  if (Firebase.ready()) {
    String path = "/sensor"; // Path utama di Firebase
    Firebase.setFloat(fbdo, path + "/temperature", temperature);
    Firebase.setFloat(fbdo, path + "/humidity", humidity);
    Firebase.setFloat(fbdo, path + "/distance", distance);
    Firebase.setInt(fbdo, path + "/ldr", ldrValue);

    // Kirim juga status LED agar website tahu
    Firebase.setBool(fbdo, path + "/led_merah_status", (humidity > HUMIDITY_THRESHOLD));
    Firebase.setBool(fbdo, path + "/led_kuning_status", (ldrValue < LDR_THRESHOLD));

    Serial.println("Data sent to Firebase successfully.");
  } else {
    Serial.println("Firebase not ready. Reason: " + fbdo.errorReason());
  }

  // Jeda sebelum loop berikutnya
  delay(5000); // Kirim data setiap 5 detik agar tidak membanjiri Firebase
}