#include <WiFi.h>

// Konfigurasi WiFi - GANTI dengan kredensial WiFi Anda
const char* ssid = "fuad";
const char* password = "123456789";

// Pin Definitions
#define TRIG_PIN 5          // GPIO5 - Ultrasonik TRIG
#define ECHO_PIN 18         // GPIO18 - Ultrasonik ECHO
#define RAIN_ANALOG_PIN 34  // GPIO34 (ADC1_6) - Sensor Hujan Analog
#define RAIN_DIGITAL_PIN 35 // GPIO35 (ADC1_7) - Sensor Hujan Digital

// Variabel global untuk menyimpan data sensor
float distance = 0;
int rainAnalog = 0;
int rainDigital = 0;
String rainStatus = "";

// Fungsi untuk membaca jarak dari sensor ultrasonik
float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout 30ms
  
  if (duration == 0) {
    return -1; // Error atau out of range
  }
  
  float dist = duration * 0.034 / 2; // Konversi ke cm
  return dist;
}

// Fungsi untuk membaca sensor hujan
void readRainSensor() {
  // Baca nilai analog (0-4095 untuk ESP32)
  rainAnalog = analogRead(RAIN_ANALOG_PIN);
  
  // Baca nilai digital (HIGH = tidak ada hujan, LOW = ada hujan)
  rainDigital = digitalRead(RAIN_DIGITAL_PIN);
  
  // Tentukan status: Hujan atau Kering
  // Semakin rendah nilai = semakin basah
  // Threshold: 3000 (sesuaikan jika perlu)
  if (rainAnalog > 3000) {
    rainStatus = "Kering";
  } else {
    rainStatus = "Hujan";
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Tunggu serial monitor siap
  
  Serial.println("\n\n========================================");
  Serial.println("   ESP32 Sensor Monitor - Simple Mode");
  Serial.println("   Ultrasonik + Sensor Hujan");
  Serial.println("========================================\n");
  
  // Setup pin ultrasonik
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("✓ Sensor Ultrasonik diinisialisasi");
  
  // Setup pin sensor hujan
  pinMode(RAIN_DIGITAL_PIN, INPUT);
  // Pin analog tidak perlu pinMode untuk ESP32
  Serial.println("✓ Sensor Hujan diinisialisasi");
  
  // Test baca sensor hujan
  int testRain = analogRead(RAIN_ANALOG_PIN);
  Serial.print("Test Sensor Hujan - Nilai: ");
  Serial.println(testRain);
  
  if (testRain == 0 || testRain == 4095) {
    Serial.println("⚠️ WARNING: Sensor hujan mungkin tidak terhubung!");
    Serial.println("   Cek: 1) Kabel AO ke GPIO34");
    Serial.println("        2) Kabel DO ke GPIO35");
    Serial.println("        3) VCC ke 3.3V/5V dan GND");
  }
  
  // Koneksi ke WiFi
  Serial.print("\n⏳ Menghubungkan ke WiFi: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  
  WiFi.mode(WIFI_STA); // Mode Station
  WiFi.begin(ssid, password);
  
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 30) {
    delay(500);
    Serial.print(".");
    attempt++;
    
    // Tampilkan status koneksi
    if (attempt % 10 == 0) {
      Serial.println();
      Serial.print("Status WiFi: ");
      switch(WiFi.status()) {
        case WL_NO_SSID_AVAIL:
          Serial.println("SSID tidak ditemukan!");
          break;
        case WL_CONNECT_FAILED:
          Serial.println("Koneksi gagal - cek password!");
          break;
        case WL_DISCONNECTED:
          Serial.println("Terputus...");
          break;
        default:
          Serial.println("Menghubungkan...");
      }
    }
  }
  
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✓ WiFi terhubung!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("✗ Gagal terhubung ke WiFi!");
    Serial.println("Kemungkinan masalah:");
    Serial.println("  1. Password salah");
    Serial.println("  2. Router menggunakan 5GHz (gunakan 2.4GHz)");
    Serial.println("  3. Signal terlalu lemah");
    Serial.println("\nProgram tetap berjalan tanpa WiFi...");
  }
  
  Serial.println("\n========================================");
  Serial.println("Mulai membaca sensor setiap 2 detik...");
  Serial.println("========================================\n");
  
  delay(1000);
}

