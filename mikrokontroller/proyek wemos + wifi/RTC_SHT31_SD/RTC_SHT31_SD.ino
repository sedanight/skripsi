#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h> 
#include <SD.h>
#include "RTClib.h"
#include "Adafruit_SHT31.h"
bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
RTC_DS3231 rtc;
char dataHari[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
String hari;
int tanggal, bulan, tahun, jam, menit, detik;
File myFile;
float h,t;

void setup () {   
  Serial.begin(115200);
  Serial.println("Mulai....");
  delay(1000);
  while (!Serial);
  delay(10);
  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
  Serial.println("DISABLED");
  delay(1000);
  if (!SD.begin(10)) 
  {
  Serial.println("Gagal Membuka Micro SD!");
  return;
  }
  Serial.println("Berhasil Membuka Micro SD");
  delay(1000);
  if (! rtc.begin()) {
  Serial.println("RTC Tidak Ditemukan");
  Serial.flush();
  abort();
  }
 //Atur Waktu
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //  rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0)); 
  Serial.println("Memerikasa Koneksi RTC . . ."); 
  delay(1000);
}

void loop () {
  DateTime now = rtc.now();
  hari    = dataHari[now.dayOfTheWeek()];
  tanggal = now.day(), DEC;
  bulan   = now.month(), DEC;
  tahun   = now.year(), DEC;
  jam     = now.hour(), DEC;
  menit   = now.minute(), DEC;
  detik   = now.second(), DEC;
  Serial.println(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
  Serial.println(String() + jam + ":" + menit + ":" + detik);
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Failed to read humidity");
  }
  // Toggle heater enabled state every 30 seconds
  // An ~3.0 degC temperature increase can be noted when heater is enabled
  if (++loopCnt == 30) {
    enableHeater = !enableHeater;
    sht31.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");
    loopCnt = 0;
  }
 // Kirim data ke SD card 
 myFile = SD.open("Data_Log.csv", FILE_WRITE); //Membuka File test.txt
 if (myFile) // jika file tersedia tulis data
 {
  myFile.println(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
  myFile.println(String() + jam + ":" + menit + ":" + detik);
  myFile.println(String() + t + "°C");
  myFile.println(String() + h + "%");
 }
 else 
 {
 Serial.println("gagal membuka Data_Log.txt"); // jika gagal print error
 }
 delay(3000); 
}
