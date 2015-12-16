arduino
=======
MySensors scripts

Air Quality:
* AirQuality-MQ135.ino : for CO2/COV ... validated 
* AirQuality-CO2-MH-Z14.ino: CO2 through calibrated MH-Z14, validated
* AirQuality-Multiple_Gas_Sensor.ino : MQ2 MQ6 MQ131 MQ135 TGS2600 TGS2602 sensors
* DustSensor-*.ino : Dust sensors from several providers, validated for DSM
* AirQuality-MQ2.ino : for ethanol... ongoing
* AirQuality-MICS2614.ino: on going
* AirQuality-CO-NO2-NH3.ino: for Mics-6814 validated
* AirQuality-HCHO.ino: for HSHO, validated

Environmental sensors:
* PressureSensor.ino : validated, works well, too much temp reading given back
* SoundSensor2.ino: tested ok (not SPL)
* UVsensor.ino : validated 
* MotionSensor2.ino : motion sensor validated
* VibrationSensor.ino : simple vibration sensor, tested
* FloodSensor: tested
* LeafWetnessSensor.ino: validated, need an immersion gold sensor
* SoilMoistSensor.ino: validated
* SoilMoistSensorSHT1x.ino: validated, sensor cannot be burried

Variants for ceech board (Solar pannel + LiOn/LiPo NRF24L compatible board):
* PressureSensor-c.ino: validated
* LuxUVSensor-c.ino   : validated
* LuxSensor-c.ino     : validated

Energy sensors
* WaterMeterPulseSensor2.ino : for use with water meter that have a reed switch, validated 
* WaterMeterPulseSensor2-gs.ino : water meter with GreyScale Dfrobot sensor for Residia Jet water meter
