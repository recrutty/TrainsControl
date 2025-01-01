#pragma once

enum class PotState
{
    DeadZone,
    NegativeValue,
    PositiveValue,
    Unknown
};

class PowerControl
{
    
    private:
        //Editable values. Adjust variable due to HW behavioral
        const int potDeadZoneSize = 100;

    public:
        PowerControl(int PotPin, int pwmOutPin, int positiveOutputEnablePin, int negativeOutputEnablePin);
        
        auto updateOutput()                             -> void;
        
    private:
        auto GetPotState(int analogValue)               -> PotState;
        auto TurnOffOutput()                            -> void;
        auto SetOutputPwm(int lowInputLimit,
                          int highInputLimit, 
                          int relativeValue, 
                          bool inverted)                -> void;
    
        const int PotPin;
        const int pwmOutPin;
        const int positiveOutputEnablePin;
        const int negativeOutputEnablePin;
        
        //Do not edit this
        const long analogInMaxValue      = 1023;
        const long analogInHalfValue     = analogInMaxValue / 2;
        
        const long potDeadZoneHalfSize   = potDeadZoneSize / 2;
        const long potDeadZoneLowerLimit = analogInHalfValue - potDeadZoneHalfSize + 1;
        const long potDeadZoneUpperLimit = analogInHalfValue + potDeadZoneHalfSize;
        
        const unsigned int pwmMaxOutput          = 255;
};

PowerControl::PowerControl(int PotPin, int pwmOutPin, int positiveOutputEnablePin, int negativeOutputEnablePin) :
    PotPin(PotPin),
    pwmOutPin(pwmOutPin),
    positiveOutputEnablePin(positiveOutputEnablePin),
    negativeOutputEnablePin(negativeOutputEnablePin) 
{
    pinMode(PotPin,                     INPUT);
    pinMode(pwmOutPin,                  OUTPUT);
    pinMode(positiveOutputEnablePin,    OUTPUT);
    pinMode(negativeOutputEnablePin,    OUTPUT);
}

auto PowerControl::updateOutput() -> void
{
    auto potValue = analogRead(PotPin);
    
    switch(GetPotState(potValue))
    {
        case PotState::DeadZone:
        {
            TurnOffOutput();
            break;
        }
        case PotState::NegativeValue:
        {
            digitalWrite(positiveOutputEnablePin, 0);
            digitalWrite(negativeOutputEnablePin, 1);
            
            SetOutputPwm(0, potDeadZoneLowerLimit, potValue, true);
            break;
        }
        case PotState::PositiveValue:
        {
            digitalWrite(positiveOutputEnablePin, 1);
            digitalWrite(negativeOutputEnablePin, 0);
            
            SetOutputPwm(potDeadZoneUpperLimit, analogInMaxValue, potValue, false);
            break;
        }
        default:
            break;
    }
}

auto PowerControl::GetPotState(int analogValue) -> PotState
{
    PotState retVal = PotState::Unknown;
    
    if ((analogValue >= potDeadZoneLowerLimit) && (analogValue <= potDeadZoneUpperLimit))
    {
        retVal = PotState::DeadZone;
    }
    
    if (analogValue < potDeadZoneLowerLimit)
    {
        retVal = PotState::NegativeValue;
    }
    
    if (analogValue > potDeadZoneUpperLimit)
    {
        retVal = PotState::PositiveValue;
    }
    
    return retVal;
}

auto PowerControl::TurnOffOutput() -> void
{
    analogWrite(pwmOutPin, 0);
    digitalWrite(positiveOutputEnablePin, 0);
    digitalWrite(negativeOutputEnablePin, 0);
}

auto PowerControl::SetOutputPwm(int lowInputLimit, int highInputLimit, int relativeValue, bool inverted) -> void
{    
    long absoluteValue = relativeValue - lowInputLimit;
    long absoluteLimit = highInputLimit - lowInputLimit;
    
    unsigned long outValue = (absoluteValue * pwmMaxOutput) / absoluteLimit;
    if(inverted)
    {
        outValue = pwmMaxOutput - outValue;
    }

    analogWrite(pwmOutPin, outValue);
}
