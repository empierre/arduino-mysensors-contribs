/*
    MutichannelGasSensor.cpp
    2015 Copyright (c) Seeed Technology Inc.  All right reserved.

    Author: Jacky Zhang
    2015-3-17
    http://www.seeed.cc/
    modi by Jack, 2015-8

    The MIT License (MIT)

    Copyright (c) 2015 Seeed Technology Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include <math.h>
#include <Wire.h>
#include <Arduino.h>
#include "MutichannelGasSensor.h"

/*********************************************************************************************************
** Function name:           begin
** Descriptions:            initialize I2C
*********************************************************************************************************/
void MutichannelGasSensor::begin(int address)
{
    Wire.begin();
    i2cAddress = address;
    is_connected = 0;
    if (readR0() >= 0) is_connected = 1;
}

void MutichannelGasSensor::begin()
{
    begin(DEFAULT_I2C_ADDR);
}

/*********************************************************************************************************
** Function name:           sendI2C
** Descriptions:            send one byte to I2C Wire
*********************************************************************************************************/
void MutichannelGasSensor::sendI2C(unsigned char dta)
{
    Wire.beginTransmission(i2cAddress);               // transmit to device #4
    Wire.write(dta);                                    // sends one byte
    Wire.endTransmission();                             // stop transmitting
}

/*********************************************************************************************************
** Function name:           readData
** Descriptions:            read 4 bytes from I2C slave
*********************************************************************************************************/
int16_t MutichannelGasSensor::readData(uint8_t cmd)
{
    uint16_t timeout = 0;
    uint8_t buffer[4];
    uint8_t checksum = 0;
    int16_t rtnData = 0;

    //send command
    sendI2C(cmd);
    //wait for a while
    delay(2);
    //get response
    Wire.requestFrom(i2cAddress, (uint8_t)4);    // request 4 bytes from slave device
    while(Wire.available() == 0)
    {
        if(timeout++ > 100)
            return -2;//time out
        delay(2);
    }
    if(Wire.available() != 4)
        return -3;//rtnData length wrong
    buffer[0] = Wire.read();
    buffer[1] = Wire.read();
    buffer[2] = Wire.read();
    buffer[3] = Wire.read();
    checksum = (uint8_t)(buffer[0] + buffer[1] + buffer[2]);
    if(checksum != buffer[3])
        return -4;//checksum wrong
    rtnData = ((buffer[1] << 8) + buffer[2]);

    return rtnData;//successful
}

/*********************************************************************************************************
** Function name:           readR0
** Descriptions:            read R0 stored in slave MCU
*********************************************************************************************************/
int16_t MutichannelGasSensor::readR0(void)
{
    int16_t rtnData = 0;

    rtnData = readData(0x11);
    if(rtnData >= 0)
        res0[0] = rtnData;
    else
        return rtnData;//unsuccessful

    rtnData = readData(0x12);
    if(rtnData >= 0)
        res0[1] = rtnData;
    else
        return rtnData;//unsuccessful

    rtnData = readData(0x13);
    if(rtnData >= 0)
        res0[2] = rtnData;
    else
        return rtnData;//unsuccessful

    return 0;//successful
}

/*********************************************************************************************************
** Function name:           readR
** Descriptions:            read resistance value of each channel from slave MCU
*********************************************************************************************************/
int16_t MutichannelGasSensor::readR(void)
{
    int16_t rtnData = 0;

    rtnData = readData(0x01);
    if(rtnData >= 0)
        res[0] = rtnData;
    else
        return rtnData;//unsuccessful

    rtnData = readData(0x02);
    if(rtnData >= 0)
        res[1] = rtnData;
    else
        return rtnData;//unsuccessful

    rtnData = readData(0x03);
    if(rtnData >= 0)
        res[2] = rtnData;
    else
        return rtnData;//unsuccessful

    return 0;//successful
}