void loop() {
  // Baca semua sensor
  distance = readDistance();
  readRainSensor();
  
  // Tampilkan data di Serial Monitor
  Serial.println("╔════════════════════════════════════════════╗");
  Serial.println("║          DATA SENSOR TERBARU               ║");
  Serial.println("╠════════════════════════════════════════════╣");
  
  // Data Jarak
  Serial.print("║ Jarak           : ");
  if (distance > 0 && distance < 400) {
    if (distance < 10) {
      Serial.print(" ");
    }
    Serial.print(distance, 1);
    Serial.println(" cm                  ║");
  } else {
    Serial.println("Out of range              ║");
  }
  
  // Data Sensor Hujan - Analog
  Serial.print("║ Hujan (Analog)  : ");
  Serial.print(rainAnalog);
  if (rainAnalog < 10) {
    Serial.print("    ");
  } else if (rainAnalog < 100) {
    Serial.print("   ");
  } else if (rainAnalog < 1000) {
    Serial.print("  ");
  } else {
    Serial.print(" ");
  }
  Serial.println("                  ║");
  
  // Data Sensor Hujan - Status
  Serial.print("║ Status Hujan    : ");
  Serial.print(rainStatus);
  int spaces = 21 - rainStatus.length();
  for (int i = 0; i < spaces; i++) {
    Serial.print(" ");
  }
  Serial.println("║");
  
  // Data Sensor Hujan - Digital
  Serial.print("║ Digital Pin     : ");
  if (rainDigital == HIGH) {
    Serial.println("Tidak Hujan              ║");
  } else {
    Serial.println("Hujan Terdeteksi         ║");
  }
  
  Serial.println("╠════════════════════════════════════════════╣");
  Serial.println("║           INFORMASI WIFI                   ║");
  Serial.println("╠════════════════════════════════════════════╣");
  
  // Status WiFi
  Serial.print("║ Status          : ");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Terhubung                ║");
    
    // SSID
    Serial.print("║ SSID            : ");
    Serial.print(WiFi.SSID());
    int ssidSpaces = 21 - WiFi.SSID().length();
    for (int i = 0; i < ssidSpaces; i++) {
      Serial.print(" ");
    }
    Serial.println("║");
    
    // IP Address
    Serial.print("║ IP Address      : ");
    String ip = WiFi.localIP().toString();
    Serial.print(ip);
    int ipSpaces = 21 - ip.length();
    for (int i = 0; i < ipSpaces; i++) {
      Serial.print(" ");
    }
    Serial.println("║");
    
    // Gateway
    Serial.print("║ Gateway         : ");
    String gateway = WiFi.gatewayIP().toString();
    Serial.print(gateway);
    int gwSpaces = 21 - gateway.length();
    for (int i = 0; i < gwSpaces; i++) {
      Serial.print(" ");
    }
    Serial.println("║");
    
    // Subnet Mask
    Serial.print("║ Subnet Mask     : ");
    String subnet = WiFi.subnetMask().toString();
    Serial.print(subnet);
    int subSpaces = 21 - subnet.length();
    for (int i = 0; i < subSpaces; i++) {
      Serial.print(" ");
    }
    Serial.println("║");
    
    // MAC Address
    Serial.print("║ MAC Address     : ");
    String mac = WiFi.macAddress();
    Serial.print(mac);
    int macSpaces = 21 - mac.length();
    for (int i = 0; i < macSpaces; i++) {
      Serial.print(" ");
    }
    Serial.println("║");
    
    // Signal Strength (RSSI)
    Serial.print("║ Signal (RSSI)   : ");
    int rssi = WiFi.RSSI();
    Serial.print(rssi);
    Serial.print(" dBm");
    
    // Indikator kualitas signal
    String quality;
    if (rssi > -50) {
      quality = " (Sangat Baik)";
    } else if (rssi > -60) {
      quality = " (Baik)";
    } else if (rssi > -70) {
      quality = " (Cukup)";
    } else {
      quality = " (Lemah)";
    }
    Serial.print(quality);
    
    int rssiStr = String(rssi).length() + 4 + quality.length(); // angka + " dBm" + quality
    int rssiSpaces = 21 - rssiStr;
    for (int i = 0; i < rssiSpaces; i++) {
      Serial.print(" ");
    }
    Serial.println("║");
    
    // Channel
    Serial.print("║ Channel         : ");
    Serial.print(WiFi.channel());
    int chSpaces = 19 - String(WiFi.channel()).length();
    for (int i = 0; i < chSpaces; i++) {
      Serial.print(" ");
    }
    Serial.println("║");
    
  } else {
    Serial.println("Terputus                 ║");
    Serial.println("║ SSID            : -                        ║");
    Serial.println("║ IP Address      : -                        ║");
    Serial.println("║ Signal          : -                        ║");
  }
  
  Serial.println("╚════════════════════════════════════════════╝");
  Serial.println();
  
  delay(2000); // Update setiap 2 detik
}