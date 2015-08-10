/*
  Arduino Pressure sensor based on BMP085
  Requires Lib BMP085 V2
      https://github.com/adafruit/Adafruit_BMP085_Unified
      https://github.com/adafruit/Adafruit_Sensor
  
 Contribution: epierre

 G      GND
 V      VCC 5V/3.3V
 SCL    A4
 SDA    A5

*/

#include <MySensor.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

//float seaLevelPressure = 1013;
float myAltitude = 45;

#define LTC4067_CHRG_PIN	A1		//analog input A1 on ATmega 328 is /CHRG signal from LTC4067
#define batteryVoltage_PIN	A0		//analog input A0 on ATmega328 is battery voltage ( /2)
#define solarVoltage_PIN	A2		//analog input A2 is solar cell voltage (/ 2)
#define solarCurrent_PIN	A6		//analog input A6 is input current ( I=V/Rclprog x 1000 )
#define batteryChargeCurrent_PIN	A7	//analog input A7 is battery charge current ( I=V/Rprog x 1000 )
#define LTC4067_SUSPEND_PIN	9		//digital output D9 - drive it high to put LTC4067 in SUSPEND mode

const float VccMin        = 1.0 * 3.5; // Minimum expected Vcc level, in Volts. Example for 1 rechargeable lithium-ion.
const float VccMax        = 1.0 * 4.2; // Maximum expected Vcc level, in Volts.

#define CHILD_ID_PRESSURE 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_FORECAST 2
#define BATT_CHILD_ID 10
#define SOLAR_CHILD_ID 11
#define PRESSURE_SENSOR_ANALOG_PIN 0

// PIN Radio
#define RADIO_CE_PIN    7       // radio chip enable
#define RADIO_SS_PIN    8      // CS SS serial select

//float lastBattVoltage;
//float lastBattCurrent;
//float lastSolarVoltage;
//float lastSolarCurrent;
int lastBattPct = 0;

float VccReference = 3.3 ;				// voltage reference for measurement, definitive init in setup
unsigned long SLEEP_TIME = 10; // Sleep time between reads (in seconds)

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);      // Digital Pressure Sensor
MySensor  gw(RADIO_CE_PIN, RADIO_SS_PIN);
MyMessage batteryVoltageMsg(BATT_CHILD_ID, V_VOLTAGE);		// Battery voltage (V)
MyMessage batteryCurrentMsg(BATT_CHILD_ID, V_CURRENT);		// Battery current (A)
MyMessage solarVoltageMsg(SOLAR_CHILD_ID, V_VOLTAGE);		// Solar voltage (V)
MyMessage solarCurrentMsg(SOLAR_CHILD_ID, V_CURRENT);
MyMessage pressureMsg(CHILD_ID_PRESSURE, V_PRESSURE);
MyMessage tempMsg(CHILD_ID_TEMP, V_TEMP);
MyMessage forecastMsg(CHILD_ID_FORECAST, V_FORECAST);

float lastPressure = -1;
float lastTemp = -1;
int lastForecast = -1;
char *weather[] = {"stable", "sunny", "cloudy", "unstable", "thunderstorm", "unknown"};
int minutes;
float pressureSamples[180];
int minuteCount = 0;
bool firstRound = true;
float pressureAvg[7];
float dP_dt;
boolean metric;

void setup() {
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Pressure Sensor", "1.0");

  gw.present(BATT_CHILD_ID, S_POWER);						// Battery parameters
  gw.present(SOLAR_CHILD_ID, S_POWER);					// Solar parameters

  // use VCC (3.3V) reference
  analogReference(DEFAULT);								// default external reference = 3.3v for Ceech board
  VccReference = 3.323 ;									// measured Vcc input (on board LDO)
  pinMode(LTC4067_SUSPEND_PIN, OUTPUT);					// suspend of Lion charger set
  digitalWrite(LTC4067_SUSPEND_PIN, LOW);

  if (!bmp.begin(BMP085_MODE_ULTRAHIGHRES)) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) { }
  }

  // Register sensors to gw (they will be created as child devices)

  gw.present(CHILD_ID_PRESSURE, S_BARO);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  metric = gw.getConfig().isMetric;

}


