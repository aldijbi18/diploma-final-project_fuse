#include <Keypad.h> // library keypad
#include <Wire.h> // library I2C
#include <LiquidCrystal_I2C.h> // library LCD I2C
#include <SoftwareSerial.h> // library UART
LiquidCrystal_I2C lcd(0x27, 16, 2); //alamat dan ukuran LCD
SoftwareSerial wemos(12, 11); // port koneksi ke board D1 Mini
#define relay 13 // pin relay
#define buzz 10 // pin buzzer
const int sensorIn = A3; // pin sensor arus
int mVperAmp = 185; // sensitivitas sensor arus
float offset = 0.132; // nilai margin arus
double Voltage = 0; // deklarasi variabel
double VRMS = 0; // deklarasi variabel
double AmpsRMS = 0; // deklarasi variabel
double power = 0; // deklarasi variabel
double ss = 0; // deklarasi variabel
double saldo = 0; // deklarasi variabel
double k = 0; //deklarasi variabel
int token; // deklarasi variabel
int num1 = 0; // deklarasi variabel
String down, up, tkn, kwhdep; // deklarasi variabel
int k1, k2, k3; // deklarasi variabel
const byte ROWS = 4; // deklarasi baris keypad
const byte COLS = 4; //deklarasi kolom keypad
char keys[ROWS][COLS] = { // array keypad
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // pin yang digunakan baris keypad
byte colPins[COLS] = {5, 4, 3, 2}; // pin yang digunakan kolom keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
// mengaktifkan array keypad
byte lghtng[] = { // custom character LCD Crystal
  B00011,
  B00110,
  B01100,
  B11111,
  B00110,
  B01100,
  B11000,
  B10000
};
byte pwr[] = { // custom character LCD Crystal
  B01110,
  B11111,
  B10001,
  B10101,
  B10001,
  B10111,
  B10111,
  B11111
};
float getVPP() // program pembacaan ADC sensor arus
{
  float result;
  int readValue;
  int maxValue = 0;
  int minValue = 1024;
  for ( int i = 0; i < 1000; i++)
  {
    readValue = analogRead(sensorIn);
    if (readValue > maxValue)
    {
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      minValue = readValue;
    }
    delay(1);
  }
  result = ((maxValue - minValue) * 5.0) / 1024.0;
  return result;
}
void d() { // program mendapatkan nilai token dan kwh deposit
  down = "";
  while (wemos.available() != 0) {
    down = down + String(char (wemos.read()));
  }
  k1 = down.indexOf(';');
  k2 = down.indexOf(';', k1 + 1);
  k3 = down.indexOf(';', k2 + 1);
  tkn = down.substring(k1 + 1, k2);
  kwhdep = down.substring(k2 + 1, k3);
}
void isi() { // program pengisian menggunakan keypad
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Isi Token Anda");
  lcd.setCursor(0, 1);
  lcd.print("Token :");
  while (1)
  {
    char button = keypad.getKey();
    if (button >= '0' && button <= '9')
    {
      num1 = num1 * 10 + (button - '0');
      lcd.setCursor(8, 1);
      lcd.print(num1);
    }
    if (button == '#') {
      saldo = saldo + num1;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Token Terisi!!");
      lcd.setCursor(1, 1);
      lcd.print("Saldo : ");
      lcd.setCursor(9, 1);
      lcd.print(saldo);
      delay(1000);
      num1 = 0;
      break;
    }
    else if ((button == 'A') || (button == 'B') || (button == 'C') ||
             (button == 'D') || (button == '*')) {
      lcd.clear();
      num1 = 0;
      break;
    }
  }
}
void kwhcount () { // program pengurangan kwh
  Voltage = getVPP(); // memanggil fungsi void getVPP
  VRMS = (Voltage / 2.0) * 0.707; // persamaan untuk menemukan Vrms
  AmpsRMS = ((VRMS * 1000) / mVperAmp) - offset; // persamaan untuk menemukan arus
  if (AmpsRMS < 0 ) AmpsRMS = 0; // menghilangkan nilai negatif pada arus
  power = AmpsRMS * 225; // persamaan menghitung daya beban
  saldo = saldo - (power / 3600000); // persamaan pengurangan token
}
void buzzer () { // program logika tempo bunyi buzzer terhadap kwh
  if (saldo > 0) { // logika kWh dibawah 5 untuk mengaktifkan buzzer tempo cepat
    if (saldo <= 5) {
      digitalWrite(buzz, HIGH);
      delay(300);
      digitalWrite(buzz, LOW);
    }
    digitalWrite(relay, LOW);
  }
  else if (saldo <= 0) { // logika kWh dibawah 0 untuk mengaktifkan buzzer tempo lambat
    saldo = 0;
    digitalWrite(relay, HIGH);
    digitalWrite(buzz, HIGH);
    delay(700);
    digitalWrite(buzz, LOW);
  }
  ss = saldo;
}
void serialmonitor () { // program menampilkan parameter pada Serial Monitor
  Serial.print("Saldo : ");
  Serial.print(saldo, 9);
  Serial.print(" kWh, P : ");
  Serial.print(power);
  Serial.print(" W, I : ");
  Serial.print(AmpsRMS, 9);
  Serial.println(" A");
}
void lcdtampil () { // program menampilkan parameter pada LCD Crystal
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.write(1);
  lcd.setCursor(3, 0);
  lcd.print(saldo, 4);
  lcd.setCursor(12, 0);
  lcd.print("kWh");
  lcd.setCursor(1, 1);
  lcd.write(0);
  lcd.setCursor(3, 1);
  lcd.print(power, 4);
  lcd.setCursor(13, 1);
  lcd.print("W");
}
void setup() {
  lcd.begin(); // menginisiasi LCD
  lcd.clear(); // membersihkan tampilan LCD
  lcd.backlight(); // konfigurasi kecerahan LCD
  lcd.createChar(0, lghtng); // membentuk custom character
  lcd.createChar(1, pwr); // membentuk custom character
  Serial.begin(9600); // menginisiasi baudrate untuk Serial Monitor
  wemos.begin(9600); // menginisiasi baudrate untuk board D1 Mini
  pinMode(relay, OUTPUT); // konfigurasi pin Relay
  pinMode(buzz, OUTPUT); // konfigurasi pin buzzer
  lcd.clear(); // membersihkan tampilan LCD
  lcd.setCursor(6, 0); // setting letak cursor LCD
  lcd.print("FUSE"); // menampilkan tulisan pada LCD
  lcd.setCursor(0, 1); // setting letak cursor LCD
  lcd.print("Input & Monitor!"); // menampilkan tulisan pada LCD
  delay(2000); // menahan tampilan selama 2 detik
  lcd.clear(); // membersihkan tampilan LCD
  lcd.setCursor(2, 0); // setting letak cursor LCD
  lcd.print("TUGAS AKHIR:"); // menampilkan tulisan pada LCD
  lcd.setCursor(4, 1); // setting letak cursor LCD
  lcd.print("17201002"); // menampilkan tulisan pada LCD
  delay(1000); // menahan tampilan selama 1 detik
  d();
  k = kwhdep.toDouble(); // mendapatkan kwh deposit
  saldo = k; // menyimpan kwh deposit ke prototype
}
void loop() {
  d(); // program pengambilan kwh deposit dan token
  token = tkn.toInt(); // mendapatkan token
  saldo = saldo + token; // proses penambahan token
  char key = keypad.getKey();
  if (key) isi(); // logika untuk memanggil program pengisian menggunakan keypad
  if (saldo > ss) { // logika untuk mengaktifkan buzzer ketika pengisian
    ss = saldo;
    digitalWrite(buzz, HIGH);
    delay(1000);
    digitalWrite(buzz, LOW);
    lcd.clear();
  }
  kwhcount(); // program pengurangan kwh
  buzzer (); // menjalankan logika tempo bunyi buzzer terhadap kwh
  wemos.println(saldo); // mengirim data kWh ke board D1 Mini
  serialmonitor(); // menampilkan parameter pada Serial Monitor
  lcdtampil(); // menampilkan parameter pada LCD Crystal
}
