#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleTimer.h>

SimpleTimer timer;

// =======================================================
// ======= PH SENSOR =====================================
// =======================================================
const int analog_in_pin_ph = A0; // pin the Ph sensor is connected to
float adc_resolution = 1024.0;
//-----------------------------------------
float calibration_adjustment_ph = -.08; // adjust this to calibrate
int voltage_input_ph = 4.98; // voltage can be 5 or 3.3
//-----------------------------------------
float calibration_value_ph = 21.34 + calibration_adjustment_ph;
unsigned long int average_value_ph;
int buffer_array_ph[10],temp;
float ph_value; // actual pH value to display on screen
 
float getPH()
{
 // *Taking 10 samples of the analogui Input A0, ordering them and iscarding the highest and lowest
 // *calculating the mean with teh six remaining by converting this value to voltage in the variable pHVol
 // *then using the equiation to convert pHVol to pHValue 

timer.run(); // Initiates SimpleTimer

// Use the for loop we take samples, then arrange the values, and finally take the average.

 Serial.print("Loop readings = ");
 for(int i=0;i<10;i++) 
  { 
    buffer_array_ph[i]=analogRead(analog_in_pin_ph);
    Serial.print(buffer_array_ph[i]); // print the voltage readout in the Serial Monitor
    Serial.print(" / ");
    delay(30);
  }
 for(int i=0;i<9;i++)
  {
    for(int j=i+1;j<10;j++)
      {
        if(buffer_array_ph[i]>buffer_array_ph[j])
          {
            temp=buffer_array_ph[i];
            buffer_array_ph[i]=buffer_array_ph[j];
            buffer_array_ph[j]=temp;
          }
      }
  }

 average_value_ph=0;
 for(int i=2;i<8;i++)
  {
    average_value_ph += buffer_array_ph[i];
  }
  
 float voltage_ph = (float)average_value_ph * voltage_input_ph / 1024 / 6; 
 ph_value = (-5.70 * voltage_ph) + calibration_value_ph; // Calculate the actual pH
 
 Serial.print("            ");
 Serial.print("average_value_ph = ");
 Serial.print(average_value_ph);
 Serial.print("         ");
 Serial.print("voltage_ph = ");
 Serial.print(voltage_ph);
 Serial.print("         ");
 Serial.print("calibration_value_ph = ");
 Serial.print(calibration_value_ph);
 Serial.print("         ");
 Serial.print("ph_value = ");
 Serial.println(ph_value);
  
 delay(1000); // pause between serial monitor output - can be set to zero after testing
 return ph_value;
}

// =======================================================
// ======= PPM OCEAN TDS METER SENSOR ====================
// =======================================================
#define TdsSensorPin A1
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;

int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

int getTDSReading()
{
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      //Serial.print("TDS Value:");
      //Serial.print(tdsValue,0);
      //Serial.println("ppm");
      //return tdsValue;
   }
   return tdsValue;
}


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
void printData(DeviceAddress deviceAddress) // This is probably not needed
{
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  //displayWaterTemp(deviceAddress);
  Serial.println();
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}
// --------  function to return water tempurature -------------
float getWaterTemp(DeviceAddress deviceAddress)
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
// ----------End Tempurature Sensors  --------------

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
  lcd.print("TDS:");
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
  //tempurature
  float temp = getWaterTemp(waterThermometer);
  if (temp == -10){
    lcd.print("(error)   ");
  } else {
    lcd.print(temp);
    lcd.print("(C)");
  }
  lcd.setCursor(5,1);
  int tds = getTDSReading();
  //Serial.print(tds);
  if (tds > 1000){
    lcd.print("(error)     ");
  } else {
    lcd.print(tds);
    lcd.print("(PPM)     ");
  }
  lcd.setCursor(5,2);
  float ph = getPH();
  lcd.print(ph); 
}

// ------- END LCD DISPLAY ------------------------------
// ==================================================
// ===========  MAIN SETUP ==========================
// ==================================================
void setup(void)
{
  Serial.begin(9600);// start serial port
  Serial.println("Starting Hydroponics Automation Controler");
  pinMode(TdsSensorPin,INPUT); // setup TDS sensor
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
  //Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  //Serial.print(getWaterTemp(waterThermometer));
  //Serial.print("  ---  ");
  //Serial.println("DONE");

  // get TDS 
  getTDSReading();
  //Serial.print("Requesting TDS...");
  //Serial.print(getTDSReading());
  //Serial.print("  ---  ");
  //Serial.println("DONE");

  // get PH
  getPH();
  //Serial.print("Requesting PH...");
  //Serial.print(getPH());
  //Serial.print("  ---  ");
  //Serial.println("DONE");


  displayMainscreenData();

}

// ----------------- END MAIN LOOP ------------------------