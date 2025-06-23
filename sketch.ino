#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "Arroyyan"
#define WIFI_PASSWORD "MuhammadArroyyan#"
#define API_KEY "AIzaSyCL01ZczH3DV1pDL0drOTmnKgukk-YvJuk"
#define DATABASE_URL "https://smart-office-room-23-default-rtdb.asia-southeast1.firebasedatabase.app"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

// === Komponen Keluaran ===
LiquidCrystal_I2C lcd(0x27, 16, 2);

// === Pin Sensor ===
#define TRIG_PIN 5
#define ECHO_PIN 18
#define DHT_PIN 15
#define LDR_PIN 34
#define LED_KUNING 2
#define LED_MERAH 4

#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// === Ambang Batas ===
#define LDR_THRESHOLD 1000
#define HUMIDITY_THRESHOLD 60.0

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  dht.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }
  Serial.println("\nWiFi Connected!");
  lcd.clear();
  lcd.print("WiFi Connected!");

  // === Konfigurasi Firebase ===
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  // Login anonymous
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("SignUp OK");
    signupOK = true;
  } else {
    Serial.printf("SignUp error: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  delay(2000);
  lcd.clear();
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT!");
    lcd.setCursor(0, 0);
    lcd.print("DHT Error!");
    delay(1000);
    return;
  }

  // Baca jarak
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  // Baca LDR
  int ldrValue = analogRead(LDR_PIN);

  // Kontrol LED
  digitalWrite(LED_KUNING, ldrValue < LDR_THRESHOLD ? HIGH : LOW);
  digitalWrite(LED_MERAH, humidity > HUMIDITY_THRESHOLD ? HIGH : LOW);

  // Tampilkan LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("T:%.1fC H:%d%%", temperature, (int)humidity);
  lcd.setCursor(0, 1);
  lcd.printf("D:%dcm LDR:%d", (int)distance, ldrValue);

  // Kirim ke Firebase
  if (Firebase.ready() && signupOK) {
    String path = "/sensor";
    Firebase.RTDB.setFloat(&fbdo, path + "/temperature", temperature);
    Firebase.RTDB.setFloat(&fbdo, path + "/humidity", humidity);
    Firebase.RTDB.setFloat(&fbdo, path + "/distance", distance);
    Firebase.RTDB.setInt(&fbdo, path + "/ldr", ldrValue);
    Firebase.RTDB.setBool(&fbdo, path + "/led_merah_status", humidity > HUMIDITY_THRESHOLD);
    Firebase.RTDB.setBool(&fbdo, path + "/led_kuning_status", ldrValue < LDR_THRESHOLD);
    Serial.println("Data sent to Firebase.");
  } else {
    Serial.println("Firebase not ready. Reason: " + fbdo.errorReason());
  }

  delay(5000);
}