arduino
=======
MySensors scripts

Air Quality:
* AirQuality-MQ135.ino : for CO2/COV ... validated 
* AirQuality-CO2-MH-Z14.ino: CO2 through calibrated MH-Z14, validated
* AirQuality-Multiple_Gas_Sensor.ino : MQ2 MQ6 MQ131 MQ135 TGS2600 TGS2602 sensors
* DustSensor-*.ino : Dust sensors from several providers, validated for DSM
* AirQuality-MQ2.ino : for ethanol... still ongoing...
* AirQuality-MICS2614.ino: ongoing

Environmental sensors:
* PressureSensor.ino : validated, works well, too much temp reading given back
* SoundSensor2.ino: not tested
* UVsensor.ino : validated 
* MotionSensor2.ino : motion sensor
* VibrationSensor.ino : simple vibration sensor
* FloodSensor: not tested
* LeafWetnessSensor.ino: validated, need an immersion gold sensor
* SoilMoistSensorGypsum.ino: not tested
* SoilMoistSensorSHT1x.ino: validated, sensor cannot be burried

Variants for ceech board (Solar pannel + LiOn/LiPo NRF24L compatible board):
* PressureSensor-c.ino
* LuxUVSensor-c.ino
* LuxSensor-c.ino 

Energy sensors
* WaterMeterPulseSensor2.ino : for use with water meter that have a reed switch, validated 
* WaterMeterPulseSensor2-gs.ino : water meter with GreyScale Dfrobot sensor for Residia Jet water meter
