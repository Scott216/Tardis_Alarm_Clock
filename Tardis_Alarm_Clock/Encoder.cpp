#include "Encoder.h"

boolean Encoder::rotating;   // debounce management
boolean Encoder::A_set;
boolean Encoder::B_set;
volatile int encoderPos;             // a counter for the dial
volatile uint32_t lastRotatedTime;   // millis timestamp when encoder was last turned
int Encoder::encoderPinA;
int Encoder::encoderPinB;


void doEncoderA();
void doEncoderB();

// Constructor
Encoder::Encoder()
{}

// Destructor
Encoder::~Encoder()
{}

// Setup encoder pins
void Encoder::begin(int encPinA, int encPinB)
{
  Encoder::encoderPinA = encPinA;
  Encoder::encoderPinB = encPinB;
  
  pinMode(Encoder::encoderPinA, INPUT_PULLUP); 
  pinMode(Encoder::encoderPinB, INPUT_PULLUP);
  resetValue();
  Encoder::rotating = false;
  lastRotatedTime = 0;
  // interrupt service routine vars
  Encoder::A_set = false;
  Encoder::B_set = false;

  // encoder pin on interrupt 0 on pin D2
  attachInterrupt(0, rotatingCW, CHANGE);

  // encoder pin on interrupt 1 on pin D3
//srg using pin 3 for PWM, can't use for interrupt.  Need to use PinChangeInt.h  http://code.google.com/p/arduino-pinchangeint/wiki/Usage
 // attachInterrupt(1, rotatingCCW, CHANGE);
  
  // http://code.google.com/p/arduino-pinchangeint/wiki/Usage
  
} // begin()


int Encoder::currentValue()
{
  Encoder::rotating = true;  // reset the debouncer
  return encoderPos;
}

int Encoder::previousValue()
{
  Encoder::rotating = true;  // reset the debouncer
  return lastReportedPos;
}

// Reset encoder positions
void Encoder::resetValue()
{
  encoderPos = 0;
  lastReportedPos = 0;
}


// See if the encoder was moved recentlty base on timeTrheshold in mS
bool Encoder::isTurning(uint32_t timeThreshold)
{
  Encoder::rotating = true;  // reset the debouncer
  
  if ((long)(millis() - lastRotatedTime) > timeThreshold)
  { return false; }
  else
  { return true; }

}  // isTurning()


void Encoder::rotatingCW()
{
  // debounce
  if ( Encoder::rotating ) delay (1);  // wait a little until the bouncing is done
  
  // Test transition, did things really change? 
  if( digitalRead(Encoder::encoderPinA) != A_set ) 
  {  // debounce once more
    Encoder::A_set = !Encoder::A_set;

    // adjust counter + if A leads B
    if ( Encoder::A_set && !Encoder::B_set ) 
    {  
      encoderPos += 1; 
      lastRotatedTime = millis();
    }
    Encoder::rotating = false;  // no more debouncing until loop() hits again
  }
  
} // rotatingCW

void Encoder::rotatingCCW()
{
  if ( Encoder::rotating ) delay (1);  // wait a little until the bouncing is done
  
  // Test transition, did things really change? 
  if( digitalRead(Encoder::encoderPinB) != Encoder::B_set ) 
  {  // debounce once more
    Encoder::B_set = !Encoder::B_set;
    
    //  adjust counter - 1 if B leads A
    if( Encoder::B_set && !Encoder::A_set ) 
    { 
      encoderPos -= 1; 
      lastRotatedTime = millis();
    }
    Encoder::rotating = false;
  }
}  // rotatingCCW()






