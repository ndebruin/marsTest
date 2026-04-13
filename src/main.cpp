#include <Arduino.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <Servo.h>


#include "LSM6DSO32Sensor.h"
#include "ASM330LHHSensor.h"
#include "LIS2MDLSensor.h"
#include "LPS22HBSensor.h"

#include "LoRaE22.h"
#include "RadioConfigs.h"

#include "PwmInput.h"

#include "flatbuffers/flatbuffers.h"
#include "Rocket30KTelemetryPacket_generated.h"


// #define SENSOR_DEBUG
#define SENSOR_PRINT
// #define RADIO_DEBUG


// initialize our sensor buses 
SPIClass SENSORS_SPI(SENSORS_SPI_MOSI, SENSORS_SPI_MISO, SENSORS_SPI_SCK);
TwoWire GPS_I2C(GPS_I2C_SDA, GPS_I2C_SCL);
HardwareSerial GPS_SERIAL(GPS_SERIAL_RX, GPS_SERIAL_TX);
TwoWire CONNECTOR_I2C(CONNECTOR_I2C_SDA, CONNECTOR_I2C_SCL);
SPIClass CAMERA_SPI(CAMERA_MOSI, CAMERA_MISO, CAMERA_SCK);
HardwareSerial RADIO_SERIAL(RADIO_SERIAL_RX, RADIO_SERIAL_TX);

LSM6DSO32Sensor lsm(&SENSORS_SPI, SENSORS_LSM_CS);
ASM330LHHSensor asmSens(&SENSORS_SPI, SENSORS_ASM_CS);
LIS2MDLSensor lis(&SENSORS_SPI, SENSORS_LIS_CS);
LPS22HBSensor lps(&SENSORS_SPI, SENSORS_LPS_CS);

// PwmInput encoder1(ENCODER2_PWM);
// Servo servo1 = Servo();

const char* callsign = "KV0R";
LoRaE22 radioModule(&RADIO_SERIAL, RADIO_M0, RADIO_M1, RADIO_AUX, callsign);

// Implement this per your platform, then pass a callback to it in setup().
// This is a MARS SPECIFIC implementation.
bool changeSerialPortConfig(RadioConfigTypes::SerialSpeeds baudRate, RadioConfigTypes::ParityConfig parity){
  // this is safe to call even when the port is not open.
  RADIO_SERIAL.end();

  uint32_t baud = 0;
  uint16_t parityConfig = 0;
  
  // the radio's baud rates don't follow any pattern over the entire range, so ugly switch statement it is
  switch(baudRate){
    case RadioConfigTypes::SerialSpeeds::BAUD_1200:
      baud = 1200; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_2400:
      baud = 2400; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_4800:
      baud = 4800; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_9600:
      baud = 9600; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_19200:
      baud = 19200; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_38400:
      baud = 38400; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_57600:
      baud = 57600; break;
    case RadioConfigTypes::SerialSpeeds::BAUD_115200:
      baud = 115200; break;
  };
  // this is just easier
  switch(parity){
    case RadioConfigTypes::ParityConfig::Parity_8N1:
      parityConfig = SERIAL_8N1; break;
    case RadioConfigTypes::ParityConfig::Parity_8E1:
      parityConfig = SERIAL_8E1; break;
    case RadioConfigTypes::ParityConfig::Parity_8O1:
      parityConfig = SERIAL_8O1; break;
  };
  
  RADIO_SERIAL.begin(baud, parityConfig);

  return true;
};


void sensorInit();
void sensorUpdate();
void radioInit();
void radioUpdate();


void setup()
{
  SerialUSB.begin(); while(!SerialUSB.available()){};
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);


  // SerialUSB.println(encoder1.begin());
  // servo1.attach(PWM_OUT3);


  flatbuffers::FlatBufferBuilder builder;

  auto packet = hprc::CreateRocket30KTelemetryPacket(builder);
  
  sensorInit();
  // radioInit();
}

unsigned long long counterBufferNotEmpty = 0;
void loop()
{
  digitalToggle(LED_GREEN);
  sensorUpdate();
  // radioUpdate();

  





  // SerialUSB.print(" freq: "); SerialUSB.print(encoder1.getFrequency());
  // SerialUSB.print(" duty cycle: "); SerialUSB.print(encoder1.getDutyCycle());
  // SerialUSB.println("");`
  // servo1.write(90);
  delay(50);
}






void radioInit(){
  // build our config
  RadioConfig config;
  config.address = ADDRESS;
  config.networkId = NETWORKID;
  config.encryptionKey = ENCRYPTIONKEY;
  config.parityConfig = PARITYCONFIG;
  config.serialSpeed = SERIALSPEED;
  config.airDataRate = AIRDATARATE;
  config.packetSize = PACKETSIZE;
  config.worMode = WORMODE;
  config.worPeriod = WORPERIOD;
  config.relayMode = RELAYMODE;
  config.destination = DESTINATIONMODE;
  config.txPower = dBm33;
  config.ambientRSSIEnabled = AMBIENTRSSI;
  config.rssiReadingsEnabled = RSSIREADINGS;
  config.listenBeforeTxEnable = LISTENBEFORETX;
  radioModule.setConfig(config);
  radioModule.setFrequency(FREQUENCY);

  radioModule.changeSerialPortCallback(changeSerialPortConfig);
  radioModule.setTimeout(2000);


  #ifdef RADIO_DEBUG
    uint8_t configBuffer[9];
    radioModule.buildConfigBuffer((unsigned char*)&configBuffer);
    SerialUSB.print("desired config registers: ");
    for (size_t i; i<sizeof(configBuffer);i++){
      SerialUSB.printf("%0X ",configBuffer[i]);
    }
    SerialUSB.println("");
  #endif

  int8_t code = radioModule.init(3);
  radioModule.setMode(RadioMode::Normal);

  #ifdef RADIO_DEBUG
    SerialUSB.println("done initing");
    SerialUSB.println(code);
  #endif

  
}

