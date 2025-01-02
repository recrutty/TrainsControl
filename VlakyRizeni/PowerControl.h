#pragma once

struct DirectionSwitchingContext
{
    int endSwitch1Pin;
    int endSwitch2Pin;
    int enablePin;

    bool reversed = false;
};

enum class PotState
{
    DeadZone,
    NegativeValue,
    PositiveValue,
    Unknown
};

enum class Direction
{
    Forward,
    Backward
};

class PowerControl
{
    private:
        //Editable values. Adjust variable due to HW behavioral
        const int potDeadZoneSize = 200;

    public:
        PowerControl(int potPin, int pwmOutPin, int positiveOutputEnablePin, int negativeOutputEnablePin, DirectionSwitchingContext* switchingContext);
        
        auto updateOutput()                             -> void;
        
    private:
        auto GetPotState(int analogValue)               -> PotState;
        auto TurnOffOutput()                            -> void;
        auto SetOutput(Direction direction)             -> void;
        auto SetOutputPwm(int lowInputLimit,
                          int highInputLimit, 
                          int relativeValue, 
                          bool inverted)                -> void;
        auto ShouldReverseDirection()                   -> bool;
    
        const int potPin;
        const int pwmOutPin;
        const int positiveOutputEnablePin;
        const int negativeOutputEnablePin;
        DirectionSwitchingContext* switchingContext;

        //Do not edit this
        const long analogInMaxValue      = 1023;
        const long analogInHalfValue     = analogInMaxValue / 2;
        
        const long potDeadZoneHalfSize   = potDeadZoneSize / 2;
        const long potDeadZoneLowerLimit = analogInHalfValue - potDeadZoneHalfSize + 1;
        const long potDeadZoneUpperLimit = analogInHalfValue + potDeadZoneHalfSize;
        
        const unsigned int pwmMaxOutput  = 255;
        
};

PowerControl::PowerControl(int potPinInst, int pwmOutPinInst, int positiveOutputEnablePinInst, int negativeOutputEnablePinInst, DirectionSwitchingContext* switchingContextInst) :
    potPin(potPinInst),
    pwmOutPin(pwmOutPinInst),
    positiveOutputEnablePin(positiveOutputEnablePinInst),
    negativeOutputEnablePin(negativeOutputEnablePinInst),
    switchingContext(switchingContextInst)
{
    pinMode(potPin,                     INPUT);
    pinMode(pwmOutPin,                  OUTPUT);
    pinMode(positiveOutputEnablePin,    OUTPUT);
    pinMode(negativeOutputEnablePin,    OUTPUT);
    
    if(switchingContext)
    {
        pinMode(switchingContext->endSwitch1Pin, INPUT_PULLUP);
        pinMode(switchingContext->endSwitch2Pin, INPUT_PULLUP);
        pinMode(switchingContext->enablePin,     INPUT_PULLUP);
    }
}

auto PowerControl::updateOutput() -> void
{
    auto potValue = analogRead(potPin);

    switch(GetPotState(potValue))
    {
        case PotState::DeadZone:
        {
            TurnOffOutput();
            break;
        }
        case PotState::NegativeValue:
        {
            SetOutput(ShouldReverseDirection() ? Direction::Forward : Direction::Backward);
            SetOutputPwm(0, potDeadZoneLowerLimit, potValue, true);
            break;
        }
        case PotState::PositiveValue:
        {
            SetOutput(ShouldReverseDirection() ? Direction::Backward : Direction::Forward);
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

auto PowerControl::SetOutput(Direction direction) -> void
{
    switch(direction)
    {
        case Direction::Forward:
        {
            digitalWrite(positiveOutputEnablePin, 0);
            digitalWrite(negativeOutputEnablePin, 1);
            break;
        }
        case Direction::Backward:
        {
            digitalWrite(negativeOutputEnablePin, 0);
            digitalWrite(positiveOutputEnablePin, 1);
            break;
        }
    }
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

auto PowerControl::ShouldReverseDirection() -> bool
{
    if (switchingContext == nullptr)
    {
        return false;
    }
    
    bool switchingDisabled = digitalRead(switchingContext->enablePin);
    if (switchingDisabled)
    {
        return false;
    }
    
    if(switchingContext->reversed)
    {
        bool endSwitch2Active = !digitalRead(switchingContext->endSwitch2Pin);
        
        if(endSwitch2Active)
        {
            switchingContext->reversed = false;
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        bool endSwitch1Active = !digitalRead(switchingContext->endSwitch1Pin);
        
        if(endSwitch1Active)
        {
            switchingContext->reversed = true;
            return true;
        }
        else
        {
            return false;
        }
    }
}
