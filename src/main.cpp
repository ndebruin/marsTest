#include <Arduino.h>
#include <SPI.h>
#include <ASM330LHHSensor.h>
// #include <LSM6DSOSensor.h>

SPIClass dev_spi(PD_7, PG_9, PG_11);

#define ASM_CS PD_5
#define LSM_CS PB_4
#define LIS_CS PA_15
#define LPS_CS PD_0

ASM330LHHSensor AccGyr(&dev_spi, ASM_CS, 2000000);
// LSM6DSOSensor AccGyr(&dev_spi, LSM_CS, 2000000);


void setup()
{
    SerialUSB.begin();
    while(!SerialUSB.available()){};
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    SerialUSB.println("preinit");
    // digitalWrite(LED_GREEN, LOW);
    dev_spi.setMISO(PG_9);
    dev_spi.setMOSI(PD_7);
    dev_spi.setSCLK(PG_11);
    
    dev_spi.begin();
    // dev_spi.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3));

    pinMode(PF4_ALT0, INPUT_PULLDOWN);
    // pinMode(pinNametoDigitalPin(PB_3_ALT2), INPUT_PULLDOWN);

    pinMode(LPS_CS, OUTPUT); digitalWrite(LPS_CS, HIGH);
    pinMode(LIS_CS, OUTPUT); digitalWrite(LIS_CS, HIGH);
    pinMode(ASM_CS, OUTPUT); //digitalWrite(ASM_CS, HIGH);
    pinMode(LSM_CS, OUTPUT); digitalWrite(LSM_CS, HIGH);

    pinMode(ASM_CS, OUTPUT); digitalWrite(ASM_CS, LOW);
    // digitalWrite(LSM_CS, LOW);

    SerialUSB.println("init");
    SerialUSB.println(AccGyr.begin());

    SerialUSB.println(AccGyr.Set_X_ODR(104.0));
    SerialUSB.println(AccGyr.Set_G_ODR(104.0));
    SerialUSB.println(AccGyr.Set_X_FS(16));
    SerialUSB.println(AccGyr.Set_G_FS(4000));

    SerialUSB.println("enable modes");
    SerialUSB.println(AccGyr.Enable_X());
    SerialUSB.println(AccGyr.Enable_G());

    delay(50);
    SerialUSB.println("have data ready?");
    uint8_t status;
    AccGyr.Get_X_DRDY_Status(&status);
    SerialUSB.println(status);
    AccGyr.Get_G_DRDY_Status(&status);
    SerialUSB.println(status);


    int32_t st;
    AccGyr.Get_X_FS(&st);
    SerialUSB.println(st);
    AccGyr.Get_G_FS(&st);
    SerialUSB.println(st);
  

    // AccGyr.Enable_X();
    // AccGyr.Enable_G();
}

void loop()
{
    digitalToggle(LED_GREEN);
    delay(500);

    int32_t accelerometer[3] = {};
    int32_t gyroscope[3] = {};
    int status;
    status = AccGyr.Get_X_Axes(accelerometer);
    if(status !=0){SerialUSB.println("ERROR");}
    status = AccGyr.Get_G_Axes(gyroscope);
    if(status !=0){SerialUSB.println("ERROR");}

  // Output data.
  SerialUSB.print("LSM6DSO: | Acc[mg]: ");
  SerialUSB.print(accelerometer[0]);
  SerialUSB.print(" ");
  SerialUSB.print(accelerometer[1]);
  SerialUSB.print(" ");
  SerialUSB.print(accelerometer[2]);
  SerialUSB.print(" | Gyr[mdps]: ");
  SerialUSB.print(gyroscope[0]);
  SerialUSB.print(" ");
  SerialUSB.print(gyroscope[1]);
  SerialUSB.print(" ");
  SerialUSB.print(gyroscope[2]);
  SerialUSB.println(" |");
}