void radioUpdate(){
   if(radioModule.moduleReady()){
      // SerialUSB.println("buffer empty and ready!");
      counterBufferNotEmpty = 0;
   }
   else{
      counterBufferNotEmpty++;
   }

   uint8_t buffer[3] = {'E', 'X', 'P'};
   // uint8_t label[3] = {'E', 'X', 'P'};
   // for(size_t i = 0; i<235;i++){
   //   buffer[i] = label[i%3];
   // }

   if((sizeof(buffer) + sizeof(callsign) + 1) * counterBufferNotEmpty < 1000){
      radioModule.sendMessage(buffer, sizeof(buffer));
      // SerialUSB.println("help");
   }
}

void sensorInit(){
    pinMode(SENSORS_ASM_CS, OUTPUT); digitalWrite(SENSORS_ASM_CS, HIGH);
    pinMode(SENSORS_LSM_CS, OUTPUT); digitalWrite(SENSORS_LSM_CS, HIGH);
    pinMode(SENSORS_LIS_CS, OUTPUT); digitalWrite(SENSORS_LIS_CS, HIGH);
    pinMode(SENSORS_LPS_CS, OUTPUT); digitalWrite(SENSORS_LPS_CS, HIGH);

    #ifdef SENSOR_DEBUG
        SerialUSB.println("spi begin");
    #endif

    SENSORS_SPI.begin();



   delay(20);

    LSM6DSO32StatusTypeDef lsmBeginStatus = lsm.begin();
    #ifdef SENSOR_DEBUG
        SerialUSB.println(lsmBeginStatus);
        SerialUSB.println("lsm begin");
    #endif
    lsm.Set_G_FS(2000);
    lsm.Set_G_ODR(100);
    lsm.Set_X_FS(32);
    lsm.Set_X_ODR(500);
    lsm.Enable_X();
    lsm.Enable_G();

    delay(20);

    ASM330LHHStatusTypeDef asmBeginStatus = asmSens.begin();
    #ifdef SENSOR_DEBUG
        SerialUSB.println(asmBeginStatus);
        SerialUSB.println("asm begin");
    #endif    
    asmSens.Set_G_FS(4000);
    asmSens.Set_G_ODR(100);
    asmSens.Set_X_FS(16);
    asmSens.Set_X_ODR(500);
    asmSens.Enable_X();
    asmSens.Enable_G();
    
    delay(20);

    LIS2MDLStatusTypeDef lisBeginStatus = lis.begin();
    #ifdef SENSOR_DEBUG
        SerialUSB.println(lisBeginStatus);
        SerialUSB.println("lis begin");
    #endif
    lis.SetOutputDataRate(100);
    lis.Enable();

    delay(20);


  LPS22HBStatusTypeDef lpsBeginStatus = lps.begin();
   #ifdef SENSOR_DEBUG
      SerialUSB.println(lpsBeginStatus);
      SerialUSB.println("lps begin");
   #endif
   lps.SetODR(100);
   lps.Enable();
  //  float odr;
  //  lps.GetODR(&odr);
  //  SerialUSB.println(odr);
}

void sensorUpdate()
{
   int32_t lsmAccel[3];
   int32_t asmAccel[3];
   int32_t mag[3];
   float pressure, temperature;
   lsm.Get_X_Axes(lsmAccel);
   asmSens.Get_X_Axes(asmAccel);
   lis.GetAxes(mag);
   lps.GetPressure(&pressure);
   lps.GetTemperature(&temperature);

   #ifdef SENSOR_PRINT
   // Output data.
   SerialUSB.print(" | LSM[g]: ");
   SerialUSB.print(lsmAccel[0]/1000.0);
   SerialUSB.print(" ");
   SerialUSB.print(lsmAccel[1]/1000.0);
   SerialUSB.print(" ");
   SerialUSB.print(lsmAccel[2]/1000.0);
   SerialUSB.print(" | ASM[g]: ");
   SerialUSB.print(asmAccel[0]/1000.0);
   SerialUSB.print(" ");
   SerialUSB.print(asmAccel[1]/1000.0);
   SerialUSB.print(" ");
   SerialUSB.print(asmAccel[2]/1000.0);
   SerialUSB.print(" | LIS[mgauss]: ");
   SerialUSB.print(mag[0]);
   SerialUSB.print(" ");
   SerialUSB.print(mag[1]);
   SerialUSB.print(" ");
   SerialUSB.print(mag[2]);
   SerialUSB.print(" | Pres[hPa]: ");
   SerialUSB.print(pressure, 2);
   SerialUSB.print(" | Temp[C]: ");
   SerialUSB.print(temperature, 2);
   SerialUSB.println(" |");
   #endif
}