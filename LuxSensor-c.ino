/*
  Vera Arduino BH1750FVI Light sensor
  communicate using I2C Protocol
  this library enable 2 slave device addresses
  Main address  0x23
  secondary address 0x5C
  connect the sensor as follows :

  VCC  >>> 5V
  Gnd  >>> Gnd
  ADDR >>> NC or GND  
  SCL  >>> A5
  SDA  >>> A4
  
  Contribution: idefix/epierre for ceech
 
*/
#include <SPI.h>
#include <MySensor.h> 
#include <Wire.h> 				// I2C
#include <BH1750.h>

#define LTC4067_CHRG_PIN	A1		//analog input A1 on ATmega 328 is /CHRG signal from LTC4067
#define batteryVoltage_PIN	A0		//analog input A0 on ATmega328 is battery voltage ( /2)
#define solarVoltage_PIN	A2		//analog input A2 is solar cell voltage (/ 2)
#define solarCurrent_PIN	A6		//analog input A6 is input current ( I=V/Rclprog x 1000 )
#define batteryChargeCurrent_PIN	A7	//analog input A7 is battery charge current ( I=V/Rprog x 1000 )
#define LTC4067_SUSPEND_PIN	9		//digital output D9 - drive it high to put LTC4067 in SUSPEND mode

const float VccMin        = 1.0*3.5;  // Minimum expected Vcc level, in Volts. Example for 1 rechargeable lithium-ion.
const float VccMax        = 1.0*4.2;  // Maximum expected Vcc level, in Volts. 

#define LIGHT_SENSOR_ANALOG_PIN 3   // Digital input did you attach your soil sensor.  
#define CHILD_ID_LIGHT 0   // Id of the sensor child
#define BATT_CHILD_ID 10
#define SOLAR_CHILD_ID 11

// PIN Radio
#define RADIO_CE_PIN    7       // radio chip enable
#define RADIO_SS_PIN    8      // CS SS serial select

float lastBattVoltage;
float lastBattCurrent;
float lastSolarVoltage;
float lastSolarCurrent;
int lastBattPct = 0;
uint16_t lastlux;
float VccReference = 3.3 ;				// voltage reference for measurement, definitive init in setup

BH1750 lightSensor;
MySensor  gw(RADIO_CE_PIN, RADIO_SS_PIN);
unsigned long SLEEP_TIME = 30*1000;  // sleep time between reads (seconds * 1000 milliseconds)
MyMessage msg(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
MyMessage batteryVoltageMsg(BATT_CHILD_ID, V_VOLTAGE);		// Battery voltage (V)
MyMessage batteryCurrentMsg(BATT_CHILD_ID, V_CURRENT);		// Battery current (A)
MyMessage solarVoltageMsg(SOLAR_CHILD_ID, V_VOLTAGE);		// Solar voltage (V)
MyMessage solarCurrentMsg(SOLAR_CHILD_ID, V_CURRENT);		// Solar current (A)

void setup()  
{ 
 
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Light Lux Sensor", "1.0");
  // Register all sensors to gw (they will be created as child devices)  
  gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  gw.present(BATT_CHILD_ID, S_POWER);						// Battery parameters
  gw.present(SOLAR_CHILD_ID, S_POWER);					// Solar parameters
  
  // use VCC (3.3V) reference
  analogReference(DEFAULT);								// default external reference = 3.3v for Ceech board
  VccReference = 3.323 ;									// measured Vcc input (on board LDO)
  pinMode(LTC4067_SUSPEND_PIN, OUTPUT);					// suspend of Lion charger set
  digitalWrite(LTC4067_SUSPEND_PIN,LOW);   			//  active (non suspend) at start

  lightSensor.begin();
}
 
void loop()     
{     

  sendVoltage();
  
  uint16_t lux = lightSensor.readLightLevel();// Get Lux value
  Serial.println(lux);
  if (lux != lastlux) {
      gw.send(msg.set(lux));
      lastlux = lux;
  } 
  
  // Power down the radio
  gw.sleep(SLEEP_TIME);
}

void sendVoltage(void)
// battery and charging values
{
	// get Battery Voltage & charge current
	float batteryVoltage = ((float)analogRead(batteryVoltage_PIN)* VccReference/1024) * 2;	// actual voltage is double
	Serial.print("Batt: ");
	Serial.print(batteryVoltage);
	Serial.print("V ; ");
	float batteryChargeCurrent = ((float)analogRead(batteryChargeCurrent_PIN) * VccReference/1024)/ 2.5 ; // current(A) = V/Rprog(kohm)
	Serial.print(batteryChargeCurrent);
	Serial.println("A ");

   
	// get Solar Voltage & charge current
	float solarVoltage = ((float)analogRead(solarVoltage_PIN)/1024 * VccReference) * 2 ;		// actual voltage is double
	Serial.print("Solar: ");
	Serial.print(solarVoltage);
	Serial.print("V ; ");
	// get Solar Current
	float solarCurrent = ((float)analogRead(solarCurrent_PIN)/1024 * VccReference)/ 2.5;		// current(A) = V/Rclprog(kohm)
	Serial.print(solarCurrent);
	Serial.print(" A; charge: ");
	Serial.println(digitalRead(LTC4067_CHRG_PIN)?"No":"Yes");
	
	// send battery percentage for node
	int battPct = 1 ;
	if (batteryVoltage > VccMin){
		battPct = 100.0*(batteryVoltage - VccMin)/(VccMax - VccMin);
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

