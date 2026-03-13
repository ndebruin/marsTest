#include <Arduino.h>
// #include <Wire.h>
// #include <HardwareSerial.h>
#include <SPI.h>
#include "LSM6DSOSensor.h"

// initialize our sensor buses 
SPIClass SENSORS_SPI(PD7, PG9, PG11);
// TwoWire GPS_I2C(GPS_I2C_SDA, GPS_I2C_SCL);
// HardwareSerial GPS_SERIAL(GPS_SERIAL_RX, GPS_SERIAL_TX);
// TwoWire CONNECTOR_I2C(CONNECTOR_I2C_SDA, CONNECTOR_I2C_SCL);
// SPIClass CAMERA_SPI(CAMERA_MOSI, CAMERA_MISO, CAMERA_SCK);
// HardwareSerial RADIO_SERIAL(RADIO_SERIAL_RX, RADIO_SERIAL_TX);

LSM6DSOSensor accGyr(&SENSORS_SPI, SENSORS_LSM_CS);

void setup()
{
    SerialUSB.begin(); while(!SerialUSB.available()){};
    pinMode(LED_RED, OUTPUT);
    #ifdef HAL_SPI_MODULE_ENABLED
    SerialUSB.println("spi enabled!");
    #endif
    SerialUSB.println("help");

    const PinMap *map = PinMap_SPI_MOSI;
    while (map->pin != NC) {
        SerialUSB.print("pin: 0x");
        SerialUSB.print(map->pin, HEX);
        SerialUSB.print("  PD_7 is: 0x");
        SerialUSB.println(PD_7, HEX);
        map++;
    }



    Serial.println("spi begin");

    SENSORS_SPI.begin();

        SerialUSB.print("PD7  MODER: "); SerialUSB.println((GPIOD->MODER >> 14) & 0x3); // expect 2 (AF)
    SerialUSB.print("PD7  AFR:   "); SerialUSB.println((GPIOD->AFR[0] >> 28) & 0xF); // expect 5 (AF5_SPI1)
    SerialUSB.print("PG9  MODER: "); SerialUSB.println((GPIOG->MODER >> 18) & 0x3); // expect 2
    SerialUSB.print("PG11 MODER: "); SerialUSB.println((GPIOG->MODER >> 22) & 0x3); // expect 2




SerialUSB.println("help");
    // Send a test byte with CS manually controlled
    pinMode(SENSORS_LSM_CS, OUTPUT);
    digitalWrite(SENSORS_LSM_CS, HIGH);

    digitalWrite(SENSORS_LSM_CS, LOW);
    byte result = SENSORS_SPI.transfer(0xAA);
    digitalWrite(SENSORS_LSM_CS, HIGH);

    SerialUSB.print("Transfer result: 0x");
    SerialUSB.println(result, HEX);





    SerialUSB.println(accGyr.begin());
SerialUSB.println("help");
    accGyr.Enable_X();
    accGyr.Enable_G();
    accGyr.Set_G_FS(2000);
    accGyr.Set_G_ODR(100);
    accGyr.Set_X_FS(32);
    accGyr.Set_X_ODR(500);
}

void loop()
{
    
    digitalWrite(LED_RED,HIGH);
    digitalWrite(PD7,HIGH);
    
    delay(100);
    digitalWrite(LED_RED,LOW);
    digitalWrite(PD7,LOW);
    
    delay(100);

          int32_t accelerometer[3];
  int32_t gyroscope[3];
  accGyr.Get_X_Axes(accelerometer);
  accGyr.Get_G_Axes(gyroscope);

  // Output data.
  SerialUSB.print("| Acc[g]: ");
  SerialUSB.print(accelerometer[0]*2/1000.0);
  SerialUSB.print(" ");
  SerialUSB.print(accelerometer[1]*2/1000.0);
  SerialUSB.print(" ");
  SerialUSB.print(accelerometer[2]*2/1000.0);
  SerialUSB.print(" | Gyr[mdps]: ");
  SerialUSB.print(gyroscope[0]);
  SerialUSB.print(" ");
  SerialUSB.print(gyroscope[1]);
  SerialUSB.print(" ");
  SerialUSB.print(gyroscope[2]);
  SerialUSB.println(" |");
}