void loop() {

  sendVoltage();
  
  sensors_event_t event;
  bmp.getEvent(&event);

  float pressure_raw;
  float pressure;
  float temperature;
  float altitude;
  
  if (event.pressure) {
      pressure_raw = event.pressure;
      //BMP085 pressure from adafruit gives you absolute pressure, that is NOT the barometric pressure the meteo forecast shows. You have to recalculate it by the above equation into the p0.
      pressure = pressure_raw/pow((1.0 - ( myAltitude/44330.0 )), 5.255);
      
      bmp.getTemperature(&temperature);
      
//      altitude = bmp.pressureToAltitude(seaLevelPressure,
//                                        event.pressure
//                                        );
                                        
  }

  if (!metric) {
    // Convert to fahrenheit
    temperature = temperature * 9.0 / 5.0 + 32.0;
  }

  int forecast = sample(pressure);

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(metric ? " *C" : " *F");
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" hPa");
  Serial.println(weather[forecast]);
  //Serial.print("Altitude = ");
  //Serial.print(altitude);


  if (ceil(temperature) != ceil(lastTemp)) {
    gw.send(tempMsg.set(temperature, 1));
    lastTemp = temperature;
  }

  if (ceil(pressure) != ceil(lastPressure)) {
    gw.send(pressureMsg.set(pressure, 1));
    lastPressure = pressure;
  }

  if (forecast != lastForecast) {
    gw.send(forecastMsg.set(weather[forecast]));
    lastForecast = forecast;
  }

  /*
   DP/Dt explanation

   0 = "Stable Weather Pattern"
   1 = "Slowly rising Good Weather", "Clear/Sunny "
   2 = "Slowly falling L-Pressure ", "Cloudy/Rain "
   3 = "Quickly rising H-Press",     "Not Stable"
   4 = "Quickly falling L-Press",    "Thunderstorm"
   5 = "Unknown (More Time needed)
  */

  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  delay(1000); //delay to allow serial to fully print before sleep

  gw.sleep(SLEEP_TIME * 1000); // sleep to conserve power
}

int sample(float pressure) {
  // Algorithm found here
  // http://www.freescale.com/files/sensors/doc/app_note/AN3914.pdf
  if (minuteCount > 180)
    minuteCount = 6;

  pressureSamples[minuteCount] = pressure;
  minuteCount++;

  if (minuteCount == 5) {
    // Avg pressure in first 5 min, value averaged from 0 to 5 min.
    pressureAvg[0] = ((pressureSamples[1] + pressureSamples[2]
                       + pressureSamples[3] + pressureSamples[4] + pressureSamples[5])
                      / 5);
  } else if (minuteCount == 35) {
    // Avg pressure in 30 min, value averaged from 0 to 5 min.
    pressureAvg[1] = ((pressureSamples[30] + pressureSamples[31]
                       + pressureSamples[32] + pressureSamples[33]
                       + pressureSamples[34]) / 5);
    float change = (pressureAvg[1] - pressureAvg[0]);
    if (firstRound) // first time initial 3 hour
      dP_dt = ((65.0 / 1023.0) * 2 * change); // note this is for t = 0.5hour
    else
      dP_dt = (((65.0 / 1023.0) * change) / 1.5); // divide by 1.5 as this is the difference in time from 0 value.
  } else if (minuteCount == 60) {
    // Avg pressure at end of the hour, value averaged from 0 to 5 min.
    pressureAvg[2] = ((pressureSamples[55] + pressureSamples[56]
                       + pressureSamples[57] + pressureSamples[58]
                       + pressureSamples[59]) / 5);
    float change = (pressureAvg[2] - pressureAvg[0]);
    if (firstRound) //first time initial 3 hour
      dP_dt = ((65.0 / 1023.0) * change); //note this is for t = 1 hour
    else
      dP_dt = (((65.0 / 1023.0) * change) / 2); //divide by 2 as this is the difference in time from 0 value
  } else if (minuteCount == 95) {
    // Avg pressure at end of the hour, value averaged from 0 to 5 min.
    pressureAvg[3] = ((pressureSamples[90] + pressureSamples[91]
                       + pressureSamples[92] + pressureSamples[93]
                       + pressureSamples[94]) / 5);
    float change = (pressureAvg[3] - pressureAvg[0]);
    if (firstRound) // first time initial 3 hour
      dP_dt = (((65.0 / 1023.0) * change) / 1.5); // note this is for t = 1.5 hour
    else
      dP_dt = (((65.0 / 1023.0) * change) / 2.5); // divide by 2.5 as this is the difference in time from 0 value
  } else if (minuteCount == 120) {
    // Avg pressure at end of the hour, value averaged from 0 to 5 min.
    pressureAvg[4] = ((pressureSamples[115] + pressureSamples[116]
                       + pressureSamples[117] + pressureSamples[118]
                       + pressureSamples[119]) / 5);
    float change = (pressureAvg[4] - pressureAvg[0]);
    if (firstRound) // first time initial 3 hour
      dP_dt = (((65.0 / 1023.0) * change) / 2); // note this is for t = 2 hour
    else
      dP_dt = (((65.0 / 1023.0) * change) / 3); // divide by 3 as this is the difference in time from 0 value
  } else if (minuteCount == 155) {
    // Avg pressure at end of the hour, value averaged from 0 to 5 min.
    pressureAvg[5] = ((pressureSamples[150] + pressureSamples[151]
                       + pressureSamples[152] + pressureSamples[153]
                       + pressureSamples[154]) / 5);
    float change = (pressureAvg[5] - pressureAvg[0]);
    if (firstRound) // first time initial 3 hour
      dP_dt = (((65.0 / 1023.0) * change) / 2.5); // note this is for t = 2.5 hour
    else
      dP_dt = (((65.0 / 1023.0) * change) / 3.5); // divide by 3.5 as this is the difference in time from 0 value
  } else if (minuteCount == 180) {
    // Avg pressure at end of the hour, value averaged from 0 to 5 min.
    pressureAvg[6] = ((pressureSamples[175] + pressureSamples[176]
                       + pressureSamples[177] + pressureSamples[178]
                       + pressureSamples[179]) / 5);
    float change = (pressureAvg[6] - pressureAvg[0]);
    if (firstRound) // first time initial 3 hour
      dP_dt = (((65.0 / 1023.0) * change) / 3); // note this is for t = 3 hour
    else
      dP_dt = (((65.0 / 1023.0) * change) / 4); // divide by 4 as this is the difference in time from 0 value
    pressureAvg[0] = pressureAvg[5]; // Equating the pressure at 0 to the pressure at 2 hour after 3 hours have past.
    firstRound = false; // flag to let you know that this is on the past 3 hour mark. Initialized to 0 outside main loop.
  }

  if (minuteCount < 35 && firstRound) //if time is less than 35 min on the first 3 hour interval.
    return 5; // Unknown, more time needed
  else if (dP_dt < (-0.25))
    return 4; // Quickly falling LP, Thunderstorm, not stable
  else if (dP_dt > 0.25)
    return 3; // Quickly rising HP, not stable weather
  else if ((dP_dt > (-0.25)) && (dP_dt < (-0.05)))
    return 2; // Slowly falling Low Pressure System, stable rainy weather
  else if ((dP_dt > 0.05) && (dP_dt < 0.25))
    return 1; // Slowly rising HP stable good weather
  else if ((dP_dt > (-0.05)) && (dP_dt < 0.05))
    return 0; // Stable weather
  else
    return 5; // Unknown
}

