#include <SPI.h>
#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h> 
#include <DallasTemperature.h> 
#include <Adafruit_SH110X.h>
#include <Adafruit_GFX.h>
#include <string> 

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <nlohmann/json.hpp>


#define SERVICE_UUID        "2aae64b6-8f24-4643-9302-0ba146f8d9f2"
#define CHARACTERISTIC_UUID "c94b7467-2490-46a2-b5e7-6a16752e13d3"

using json = nlohmann::json;


// create and config a oled screen 
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels
#define OLED_RESET -1     // can set an oled reset pin if desired
Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);

// Define Temp DS18B20 pin
OneWire ds18x20[] = {1, 2, 3, 4, 10, 20 };
const int oneWireCount = sizeof(ds18x20) / sizeof(OneWire);
DallasTemperature sensor[oneWireCount];

BLECharacteristic *pCharacteristic;

// define adc pin 
#define ADC A0

bool on = true;
int targetTemp = 45;

bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Connected");
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      Serial.println("Disconnected");
      deviceConnected = false;
      pServer->startAdvertising();
    }

};
class MyCallbacks: public BLECharacteristicCallbacks {

  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0) {
      int cmd = std::stoi(value);
      if(cmd == -1) {
        on = false;
      } else {
        on = true;
        targetTemp = cmd;
      }
    }
  }
};

// Define IRF540N pin to control MCH
int MCH1_pin = 7;
int MCH2_pin = 6;
int MCH3_pin = 5;
bool is_MCH_1_OPEN = false;
bool is_MCH_2_OPEN = false;
bool is_MCH_3_OPEN = false;

void fetchTemp();
void displayOled();
void controlHeater();
void startDs18b20();
int getBatteryPower();
void initBLE();
void sendData();
std::string getData();
float getPowerVoltage ();
int voltageToPercent(float voltage);

void setup() {
  // start serial port
  Serial.begin(9600);
  delay(250);
  display.begin(0x3c, true);
  startDs18b20();
  // config MCH pin
  pinMode(MCH1_pin, OUTPUT);
  pinMode(MCH2_pin, OUTPUT);
  // ADC
  pinMode(ADC, INPUT);

  initBLE();

}

void loop() {
  fetchTemp();
  controlHeater();
  displayOled();
  sendData();
  getBatteryPower();
  delay(1000);
}

std::string getData() {
  json j;
  j["power"] = on;
  j["heater"] = {is_MCH_1_OPEN, is_MCH_2_OPEN};
  j["targetTemp"] = targetTemp;
  j["voltage"] = getPowerVoltage();

    // Add temperature readings
  int temp[oneWireCount];
  for(int i = 0; i < oneWireCount; i++) {
    temp[i] = (int)(sensor[i].getTempCByIndex(0));
  }
  j["temp"] = temp;

  return j.dump();
}

void sendData() {
  if(deviceConnected) {
    std::string data = getData();
    pCharacteristic->setValue(data);
    pCharacteristic->notify();
  }
}

void initBLE() {
  BLEDevice::init("Mag Warmer 1");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks);

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE | 
                                         BLECharacteristic::PROPERTY_BROADCAST |
                                         BLECharacteristic::PROPERTY_NOTIFY 
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

float getPowerVoltage () {
  uint32_t voltageSum = 0;
  const int sampleCount = 16;
  for(int i=0;i< sampleCount;i++){
    voltageSum += analogReadMilliVolts(ADC);
    delay(10);
  }
  float voltageAvg = voltageSum / (float)sampleCount;
  float batteryVoltage = voltageAvg * 2 / 1000.0;
  return batteryVoltage;
}

int getBatteryPower() {
  uint32_t voltageSum = 0;
  const int sampleCount = 16;
  for(int i=0;i< sampleCount;i++){
    voltageSum += analogReadMilliVolts(ADC);
    delay(10);
  }
  float voltageAvg = voltageSum / (float)sampleCount;
  float batteryVoltage = voltageAvg * 2 / 1000.0;
  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage , 3);
  Serial.println(" V");
  int percent = voltageToPercent(batteryVoltage);
  Serial.print("Battery Power percent: ");
  Serial.print(percent);
  Serial.println("%");
  return percent;
}

