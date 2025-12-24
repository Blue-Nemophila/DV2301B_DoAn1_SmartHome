// Library
#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"
#include <MFRC522.h>
#include <ESP32Servo.h>
#include "web.h"

// ---------- USER CONFIG ----------
struct WifiInfo {
  const char* ssid;
  const char* password;
};

WifiInfo wifiList[] = {
  {"hihuhi", "ahihihia"},
  {"nguyenlethaitai", "14102005"}
};

const int wifiCount = sizeof(wifiList) / sizeof(wifiList[0]);

#define PIN_LED        27
#define PIN_BUTTON_LED 32
#define PIN_LED2        25
#define PIN_BUTTON_LED2 14
#define PIN_FAN_PWM    22   // output to transistor (PWM)
#define PIN_BUTTON_FAN 2   // input button for Fan (requires external pull-up to 3.3V)
#define PIN_DHT        15
#define PIN_FAN 22
#define DOOR_CLOSE_ANGLE  0
#define DOOR_OPEN_ANGLE   100

const int DHTTYPE = DHT11;
const float TEMP_THRESHOLD = 33.5; // ngưỡng bật quạt khi auto (°C)
const unsigned long STATUS_INTERVAL_MS = 1000; // web refresh interval (client side)

// PWM settings
//const int PWM_CHANNEL = 0;
//const int PWM_FREQ = 1000;
//const int PWM_RESOLUTION = 8; // 0-255

// ---------- GLOBAL STATE ----------
bool ledState = false;
bool fanState = false;
bool led2State = false;
bool lastButtonLed2 = HIGH;
unsigned long lastLed2Press = 0;
bool autoMode = true;     // nếu true thì fan auto theo nhiệt độ, else manual
float lastTemp = 0.0;
float lastHum = 0.0;

// ---------- RFID + SERVO ----------
#define SS_PIN   5
#define RST_PIN  4
#define SERVO_PIN 26

MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;

bool doorOpen = false;
unsigned long doorMillis = 0;
const unsigned long DOOR_OPEN_TIME = 5000; // mở cửa 5s
String lastUID = "---";

// 
DHT dht(PIN_DHT, DHTTYPE);
WebServer server(80);


void setLed(bool on) {//---------------------------------------------------------------------------------------------------------Set LED
  ledState = on;
  digitalWrite(PIN_LED, on ? HIGH : LOW);
}


void setLed2(bool on) {//--------------------------------------------------------------------------------------------------------Set LED2
  led2State = on;
  digitalWrite(PIN_LED2, on ? HIGH : LOW);
}


void handleStatus() {//-----------------------------------------------------------------------------------------------------Handle Status
  String s = "{";
  s += "\"rfid\":\"" + lastUID + "\",";
  s += "\"led\":" + String(ledState ? "true" : "false") + ",";
  s += "\"led2\":" + String(led2State ? "true" : "false") + ",";
  s += "\"fan\":" + String(fanState ? "true" : "false") + ",";
  s += "\"auto\":" + String(autoMode ? "true" : "false") + ",";
  s += "\"door\":" + String(doorOpen ? "true" : "false") + ",";
  s += "\"temp\":" + String(lastTemp, 2) + ",";
  s += "\"hum\":" + String(lastHum, 2);
  s += "}";
  server.send(200, "application/json", s);
}


void connectWifi() {//-----------------------------------------------------------------------------------------------------Connect WiFi
  Serial.println("Dang thu ket noi WiFi...");
  for (int i = 0; i < wifiCount; i++) {
    Serial.print("Thu ket noi mang: ");
    Serial.println(wifiList[i].ssid);
    WiFi.begin(wifiList[i].ssid, wifiList[i].password);

    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 30) { // 30 x 200ms = 6s
      delay(200);
      Serial.print(".");
      attempt++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nDa ket noi WiFi thanh cong!");
      Serial.print("IP cua ESP32: ");
      Serial.println(WiFi.localIP());
      return;     // → Thoát hàm vì đã thành công
    }
    Serial.println("\nThat bai, thu mang tiep theo...");
  }
  Serial.println("Khong ket noi duoc bat ky WiFi nao!");
}


void setFan(bool on) {//--------------------------------------------------------------------------------------------------------Set Fan
  fanState = on;
  digitalWrite(PIN_FAN, on ? HIGH : LOW);
}


void handleFanToggle() {//---------------------------------------------------------------------------------------------Handle Fan Toggle
  autoMode = false;          // bấm tay thì auto cút
  setFan(!fanState);         // đảo trạng thái quạt
  server.send(200, "text/plain", "OK");
}


void handleAutoToggle() {//-------------------------------------------------------------------------------------------Handle Auto Toggle
  autoMode = !autoMode;
  if (autoMode) {
    // bật auto → xét nhiệt độ ngay
    setFan(lastTemp > TEMP_THRESHOLD);
  }
  server.send(200, "text/plain", "OK");
}


void handleLedToggle() {//--------------------------------------------------------------------------------------------Handle LED Toggle
  setLed(!ledState);
  server.send(200, "text/plain", "OK");
}


void handleLed2Toggle() {//------------------------------------------------------------------------------------------Handle LED2 Toggle
  setLed2(!led2State);
  server.send(200, "text/plain", "OK");
}


bool lastButtonLed = HIGH;
unsigned long lastLedPress = 0;