void sendVoltage(void)
// battery and charging values
{
  // get Battery Voltage & charge current
  float batteryVoltage = ((float)analogRead(batteryVoltage_PIN) * VccReference / 1024) * 2;	// actual voltage is double
  Serial.print("Batt: ");
  Serial.print(batteryVoltage);
  Serial.print("V ; ");
  float batteryChargeCurrent = ((float)analogRead(batteryChargeCurrent_PIN) * VccReference / 1024) / 2.5 ; // current(A) = V/Rprog(kohm)
  Serial.print(batteryChargeCurrent);
  Serial.println("A ");


  // get Solar Voltage & charge current
  float solarVoltage = ((float)analogRead(solarVoltage_PIN) / 1024 * VccReference) * 2 ;		// actual voltage is double
  Serial.print("Solar: ");
  Serial.print(solarVoltage);
  Serial.print("V ; ");
  // get Solar Current
  float solarCurrent = ((float)analogRead(solarCurrent_PIN) / 1024 * VccReference) / 2.5;		// current(A) = V/Rclprog(kohm)
  Serial.print(solarCurrent);
  Serial.print(" A; charge: ");
  Serial.println(digitalRead(LTC4067_CHRG_PIN) ? "No" : "Yes");

  // send battery percentage for node
  int battPct = 1 ;
  if (batteryVoltage > VccMin) {
    battPct = 100.0 * (batteryVoltage - VccMin) / (VccMax - VccMin);
  }
  Serial.print("BattPct: ");
  Serial.print(battPct);
  Serial.println("% ");

  if (lastBattPct != battPct) {
    gw.send(batteryVoltageMsg.set(batteryVoltage, 3));  		// Send (V)
    gw.send(batteryCurrentMsg.set(batteryChargeCurrent, 6));  	// Send (Amps)
    gw.send(solarVoltageMsg.set(solarVoltage, 3));  			// Send (V)
    gw.send(solarCurrentMsg.set(solarCurrent, 6));  			// Send (Amps)
    gw.sendBatteryLevel(battPct);
    lastBattPct = battPct;
  }
}


