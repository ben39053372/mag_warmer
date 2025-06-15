#include <SPI.h>
#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h> 
#include <DallasTemperature.h> 
#include <Adafruit_SH110X.h>
#include <Adafruit_GFX.h>



// create and config a oled screen 
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels
#define OLED_RESET -1     // can set an oled reset pin if desired
Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);

// Define Temp DS18B20 pin
OneWire ds18x20[] = {2,3,4,5 };
const int oneWireCount = sizeof(ds18x20) / sizeof(OneWire);
DallasTemperature sensor[oneWireCount];

// Define IRF540N pin to control MCH
int MCH1_pin = 20;
int MCH2_pin = 21;
bool is_MCH_1_OPEN = false;
bool is_MCH_2_OPEN = false;

int targetTemp = 40;

void updateTemp();
void displayOled();
void controlHeater();
void startDs18b20();

void setup() {
  // start serial port
  Serial.begin(9600);
  delay(250);
  display.begin(0x3c, true);

  startDs18b20();

  // config MCH pin
  pinMode(10, OUTPUT);
  pinMode(20, OUTPUT);
}

void loop() {
  updateTemp();
  controlHeater();
  displayOled();
  delay(1000);
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
  display.println();
  display.drawLine(0, display.getCursorY(), display.width() -1, display.getCursorY(), SH110X_WHITE);
  display.println();

  for(int i = 0; i< oneWireCount; i++) {
    display.printf("Temp %d: %f %cC \n", i, sensor[i].getTempCByIndex(0), 176);
  }
  display.display();
}

void updateTemp() {
  for (int i = 0; i < oneWireCount; i++) {
    Serial.println("Requesting temperatures...");
    sensor[i].requestTemperatures();
  }
}

void controlHeater() {
  if(sensor[0].getTempCByIndex(0)< targetTemp || sensor[1].getTempCByIndex(0)< targetTemp ) {
    Serial.println("MCH 1 on!");
    is_MCH_1_OPEN = true;
    digitalWrite(MCH1_pin, HIGH);
  } else {
    Serial.println("MCH 1 off!");
    is_MCH_1_OPEN = false;
    digitalWrite(MCH1_pin, LOW);
  }
  if(sensor[2].getTempCByIndex(0)< targetTemp || sensor[3].getTempCByIndex(0) < targetTemp ) {
    Serial.println("MCH 2 on!");
    is_MCH_2_OPEN = true;
    digitalWrite(MCH2_pin, HIGH);
  } else {
    Serial.println("MCH 2 off!");
    is_MCH_2_OPEN = false;
    digitalWrite(MCH2_pin, LOW);
  }
}
// #include <OneWire.h> 
// #include <DallasTemperature.h> 

// #define DQ_Pin 1

// OneWire oneWire(DQ_Pin);
// DallasTemperature sensors(&oneWire);

// void setup(void)
// {
//   Serial.begin(9600);
//   sensors.begin();
// }

// void loop(void)
// {
//   Serial.print("Temperatures --> ");
//   sensors.requestTemperatures();
//   Serial.println(sensors.getTempCByIndex(0));
//   delay(2000);
// }

// void setup() {
//   // initialize digital pin 13 as an output.
//   pinMode(20, OUTPUT);
//   }
//   // the loop function runs over and over again forever
//   void loop() {
//   digitalWrite(20, HIGH); // turn the LED on (HIGH is the voltage level)
//   // delay(1000); // wait for a second
//   // digitalWrite(13, LOW); // turn the LED off by making the voltage LOW
//   // delay(1000); // wait for a second
//   }
