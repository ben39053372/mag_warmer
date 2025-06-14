#include <Arduino.h>
#include <OneWire.h> 
#include <DallasTemperature.h> 

// Define Temp DS18B20 pin
OneWire ds18x20[] = { 1, 2, 3, 4 };
const int oneWireCount = sizeof(ds18x20) / sizeof(OneWire);
DallasTemperature sensor[oneWireCount];

// Define IRF540N pin to control MCH
uint MCH1_pin = 10;
uint MCH2_pin = 20;

int targetTemp = 40;

void setup() {
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature Multiple Bus Control Library Simple Demo");
  Serial.print("============Ready with ");
  Serial.print(oneWireCount);
  Serial.println(" Temp Sensors================");

  // Start up the library on all defined bus-wires
  DeviceAddress deviceAddress;
  for (int i = 0; i < oneWireCount; i++) {
    sensor[i].setOneWire(&ds18x20[i]);
    sensor[i].begin();
    if (sensor[i].getAddress(deviceAddress, 0)) sensor[i].setResolution(deviceAddress, 12);
  }

  // config MCH pin
  pinMode(MCH1_pin, OUTPUT);
  pinMode(MCH2_pin, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  for (int i = 0; i < oneWireCount; i++) {
    sensor[i].requestTemperatures();
    Serial.printf("Temp sensor %d: %f ", i, sensor[i].getTempCByIndex(0));
  }

  checkTemp();

  delay(1000);
  Serial.println();
}

void checkTemp() {
  if((sensor[0].getTempCByIndex(0) + sensor[1].getTempCByIndex(0)) < targetTemp ) {
    Serial.println("MCH1 on!");
    digitalWrite(MCH1_pin, HIGH);
  } else {
    Serial.println("MCH1 off!");
    digitalWrite(MCH1_pin, LOW);
  }

  if((sensor[2].getTempCByIndex(0) + sensor[3].getTempCByIndex(0)) < targetTemp ) {
    Serial.println("MCH2 on!");
    digitalWrite(MCH2_pin, HIGH);
  } else {
    Serial.println("MCH2 off!");
    digitalWrite(MCH2_pin, LOW);
  }
}

