#include <Arduino.h>
#include <OneWire.h> 
#include <DallasTemperature.h> 

OneWire ds18x20[] = { 1, 2, 3, 4 };
const int oneWireCount = sizeof(ds18x20) / sizeof(OneWire);
DallasTemperature sensor[oneWireCount];
float temp[4];

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

  delay(1000);
  Serial.println();
}