void checkButtonLed() {//-------------------------------------------------------------------------------------------Check Button LED
  bool current = digitalRead(PIN_BUTTON_LED);
  if (lastButtonLed == HIGH && current == LOW) {
    if (millis() - lastLedPress > 200) {
      setLed(!ledState);
      lastLedPress = millis();
      Serial.println("Nut LED duoc bam");
    }
  }
  lastButtonLed = current;
}


void checkButtonLed2() {//------------------------------------------------------------------------------------------Check Button LED2
  bool current = digitalRead(PIN_BUTTON_LED2);
  if (lastButtonLed2 == HIGH && current == LOW) {
    if (millis() - lastLed2Press > 200) {
      setLed2(!led2State);
      lastLed2Press = millis();
      Serial.println("Nut LED 2 duoc bam");
    }
  }
  lastButtonLed2 = current;
}


bool lastButtonFan = HIGH;
unsigned long lastFanPress = 0;


void checkButtonFan() {//-----------------------------------------------------------------------------------------Check Button Fan
  bool current = digitalRead(PIN_BUTTON_FAN);
  // phát hiện cạnh xuống (nhấn)
  if (lastButtonFan == HIGH && current == LOW) {
    if (millis() - lastFanPress > 200) { // debounce 200ms
      autoMode = false;              // bấm tay là tắt auto
      setFan(!fanState);             // đảo trạng thái quạt
      lastFanPress = millis();
      Serial.println("Nut quat duoc bam");
    }
  }
  lastButtonFan = current;
}


void openDoor() {//----------------------------------------------------------------------------------------------Open Door
  doorServo.attach(SERVO_PIN, 500, 2400);
  doorServo.write(DOOR_OPEN_ANGLE);
  doorOpen = true;
  doorMillis = millis();
  Serial.println("CUA DA MO");
}

void closeDoor() {//---------------------------------------------------------------------------------------------Close Door
  doorServo.attach(SERVO_PIN, 500, 2400);
  doorServo.write(DOOR_CLOSE_ANGLE);
  delay(300);
  doorServo.detach();
  doorOpen = false;
  Serial.println("CUA DA DONG");
}


void handleDoorOpen() {//---------------------------------------------------------------------------------------Handle Door Open
  openDoor();
  server.send(200, "text/plain", "DOOR OPENED");
}


byte validUID[4] = {0x00, 0x4E, 0x5A, 0x60 };


void checkRFID() {//-------------------------------------------------------------------------------------------Check RFID
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;
  bool match = true;

  Serial.print("UID (HEX): ");

  lastUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) lastUID += "0";
    lastUID += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) lastUID += " ";
  }
  lastUID.toUpperCase();


  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != validUID[i]) {
      match = false;
      break;
    }
  }
  if (match) {
    Serial.println("THE HOP LE → MO CUA");

    openDoor();
  } else {
    Serial.println("THE KHONG HOP LE");
  }
  Serial.println();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}


void setup() {//----------------------------------------------------------------------------------------------------------Setup
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_BUTTON_LED, INPUT_PULLUP);
  pinMode(PIN_BUTTON_LED2, INPUT_PULLUP);
  pinMode(PIN_BUTTON_FAN, INPUT_PULLUP);
  pinMode(PIN_FAN, OUTPUT);
  digitalWrite(PIN_FAN, LOW);


  dht.begin();
  WiFi.mode(WIFI_STA);
  connectWifi();
  setupWeb();
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/led/toggle", HTTP_POST, handleLedToggle);
  server.on("/fan/toggle", HTTP_POST, handleFanToggle);
  server.on("/auto/toggle", HTTP_POST, handleAutoToggle);
  server.on("/led2/toggle", HTTP_POST, handleLed2Toggle);
  server.on("/door/open", HTTP_POST, handleDoorOpen);


  server.begin();
  Serial.println("Web server started");


  lastTemp = dht.readTemperature();
  Serial.println("Initial temperature: " + String(lastTemp) + " °C");
  lastHum = dht.readHumidity();
  setLed(false);
  setLed2(false);
  setFan(false);


  SPI.begin(18, 19, 23, SS_PIN);
  rfid.PCD_Init(SS_PIN, RST_PIN);
  doorServo.setPeriodHertz(50);
  //doorServo.attach(SERVO_PIN, 500, 2400);
  doorServo.setPeriodHertz(50);
  closeDoor();

  Serial.println("RFID + SERVO READY");
}


unsigned long lastDhtMillis = 0;
const unsigned long DHT_INTERVAL = 2000; // đọc DHT mỗi 2s
unsigned long lastRfidMillis = 0;
const unsigned long RFID_INTERVAL = 200;


void loop() {//---------------------------------------------------------------------------------------------------------Loop
  server.handleClient();
  checkButtonLed();
  checkButtonLed2();
  checkButtonFan();

  if (millis() - lastDhtMillis > DHT_INTERVAL) {
    lastDhtMillis = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) lastTemp = t;
    if (!isnan(h)) lastHum = h;
    if (autoMode) {
      setFan(lastTemp > TEMP_THRESHOLD);
    }
  }
  if (millis() - lastRfidMillis > RFID_INTERVAL) {
    lastRfidMillis = millis();
    checkRFID();
  }
  // auto đóng cửa sau 5s
  if (doorOpen && millis() - doorMillis > DOOR_OPEN_TIME) {
    closeDoor();
  }
  delay(1);
}