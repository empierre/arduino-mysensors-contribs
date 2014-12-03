/*

  Arduino soild mostire based on gypsum sensor

  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
  
 Sensor and calibration:
	DIY: See http://vanderleevineyard.com/1/category/vinduino/1.html
	Built: Davis / Watermark 200SS http://www.cooking-hacks.com/watermark-soil-moisture-sensor?_bksrc=item2item&_bkloc=product
  
 Connection:
	D6, D7: alternative powering to avoid sensor degradation
	A0, A1: alternative resisnatce mseuring

  Contributor: epierre

  Based on:	
   "Vinduino" portable soil moisture sensor code V3.00
   Date December 31, 2012
   Reinier van der Lee and Theodore Kaskalis
   www.vanderleevineyard.com
 */
 
#include <SPI.h>
#include <MySensor.h>  
#include <math.h>

#define NUM_READS 11    // Number of sensor reads for filtering
#define CHILD_ID 0

MySensor gw();  // Arduino initialization
MyMessage msg(CHILD_ID, V_HUM);  
unsigned long SLEEP_TIME = 30000; // Sleep time between reads (in milliseconds)


typedef struct {        // Structure to be used in percentage and resistance values matrix to be filtered (have to be in pairs)
  int moisture;
  long resistance;
} values;


const long knownResistor = 1200;  // Constant value of known resistor in Ohms

int activeDigitalPin = 6;         // 6 or 7 interchangeably
int supplyVoltageAnalogPin;       // 6-ON: A0, 7-ON: A1
int sensorVoltageAnalogPin;       // 6-ON: A1, 7-ON: A0

int supplyVoltage;                // Measured supply voltage
int sensorVoltage;                // Measured sensor voltage

values valueOf[NUM_READS];        // Calculated moisture percentages and resistances to be sorted and filtered

int i;                            // Simple index variable

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200); 
  
  gw.begin();
  
  gw.sendSketchInfo("Soil Moisture Sensor Gypsum", "1.0");
  gw.present(CHILD_ID, S_HUM);  
  
  // initialize the digital pin as an output.
  // Pin 6 is sense resistor voltage supply 1
  pinMode(6, OUTPUT);    

  // initialize the digital pin as an output.
  // Pin 7 is sense resistor voltage supply 2
  pinMode(7, OUTPUT);   

  delay(500);   
}

void loop() {

  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter

  for (i=0; i<NUM_READS; i++) {

		setupCurrentPath();      // Prepare the digital and analog pin values

		// Read 1 pair of voltage values
		digitalWrite(activeDigitalPin, HIGH);                 // set the voltage supply on
		delay(10);
		supplyVoltage = analogRead(supplyVoltageAnalogPin);   // read the supply voltage
		sensorVoltage = analogRead(sensorVoltageAnalogPin);   // read the sensor voltage
		digitalWrite(activeDigitalPin, LOW);                  // set the voltage supply off  
		delay(10); 

		// Calculate resistance and moisture percentage without overshooting 100
		// the 0.5 add-term is used to round to the nearest integer
		// Tip: no need to transform 0-1023 voltage value to 0-5 range, due to following fraction
		valueOf[i].resistance = long( float(knownResistor) * ( supplyVoltage - sensorVoltage ) / sensorVoltage + 0.5 );
		valueOf[i].moisture = min( int( pow( valueOf[i].resistance/31.65 , 1.0/-1.695 ) * 400 + 0.5 ) , 100 );
		//  valueOf[i].moisture = min( int( pow( valueOf[i].resistance/331.55 , 1.0/-1.695 ) * 100 + 0.5 ) , 100 );

    }
  // end of multiple read loop
  
  // Sort the moisture-resistance vector according to moisture
  sortMoistures();

  // Print out median values
  Serial.print("sensor resistance = ");
  Serial.println(valueOf[NUM_READS/2].resistance);
  gw.send(msg.set((long int)ceil(valueOf[NUM_READS/2].resistance)));

  
  // delay until next measurement (msec)
  gw.sleep(SLEEP_TIME);

}

void setupCurrentPath() {
  if ( activeDigitalPin == 6 ) {
    activeDigitalPin = 7;
    supplyVoltageAnalogPin = A1;
    sensorVoltageAnalogPin = A0;
  }
  else {
    activeDigitalPin = 6;
    supplyVoltageAnalogPin = A0;
    sensorVoltageAnalogPin = A1;
  }
}

// Selection sort algorithm
void sortMoistures() {
  int j;
  values temp;
  for(i=0; i<NUM_READS-1; i++)
    for(j=i+1; j<NUM_READS; j++)
      if ( valueOf[i].moisture > valueOf[j].moisture ) {
        temp = valueOf[i];
        valueOf[i] = valueOf[j];
        valueOf[j] = temp;
      }
}
