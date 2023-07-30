#include <Wire.h>
#include "RTClib.h"
#include "Adafruit_SHT31.h"
#include <SPI.h> 
#include <SD.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
RTC_DS3231 rtc;
char dataHari[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String hari;
int tanggal, bulan, tahun, jam, menit, detik;
File myFile;
float suhu;
float kelembaban;
int period = 300000;
unsigned long time_now = 0;
unsigned long byteCount = 0;
bool printWebData = true;
const char* ssid = "@netUNSRI-IDL"; //masukkan ssid
const char* password = ""; //masukkan password
const char* server = "10.8.199.13";
WiFiClient client;

void setup () {   
  WiFi.disconnect();
  WiFi.begin("@netUNSRI-IDL","");
  while ((!(WiFi.status() == WL_CONNECTED))){
   delay(3000);
   }
  Serial.begin(9600);
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
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting..");
  }
 if(WiFi.status() == WL_CONNECTED){
    Serial.println("Connected!!!");
  }
  else{
    Serial.println("Connected Failed!!!");
  } 
  if (!SD.begin(10)) 
  {
  Serial.println("Gagal Membuka Micro SD!");
  return;
  }
  Serial.println("Berhasil Membuka Micro SD");
  if (! rtc.begin()) {
  Serial.println("RTC Tidak Ditemukan");
  Serial.flush();
  abort();
  }
 //Atur Waktu
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //  rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0)); 
  Serial.println("Memerikasa Koneksi RTC . . .");
}
void loop (){
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
  suhu = sht31.readTemperature();
  kelembaban = sht31.readHumidity();
  if (! isnan(suhu)) {  // check if 'is not a number'
    Serial.print("Temp °C = "); Serial.print(suhu); Serial.print("\t\t");
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(kelembaban)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(kelembaban);
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
 if (myFile) {
  myFile.println(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
  myFile.println(String() + jam + ":" + menit + ":" + detik);
  myFile.println(String() + suhu + "°C");
  myFile.println(String() + kelembaban + "%");
 }
 else {
 Serial.println("gagal membuka Data_Log.csv"); // jika gagal print error
 delay(60000);
 }
 if(millis() >= time_now + period){
    time_now += period;
    SendtoDB();
  } 
  int len = client.available();
  if (len > 0) {
        byte buffer[80];
        if (len > 80) len = 80;
        client.read(buffer, len);
        if (printWebData) {
          Serial.write(buffer, len); // show in the serial monitor (slows some boards)
        }
        byteCount = byteCount + len;
  } 
}
void SendtoDB(){
   if (client.connect(server, 80)) {
    Serial.println("");
    Serial.println("connected");
    // Make a HTTP request:
    Serial.print("GET /esp8266/tambah.php?kelembaban=");
    Serial.print(kelembaban);
    Serial.print("&suhu=");
    Serial.println(suhu);
    Serial.println("");
    
    client.print("GET /esp8266/tambah.php?kelembaban=");     //YOUR URL
    client.print(kelembaban);
    client.print("&suhu=");
    client.print(suhu);
    client.print("&tanggal=");
    client.print(String() + tanggal + "-" + bulan + "-" + tahun);
    client.print("&waktu=");
    client.print(String() + jam + ":" + menit + ":" + detik);
    client.print(" ");      //SPACE BEFORE HTTP/1.1
    client.print("HTTP/1.1");
    client.println();
    client.println("Host: 10.8.199.13");
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
 }
