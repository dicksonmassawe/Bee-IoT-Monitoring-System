// Include Libraries
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "Credentials.h"
#include "DFRobot_AHT20.h"
#include "ScioSense_ENS160.h"

// ScioSense_ENS160      ens160(ENS160_I2CADDR_0);
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);

// Temperature and Humidity
DFRobot_AHT20 aht20;

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

// Build_in LED
#define led 13

void setup() {

  Serial.begin(115200);
  while (!Serial)
    ;

  // Connecting to WiFi
  connectToWiFi();

  // Initialize LED
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  // Initializing AHT2x
  uint8_t status;
  while ((status = aht20.begin()) != 0) {
    Serial.print("AHT20 sensor initialization failed. error status : ");
    Serial.println(status);
    delay(1000);
  }

  ens160.begin();
  while (!ens160.available()) {
    Serial.println("ENS160+AHT2x Initialization Failed");
    failAlert(3);
  }
  Serial.println("ENS160+AHT2x Initialized Successfully.");
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
      Serial.print(" °C,\t");
      Serial.print("Humidity: ");
      Serial.print(humd);
      Serial.print(" %RH\t");
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
  }

  // Send data to Cloud
  sendToThingSpeak(AQI, TVOC, eCO2, temp, humd);

  // Interval of Data Collection
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
  WiFi.begin(mySSID, myPASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void sendToThingSpeak(float value1, float value2, float value3, float value4, float value5) {
  String url = "https://api.thingspeak.com/update?thingspeak_write_api_key=";
  url += thingspeak_write_api_key;

  url += "&field1";
  url += "=";
  url += String(value1);

  url += "&field2";
  url += "=";
  url += String(value2);

  url += "&field3";
  url += "=";
  url += String(value3);

  url += "&field4";
  url += "=";
  url += String(value4);

  url += "&field5";
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