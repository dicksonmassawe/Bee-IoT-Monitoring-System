// Include Libraries
#include <mySD.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DFRobot_AHT20.h"

// ENS160+AHT2x
#include "ScioSense_ENS160.h"

// ScioSense_ENS160      ens160(ENS160_I2CADDR_0);
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);

// Temperature and Humidity
DFRobot_AHT20 aht20;

// SD File and String to hold sensor values
ext::File bee;
String ens160_val;  // Variable to store sensor values
String sep = ",";   // Separator

// Variable to store sensor values
int AQI;
int TVOC;
int eCO2;
int R_HP0;
int R_HP1;
int R_HP2;
int R_HP3;
float temp;
float humd;

// SD SPI
#define SD_CLK 14
#define SD_MISO 2
#define SD_MOSI 15
#define SD_CS 13

// Build_in LED
#define led 25

// OLED
#define SDA 21
#define SCL 22
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi credentials
const char *ssid = "EPAL-PRIVATE";
const char *password = "epal@yeesi";

// API keys for ThingSpeak
const char *api_key = "61MGSVMZY8Y52ECM";
const char *field_01 = "1";  // AQI
const char *field_02 = "2";  // TVOC
const char *field_03 = "3";  // eCO2
const char *field_04 = "4";  // Temperature
const char *field_05 = "5";  // Relative Humidity

void setup() {

  Serial.begin(115200);
  while (!Serial)
    ;

  // Connecting to WiFi
  connectToWiFi();

  // Initialize LED
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  // SD Card ChipSelect Pin Mode
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // Initializing AHT2x
  uint8_t status;
  while ((status = aht20.begin()) != 0) {
    Serial.print("AHT20 sensor initialization failed. error status : ");
    Serial.println(status);
    delay(1000);
  }

  // Initializing OLED
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(14, 0);
  display.println("SMART BEE KEEPING");
  Serial.println("Initializing SD card...");
  display.setCursor(0, 20);
  display.print("Initializing SD");
  for (int i = 0; i < 5; i++) {
    display.print(".");
    display.display();
    delay(500);
  }
  display.println(".");
  display.display();
  delay(2000);

  // Initializing SD Card
  while (!SD.begin(SD_CS, SD_MOSI, SD_MISO, SD_CLK)) {
    Serial.println("SD Initialization failed!");
    display.setCursor(26, 30);
    display.print(">> Failed <<");
    display.display();
    failAlert(3);
  }

  Serial.println("SD Initialization done.");
  display.setCursor(20, 30);
  display.print(">> Connected <<");
  display.display();
  doneAlert(3);

  //Initializing ENS160+AHT2x
  display.setCursor(0, 45);
  display.print("Initializing ENS160");
  for (int i = 0; i < 1; i++) {
    display.print(".");
    display.display();
    delay(500);
  }
  display.println(".");
  display.display();

  ens160.begin();
  while (!ens160.available()) {
    Serial.println("ENS160+AHT2x Initialization Failed");
    display.setCursor(26, 55);
    display.print(">> Failed <<");
    display.display();
    failAlert(3);
  }
  Serial.println("ENS160+AHT2x Initialized Successfully.");
  display.setCursor(20, 55);
  display.print(">> Connected <<");
  display.display();
  display.clearDisplay();
  doneAlert(3);

  // Print ENS160 versions
  if (ens160.available()) {
    Serial.print("\tRev: ");
    Serial.print(ens160.getMajorRev());
    Serial.print(".");
    Serial.print(ens160.getMinorRev());
    Serial.print(".");
    Serial.println(ens160.getBuild());

    Serial.print("\tStandard mode ");
    Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "done." : "failed!");
  }

  // Printing header file to SD Card.
  bee = SD.open("ENS160s.csv", FILE_WRITE);
  if (bee) {
    bee.println("AQI, TVOC (ppb), eCO2 (ppm), R HP0 (Ohm), R HP1 (Ohm), R HP2 (Ohm), R HP3 (Ohm), Temp (°C), Humd (%RH)");
    bee.flush();
    bee.close();
  } else {  //  file open error
    Serial.println("error opening ENS160s.csv");
    failAlert(3);
  }
}

void loop() {
  if (ens160.available()) {
    ens160.measure(true);
    ens160.measureRaw(true);

    // Storing Data to variables
    AQI = ens160.getAQI();
    TVOC = ens160.getTVOC();
    eCO2 = ens160.geteCO2();
    R_HP0 = ens160.getHP0();
    R_HP1 = ens160.getHP1();
    R_HP2 = ens160.getHP2();
    R_HP3 = ens160.getHP3();

    // Reading and Storing Temperature and Humidity
    if (aht20.startMeasurementReady(true)) {

      temp = aht20.getTemperature_C();  // Get temp in Celsius (℃), range -40-80℃
      humd = aht20.getHumidity_RH();    // // Get relative humidity (%RH), range 0-100℃

      Serial.print("Temp: ");
      Serial.print(temp);
      Serial.print(" °C, ");
      Serial.print("Humidity: ");
      Serial.print(humd);
      Serial.println(" %RH");
    }

    // Printing Sensor value on Serial Monitor
    Serial.print("AQI: ");
    Serial.print(AQI);
    Serial.print("\t");
    Serial.print("TVOC: ");
    Serial.print(TVOC);
    Serial.print("ppb\t");
    Serial.print("eCO2: ");
    Serial.print(eCO2);
    Serial.print("ppm\t");
    Serial.print("R HP0: ");
    Serial.print(R_HP0);
    Serial.print("Ohm\t");
    Serial.print("R HP1: ");
    Serial.print(R_HP1);
    Serial.print("Ohm\t");
    Serial.print("R HP2: ");
    Serial.print(R_HP2);
    Serial.print("Ohm\t");
    Serial.print("R HP3: ");
    Serial.print(R_HP3);
    Serial.println("Ohm");

    // Storing sensor value to a variable
    ens160_val = String(AQI) + sep + String(TVOC) + sep + String(eCO2) + sep + String(R_HP0) + sep + String(R_HP1) + sep + String(R_HP2) + sep + String(R_HP3) + sep + String(temp) + sep + String(humd);

    bee = SD.open("ENS160s.csv", FILE_WRITE);
    if (bee) {
      bee.println(ens160_val);
      bee.flush();
      bee.close();
    } else {  //  file open error
      Serial.println("error opening ENS160s.csv");
      failAlert(3);
    }
  }

  // Send data to Cloud
  sendToThingSpeak(AQI, TVOC, eCO2, temp, humd);

  delay(20000);
}

void doneAlert(int freq) {
  for (int i = 0; i < freq; i++) {
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    delay(500);
  }
}

void failAlert(int freq) {
  for (int i = 0; i < freq; i++) {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void sendToThingSpeak(float value1, float value2, float value3, float value4, float value5) {
  String url = "https://api.thingspeak.com/update?api_key=";
  url += api_key;

  url += "&field";
  url += field_01;
  url += "=";
  url += String(value1);

  url += "&field";
  url += field_02;
  url += "=";
  url += String(value2);

  url += "&field";
  url += field_03;
  url += "=";
  url += String(value3);

  url += "&field";
  url += field_04;
  url += "=";
  url += String(value4);

  url += "&field";
  url += field_05;
  url += "=";
  url += String(value5);

  Serial.println(url);

  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();
  String payload = http.getString();

  Serial.println(httpCode);
  Serial.println(payload);

  http.end();
}