void startDs18b20() {
    // Start up the library on all defined bus-wires
    DeviceAddress deviceAddress;
    for (int i = 0; i < oneWireCount; i++) {
      Serial.print("oneWireCount");
      Serial.println(oneWireCount);
      sensor[i].setOneWire(&ds18x20[i]);
      sensor[i].begin();
      sensor[i].setResolution(deviceAddress, 12);
    }
}

void displayOled() {
  Serial.println("display");
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0 ,0);
  display.setTextColor(SH110X_WHITE);
  if(on) {
    display.println("Power: On!");
  } else {
    display.println("Power: Off!");
  }
  display.println();
  display.drawLine(0, display.getCursorY(), display.width() -1, display.getCursorY(), SH110X_WHITE);
  display.println();
  if(is_MCH_1_OPEN) {
    display.println("Heater 1 On!");
  } else {
    display.println("Heater 1 Off!");
  }

  if(is_MCH_2_OPEN) {
    display.println("Heater 2 On!");
  } else {
    display.println("Heater 2 Off!");
  }
  if(is_MCH_3_OPEN) {
    display.println("Heater 3 On!");
  } else {
    display.println("Heater 3 Off!");
  }
  display.println();
  display.drawLine(0, display.getCursorY(), display.width() -1, display.getCursorY(), SH110X_WHITE);
  display.println();

  for(int i = 0; i< oneWireCount; i++) {
    float temp = sensor[i].getTempCByIndex(0);
    if(temp > 0) {
      display.printf("Temp %d: %d %cC \n", i, (int)(sensor[i].getTempCByIndex(0)), 247);
    }
  }

  display.println();
  display.drawLine(0, display.getCursorY(), display.width() -1, display.getCursorY(), SH110X_WHITE);
  display.println();

  display.printf("Target Temp: %d %cC", targetTemp, 247);

  display.display();
}

void fetchTemp() {
  for (int i = 0; i < oneWireCount; i++) {
    Serial.println("Requesting temperatures...");
    sensor[i].requestTemperatures();
  }
}

void controlHeater() {
  if(!on) {
    is_MCH_1_OPEN = false;
    is_MCH_2_OPEN = false;
    is_MCH_3_OPEN = false;
    digitalWrite(MCH1_pin, LOW);
    digitalWrite(MCH2_pin, LOW);
    digitalWrite(MCH3_pin, LOW);
    return;
  }

  for(int i = 0; i < oneWireCount; i++) {
    Serial.print("temp ");
    Serial.print(i);
    Serial.print(" :");
    Serial.println(sensor[i].getTempCByIndex(0));
  }

  if(sensor[0].getTempCByIndex(0) >= targetTemp || sensor[1].getTempCByIndex(0) >= targetTemp ) {
    Serial.println("MCH 1 off!");
    is_MCH_1_OPEN = false;
    digitalWrite(MCH1_pin, LOW);
  } else {
    Serial.println("MCH 1 on!");
    is_MCH_1_OPEN = true;
    digitalWrite(MCH1_pin, HIGH);
  }
  if(sensor[2].getTempCByIndex(0) >= targetTemp || sensor[3].getTempCByIndex(0) >= targetTemp ) {
    Serial.println("MCH 2 off!");
    is_MCH_2_OPEN = false;
    digitalWrite(MCH2_pin, LOW);
  } else {
    Serial.println("MCH 2 on!");
    is_MCH_2_OPEN = true;
    digitalWrite(MCH2_pin, HIGH);
  }
  if(sensor[4].getTempCByIndex(0) >= (targetTemp -3) || sensor[5].getTempCByIndex(0) >= (targetTemp -3) ) {
    Serial.println("MCH 3 off!");
    is_MCH_3_OPEN = false;
    digitalWrite(MCH3_pin, LOW);
  } else {
    Serial.println("MCH 3 on!");
    is_MCH_3_OPEN = true;
    digitalWrite(MCH3_pin, HIGH);
  }
}

int voltageToPercent(float voltage) {
  if (voltage >= 4.20) return 100;
  else if (voltage >= 4.04) return map(voltage * 100, 404, 420, 80, 100);
  else if (voltage >= 3.85) return map(voltage * 100, 385, 404, 50, 80);
  else if (voltage >= 3.70) return map(voltage * 100, 370, 385, 20, 50);
  else if (voltage >= 3.50) return map(voltage * 100, 350, 370, 5, 20);
  else return 0;
}