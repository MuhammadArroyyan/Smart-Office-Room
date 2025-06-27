#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

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

  // Kontrol LED (Diringkas agar lebih efisien)
  digitalWrite(LED_KUNING, (ldrValue < LDR_THRESHOLD));
  digitalWrite(LED_MERAH, (humidity > HUMIDITY_THRESHOLD));

  // Tampilkan LCD
  lcd.clear();
  // Baris 1: Suhu & Kelembapan
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1); // Tampilkan 1 angka di belakang koma
  lcd.print("C H:");
  lcd.print((int)humidity);
  lcd.print("%");

  // Baris 2: Jarak & Cahaya
  lcd.setCursor(0, 1);
  lcd.print("D:");
  lcd.print((int)distance);
  lcd.print("cm LDR:");
  lcd.print(ldrValue);

  delay(5000);
}