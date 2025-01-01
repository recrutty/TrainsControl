const int Pot1 = A0;
const int Pot2 = A1;
const int Pot3 = A2;

const int pwmOut1 = 9;
const int pwmOut2 = 10;
const int pwmOut3 = 11;

const int negativeOutputEnable1 = 2;
const int positiveOutputEnable1 = 3;
const int negativeOutputEnable2 = 4;
const int positiveOutputEnable2 = 5;
const int negativeOutputEnable3 = 6;
const int positiveOutputEnable3 = 7;

#include "PowerControl.h"

PowerControl* powerControl1;
PowerControl* powerControl2;
PowerControl* powerControl3;

void setup()
{
    powerControl1 = new PowerControl(Pot1, pwmOut1, positiveOutputEnable1, negativeOutputEnable1);
    powerControl2 = new PowerControl(Pot2, pwmOut2, positiveOutputEnable2, negativeOutputEnable2);
    powerControl3 = new PowerControl(Pot3, pwmOut3, positiveOutputEnable3, negativeOutputEnable3);
}

void loop() 
{
    powerControl1->updateOutput();
    powerControl2->updateOutput();
    powerControl3->updateOutput();
}
