#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

// =================================================
// ========== LCD DISPLAY ==========================
// =================================================
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
// Display the splash screen
void displaySplashscreen()
{
  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("CONCIERGE GROWERS");
  lcd.setCursor(5,1);
  lcd.print("Eat Good");
  lcd.setCursor(4,2);
  lcd.print("Feel Good");
  lcd.setCursor(4,3);
  lcd.print("Look Good");
  delay(1000);
  lcd.clear();
}

// Display the parts that don't change
void displayMainscreenstatic()
{
  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.setCursor(1,1);
  lcd.print("PPM:");
  lcd.setCursor(2,2);
  lcd.print("PH:");
  lcd.setCursor(0,3);
  lcd.print("Pump:");
  lcd.setCursor(14,3);
  lcd.print("(Time)");
}

void displayMainscreenData() // Display the data that changes on main screen
{
  lcd.setCursor(5,0);
  // put if statement here to print c or f depending on settings
  lcd.print(displayWaterTemp(waterThermometer));
  lcd.print("C");
  
}

// ------- END LCD DISPLAY ------------------------------

// =======================================================
// ======= TEMPURATURE SENSOR DS18B20 ====================
// =======================================================
#define ONE_WIRE_BUS 2 // Port the data wire is plugged into on the Arduino
#define TEMPERATURE_PRECISION 11
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
DeviceAddress waterThermometer, airThermometer; // arrays to hold device addresses

void setupThermometers()
{
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
}

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

// function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}
// function to return water tempurature
float displayWaterTemp(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    tempC = -10;
  }
  return tempC;
  //Serial.print(DallasTemperature::toFahrenheit(tempC)); - to convert
}

// ----------    END THERMOMETER  ------------

// ==================================================
// ===========  MAIN SETUP ==========================
// ==================================================
void setup(void)
{
  Serial.begin(9600);// start serial port
  Serial.println("Starting Hydroponics Automation Controler");
  displaySplashscreen();
  displayMainscreenstatic();
  setupThermometers();
}
// ------- END MAIN SETUP ------------------

// ====================================================
// ===========  MAIN LOOP =============================
// ====================================================

void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");

  displayMainscreenData();

}

// ----------------- END MAIN LOOP ------------------------