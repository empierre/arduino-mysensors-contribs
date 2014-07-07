/*
  Vera Arduino Sound

  connect the sensor as follows :

  +   >>> 5V
  -   >>> GND
  out >>> A0     
  
  Contribution: epierre
  based on :https://www.inkling.com/read/arduino-cookbook-michael-margolis-2nd/chapter-6/recipe-6-7
 
*/

#include <Sleep_n0m1.h>
#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>  
#include <Sensor.h>  
#include <Wire.h> 

#define CHILD_ID_SND 0
#define SND_SENSOR_ANALOG_PIN 0
unsigned long SLEEP_TIME = 30; // Sleep time between reads (in seconds)

const int ledPin = 13;            //the code will flash the LED in pin 13
const int middleValue = 512;      //the middle of the range of analog values
const int numberOfSamples = 128;  //how many readings will be taken each time
int sample;                       //the value read from microphone each time
long signal;                      //the reading once you have removed DC offset
long averageReading;              //the average of that loop of readings

long runningAverage=0;          //the running average of calculated values
const int averagedOver= 16;     //how quickly new values affect running average
                                //bigger numbers mean slower

const int threshold=400;        //at what level the light turns on

Sensor gw;
Sleep sleep;
int lastSND;

void setup()  
{ 
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Sound Sensor", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.sendSensorPresentation(CHILD_ID_SND, 22);

   pinMode(SND_SENSOR_ANALOG_PIN, INPUT);
   pinMode(ledPin, OUTPUT);

}

void loop()      
{     
 long sumOfSquares = 0;
  for (int i=0; i<numberOfSamples; i++) { //take many readings and average them
    sample = analogRead(0);               //take a reading
    signal = (sample - middleValue);      //work out its offset from the center
    signal *= signal;                     //square it to make all values positive
    sumOfSquares += signal;               //add to the total
  }
  averageReading = sumOfSquares/numberOfSamples;     //calculate running average
  runningAverage=(((averagedOver-1)*runningAverage)+averageReading)/averagedOver;

  if (runningAverage>threshold){         //is average more than the threshold ?
    digitalWrite(ledPin, HIGH);          //if it is turn on the LED
  }else{
    digitalWrite(ledPin, LOW);           //if it isn't turn the LED off
  }
  Serial.println(runningAverage);        //print the value so you can check it

  if (runningAverage != lastSND) {
      gw.sendVariable(CHILD_ID_SND, 37, runningAverage);
      lastSND = runningAverage;
  }
  
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  delay(1000); //delay to allow serial to fully print before sleep
  gw.powerDown();
  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepDelay(SLEEP_TIME * 1000); //sleep for: sleepTime 
}