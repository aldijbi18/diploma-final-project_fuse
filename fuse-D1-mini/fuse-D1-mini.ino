#include<SoftwareSerial.h> // library UART
#include<ESP8266WiFi.h> // library WiFi
#include <FirebaseArduino.h> // library Firebase
#define FIREBASE_HOST "your_project_link.firebaseio.com" 
// project host Firebase
#define FIREBASE_AUTH "your_project_secret" 
// project secret Firebase
#define WIFI_SSID "Wifi_SSID" // SSID WiFi
#define WIFI_PASSWORD "Wifi_Password" // password WiFi
int a1, a2, b1, b2, token; // deklarasi variabel
double kwh = 0; // deklarasi variabel
String up, down; // deklarasi variabel
void wifi() { // konfigurasi mencari koneksi dan inisiasi URL Firebase
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // inisiasi Firebase
}
void saldo() { // fungsi untuk mengirim nilai kWh
  up = "";
  while (Serial.available() != 0) {
    up = up + String(char (Serial.read()));
  }
  if (up != 0)
  {
    Firebase.setString("kwh", up);
  }
  Firebase.setString("token", "0");
}
void isi() { // fungsi untuk mengisi token
  down = "";
  String a = Firebase.getString("token");
  a1 = a.indexOf('"');
  a2 = a.indexOf('"', a1 + 1);
  String tkn = a.substring(a1 + 1, a2);
  token = tkn.toInt();
  String b = Firebase.getString("kwh");
  b1 = b.indexOf('"');
  b2 = b.indexOf('"', b1 + 1);
  String kd = b.substring(b1 + 1, b2);
  kwh = kd.toDouble();
  down += ";";
  down += token;
  down += ";";
  down += kwh;
  down += ";";
}
void setup() {
  Serial.begin(9600); // konfigurasi baudrate
  wifi(); // memanggil fungsi wifi
}
void loop() {
  if (WiFi.status() != WL_CONNECTED) wifi();
  // jika koneksi terputus, memanggil wifi()
  isi();
  Serial.println(down);
  saldo();
  delay(700);
}
