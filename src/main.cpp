#include <Arduino.h>
// #include <Wire.h>
// #include <HardwareSerial.h>
#include <SPI.h>
#include "LSM6DSOSensor.h"
#include "ASM330LHHSensor.h"
#include "LIS2MDLSensor.h"
#include "LPS22HBSensor.h"

// initialize our sensor buses 
SPIClass SENSORS_SPI(SENSORS_SPI_MOSI, SENSORS_SPI_MISO, SENSORS_SPI_SCK);
// TwoWire GPS_I2C(GPS_I2C_SDA, GPS_I2C_SCL);
// HardwareSerial GPS_SERIAL(GPS_SERIAL_RX, GPS_SERIAL_TX);
// TwoWire CONNECTOR_I2C(CONNECTOR_I2C_SDA, CONNECTOR_I2C_SCL);
// SPIClass CAMERA_SPI(CAMERA_MOSI, CAMERA_MISO, CAMERA_SCK);
// HardwareSerial RADIO_SERIAL(RADIO_SERIAL_RX, RADIO_SERIAL_TX);

LSM6DSOSensor lsm(&SENSORS_SPI, SENSORS_LSM_CS);
ASM330LHHSensor asmSens(&SENSORS_SPI, SENSORS_ASM_CS);
LIS2MDLSensor lis(&SENSORS_SPI, SENSORS_LIS_CS);
LPS22HBSensor lps(&SENSORS_SPI, SENSORS_LPS_CS);

void setup()
{
    SerialUSB.begin(); while(!SerialUSB.available()){};
    pinMode(LED_GREEN, OUTPUT);



    Serial.println("spi begin");

    SENSORS_SPI.begin();

    SerialUSB.println(lsm.begin());
    SerialUSB.println("lsm begin");
    lsm.Enable_X();
    lsm.Enable_G();
    lsm.Set_G_FS(2000);
    lsm.Set_G_ODR(100);
    lsm.Set_X_FS(32);
    lsm.Set_X_ODR(500);

    SerialUSB.println(asmSens.begin());
    SerialUSB.println("asm begin");
    asmSens.Enable_X();
    asmSens.Enable_G();
    asmSens.Set_G_FS(4000);
    asmSens.Set_G_ODR(100);
    asmSens.Set_X_FS(16);
    asmSens.Set_X_ODR(500);

    SerialUSB.println(lis.begin());
    SerialUSB.println(lis.Enable());
    SerialUSB.println("lis begin");
    lis.SetOutputDataRate(100);

    SerialUSB.println(lps.begin());
    SerialUSB.println(lps.Enable());
    SerialUSB.println("lps begin");
    lps.SetODR(100);

}

void loop()
{
    
    digitalToggle(LED_GREEN);
    
    delay(100);

    int32_t lsmAccel[3];
    int32_t asmAccel[3];
    int32_t mag[3];
    float pressure, temperature;
    lsm.Get_X_Axes(lsmAccel);
    asmSens.Get_G_Axes(asmAccel);
    lis.GetAxes(mag);
    lps.GetPressure(&pressure);
    lps.GetTemperature(&temperature);

    // Output data.
    SerialUSB.print(" | LSM[g]: ");
    SerialUSB.print(lsmAccel[0]*2/1000.0);
    SerialUSB.print(" ");
    SerialUSB.print(lsmAccel[1]*2/1000.0);
    SerialUSB.print(" ");
    SerialUSB.print(lsmAccel[2]*2/1000.0);
    SerialUSB.print(" | ASM[g]: ");
    SerialUSB.print(asmAccel[0]/1000.0);
    SerialUSB.print(" ");
    SerialUSB.print(asmAccel[1]/1000.0);
    SerialUSB.print(" ");
    SerialUSB.print(asmAccel[2]/1000.0);
    SerialUSB.print(" | LIS[mgauss]: ");
    SerialUSB.print(mag[0]*1.5);
    SerialUSB.print(" ");
    SerialUSB.print(mag[1]*1.5);
    SerialUSB.print(" ");
    SerialUSB.print(mag[2]*1.5);
    SerialUSB.print(" | Pres[hPa]: ");
    SerialUSB.print(pressure, 2);
    SerialUSB.print(" | Temp[C]: ");
    SerialUSB.print(temperature, 2);
    SerialUSB.println(" |");
}