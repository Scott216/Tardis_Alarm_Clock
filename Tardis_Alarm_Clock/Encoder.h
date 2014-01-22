/* 
Encoder Source: http://arduino.cc/playground/Main/RotaryEncoders#Example15
To avoid conflicts with  I2C which uses Pin 2 & 3, encoder will use interrupts 2 (pin D0) and 3 (pin D1).
*/

#ifndef ENCODER_H
#define ENCODER_H

#include "Arduino.h"
// #include <PinChangeInt.h>  // http://code.google.com/p/arduino-pinchangeint/wiki/Usage


class Encoder
{
  public:
    Encoder();
    ~Encoder();
    void begin(int encoderPinA, int encoderPinB);
    int currentValue();
    int previousValue();
    void resetValue();  // set encoder value to zero
    bool isTurning(uint32_t timeThreshold);
    

  protected:
    int lastReportedPos;      // change management
    // Set interrupt functions
    static void rotatingCW();  // Static function is referenced without making an instance of the class.  See: http://stackoverflow.com/questions/4124102/whats-a-static-method-in-c
    static void rotatingCCW();
    static boolean A_set;
    static boolean B_set;
    static boolean rotating;
    static int encoderPinA;
    static int encoderPinB;
    
};



#endif

