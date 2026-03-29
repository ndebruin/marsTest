#pragma once
#include <Arduino.h>
#include <HardwareTimer.h>
#include "pinmap.h"

class PwmInput {
public:
    PwmInput(uint32_t Pin) :pin(Pin) {};

    uint8_t begin()
    {
        TIM_TypeDef *Instance = (TIM_TypeDef *)pinmap_peripheral(digitalPinToPinName(pin), PinMap_PWM);
        channelRising = STM_PIN_CHANNEL(pinmap_function(digitalPinToPinName(pin), PinMap_PWM));
        channelFalling = getMatchingChannel(channelRising);
    
        timer = new HardwareTimer(Instance);
        uint32_t PrescalerFactor = 1;
        timer->setPrescaleFactor(PrescalerFactor);
        timer->setOverflow(0x10000); // Max Period value to have the largest possible time to detect rising edge and avoid timer rollover
        timer->attachInterrupt(channelRising, [this] { TIMINPUT_Capture_Rising_IT_callback(); });
        timer->attachInterrupt(channelFalling, [this] { TIMINPUT_Capture_Falling_IT_callback(); });
        timer->attachInterrupt([this] { Rollover_IT_callback(); });

        timer->resume();

        input_freq = timer->getTimerClkFreq() / timer->getPrescaleFactor();
    
        return 0;
    };

    float getDutyCycle(){return DutycycleMeasured;};  // 0 - 100
    float getFrequency(){return FrequencyMeasured;};  // Hz

private:
    uint32_t pin;
    uint32_t channelRising, channelFalling;
    volatile uint32_t FrequencyMeasured, DutycycleMeasured, LastPeriodCapture = 0, CurrentCapture, HighStateMeasured;
    uint32_t input_freq = 0;
    volatile uint32_t rolloverCompareCount = 0;
    HardwareTimer *timer;

    uint32_t getMatchingChannel(uint32_t channel)
    {
        switch(channel){
            case 1: return 2;
            case 2: return 1;
            case 3: return 4;
            case 4: return 3;
        };
        return 0; // BAD
    };

    // following functions pulled from https://github.com/stm32duino/STM32Examples/blob/main/examples/Peripherals/HardwareTimer/Frequency_Dutycycle_measurement/Frequency_Dutycycle_measurement.ino
    void TIMINPUT_Capture_Rising_IT_callback(void)
    {
        CurrentCapture = timer->getCaptureCompare(channelRising);
        /* frequency computation */
        if (CurrentCapture > LastPeriodCapture)
        {
          FrequencyMeasured = input_freq / (CurrentCapture - LastPeriodCapture);
          DutycycleMeasured = (HighStateMeasured * 100) / (CurrentCapture - LastPeriodCapture);
        }
        else if (CurrentCapture <= LastPeriodCapture)
        {
          /* 0x1000 is max overflow value */
          FrequencyMeasured = input_freq / (0x10000 + CurrentCapture - LastPeriodCapture);
          DutycycleMeasured = (HighStateMeasured * 100) / (0x10000 + CurrentCapture - LastPeriodCapture);
        }

        LastPeriodCapture = CurrentCapture;
        rolloverCompareCount = 0;
    };
    void Rollover_IT_callback(void)
    {
        rolloverCompareCount++;

        if (rolloverCompareCount > 1)
        {
            FrequencyMeasured = 0;
            DutycycleMeasured = 0;
        }
    };
    void TIMINPUT_Capture_Falling_IT_callback(void)
    {
      /* prepare DutyCycle computation */
      CurrentCapture = timer->getCaptureCompare(channelFalling);

      if (CurrentCapture > LastPeriodCapture)
      {
        HighStateMeasured = CurrentCapture - LastPeriodCapture;
      }
      else if (CurrentCapture <= LastPeriodCapture)
      {
        /* 0x1000 is max overflow value */
        HighStateMeasured = 0x10000 + CurrentCapture - LastPeriodCapture;
      }
    };
};