/*********************************************************************************************************
** Function name:           readR
** Descriptions:            calculate gas concentration of each channel from slave MCU
** Parameters:
                            gas - gas type
** Returns:
                            float value - concentration of the gas
*********************************************************************************************************/
float MutichannelGasSensor::calcGas(int gas)
{
    if(!is_connected)
    {
        if(readR0() >= 0) is_connected = 1;
        else return -1.0f;
    }

    if(readR() < 0)
        return -2.0f;

    float ratio0 = (float)res[0] / res0[0];
    float ratio1 = (float)res[1] / res0[1];
    float ratio2 = (float)res[2] / res0[2];

    float c = 0;

    switch(gas)
    {
        case CO:
        {
            if(ratio1 < 0.01) ratio1 = 0.01;
            if(ratio1 > 3) ratio1 = 3;
            //c = pow(10, 0.6) / pow(ratio1, 1.2);
            c = pow(ratio1, -1.179)*4.385;  //mod by jack
            break;
        }
        case NO2:
        {
            if(ratio2 < 0.07) ratio2 = 0.07;
            if(ratio2 > 40) ratio2 = 40;
            //c = ratio2 / pow(10, 0.8);
            c = pow(ratio2, 1.007)/6.855;  //mod by jack
            break;
        }
        case NH3:
        {
            if(ratio0 < 0.04) ratio0 = 0.04;
            if(ratio0 > 0.8) ratio0 = 0.8;
            //c = 1 / (ratio0 * ratio0 * pow(10, 0.4));
            c = pow(ratio0, -1.67)/1.47;  //modi by jack
            break;
        }
        case C3H8:  //add by jack
        {
            if(ratio0 < 0.23) ratio0 = 0.23;
            if(ratio0 > 0.8) ratio0 = 0.8;
            c = pow(ratio0, -2.518)*570.164;
            break;
        }
        case C4H10:  //add by jack
        {
            if(ratio0 < 0.15) ratio0 = 0.15;
            if(ratio0 > 0.65) ratio0 = 0.65;
            c = pow(ratio0, -2.138)*398.107;
            break;
        }
        case CH4:  //add by jack
        {
            if(ratio1 < 0.5) ratio1 = 0.5;
            if(ratio1 > 0.7) ratio1 = 0.7;
            c = pow(ratio1, -4.363)*630.957;
            break;
        }
        case H2:  //add by jack
        {
            if(ratio1 < 0.04) ratio1 = 0.04;
            if(ratio1 > 0.8) ratio1 = 0.8;
            c = pow(ratio1, -1.8)*0.73;
            break;
        }
        case C2H5OH:  //add by jack
        {
            if(ratio1 < 0.04) ratio1 = 0.04;
            if(ratio1 > 1.1) ratio1 = 1.1;
            c = pow(ratio1, -1.552)*1.622;
            break;
        }
        default:
            break;
    }

    return isnan(c)?-3:c;
}

/*********************************************************************************************************
** Function name:           changeI2cAddr
** Descriptions:            change I2C address of the slave MCU, and this address will be stored in EEPROM of slave MCU
*********************************************************************************************************/
void MutichannelGasSensor::changeI2cAddr(uint8_t newAddr)
{
    Wire.beginTransmission(i2cAddress); // transmit to device
    Wire.write(0x23);              // sends one byte
    Wire.write(newAddr);              // sends one byte
    Wire.endTransmission();    // stop transmitting
    i2cAddress = newAddr;
}

/*********************************************************************************************************
** Function name:           doCalibrate
** Descriptions:            tell slave to do a calibration, it will take about 8s
                            after the calibration, must reread the R0 values
*********************************************************************************************************/
void MutichannelGasSensor::doCalibrate(void)
{
    sendI2C(0x22);
    delay(8000);
    if(readR0() >= 0) is_connected = 1;
    else is_connected = 0;
}

/*********************************************************************************************************
** Function name:           powerOn
** Descriptions:            power on sensor heater
*********************************************************************************************************/
void MutichannelGasSensor::powerOn(void)
{
    sendI2C(0x21);
}

/*********************************************************************************************************
** Function name:           powerOff
** Descriptions:            power off sensor heater
*********************************************************************************************************/
void MutichannelGasSensor::powerOff(void)
{
    sendI2C(0x20);
}

/*********************************************************************************************************
** Function name:           measure_xxx
** Descriptions:            measure the concentration of xxx, with unit ppm
*********************************************************************************************************/
float MutichannelGasSensor::measure_CO()
{
    return calcGas(CO);
}
float MutichannelGasSensor::measure_NO2()
{
    return calcGas(NO2);
}
float MutichannelGasSensor::measure_NH3()
{
    return calcGas(NH3);
}
float MutichannelGasSensor::measure_C3H8()
{
    return calcGas(C3H8);
}
float MutichannelGasSensor::measure_C4H10()
{
    return calcGas(C4H10);
}
float MutichannelGasSensor::measure_CH4()
{
    return calcGas(CH4);
}
float MutichannelGasSensor::measure_H2()
{
    return calcGas(H2);
}
float MutichannelGasSensor::measure_C2H5OH()
{
    return calcGas(C2H5OH);
}



MutichannelGasSensor mutichannelGasSensor;
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
