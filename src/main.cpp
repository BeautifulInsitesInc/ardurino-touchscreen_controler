#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// ------ TEMPURATURE SENSOR DS18B20 -----------------------

#define ONE_WIRE_BUS 2 // Port the data wire is plugged into on the Arduino
#define TEMPERATURE_PRECISION 9

OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.

DeviceAddress waterThermometer, airThermometer; // arrays to hold device addresses

//  ----------- END TEMPURATURE SENSOR ---------------------
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Starting Hydroponics Automation Controler");

  // ----- SETUP - THERMOMETER  -------
  // Start up the library
  sensors.begin();
  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // Search for devices on the bus and assign based on an index. 
  if (!sensors.getAddress(waterThermometer, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(airThermometer, 1)) Serial.println("Unable to find address for Device 1");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  Serial.println();
  Serial.print("Device 1 Address: ");
  Serial.println();

  // set the resolution to per device
  sensors.setResolution(waterThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(airThermometer, TEMPERATURE_PRECISION);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(waterThermometer), DEC);
  Serial.println();

  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(airThermometer), DEC);
  Serial.println();
  // ------------ END SETUP - THERMOMETER ------------

}
// ------- FUNCTIONS - THERMOMETER ----------------
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}
// ------- END FUNCTIONS THERMOMETER -----------------------



void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");

  // print the device information
  Serial.print("Water Tempurature :");
  printData(waterThermometer);
  printData(airThermometer);
}