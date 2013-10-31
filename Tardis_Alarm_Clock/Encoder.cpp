#include "Encoder.h"

boolean rotating;   // debounce management
boolean A_set;  
boolean B_set;
volatile int encoderPos;             // a counter for the dial
volatile uint32_t lastRotatedTime;   // millis timestamp when encoder was last turned
int encoderPinA;
int encoderPinB;

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
  
  encoderPinA = encPinA;
  encoderPinB = encPinB;
  
  
  pinMode(encoderPinA, INPUT_PULLUP); 
  pinMode(encoderPinB, INPUT_PULLUP);
  resetValue();
  rotating = false;
  lastRotatedTime = 0;
  // interrupt service routine vars
  A_set = false;              
  B_set = false;


  // encoder pin on interrupt 0 on pin D2
  attachInterrupt(0, rotatingCW, CHANGE);

  // encoder pin on interrupt 1 on pin D3
//srg using pin 3 for PWM, can't use for interrupt
 // attachInterrupt(1, rotatingCCW, CHANGE);
  
} // begin()


int Encoder::currentValue()
{
  rotating = true;  // reset the debouncer
  return encoderPos;
}

int Encoder::previousValue()
{
  rotating = true;  // reset the debouncer
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
  rotating = true;  // reset the debouncer
  
  if ((long)(millis() - lastRotatedTime) > timeThreshold)
  { return false; }
  else
  { return true; }

}  // isTurning()


void Encoder::rotatingCW()
{
  // debounce
  if ( rotating ) delay (1);  // wait a little until the bouncing is done
  
  // Test transition, did things really change? 
  if( digitalRead(encoderPinA) != A_set ) 
  {  // debounce once more
    A_set = !A_set;

    // adjust counter + if A leads B
    if ( A_set && !B_set ) 
    {  
      encoderPos += 1; 
      lastRotatedTime = millis();
    }
    rotating = false;  // no more debouncing until loop() hits again
  }
  
} // rotatingCW

void Encoder::rotatingCCW()
{
  if ( rotating ) delay (1);  // wait a little until the bouncing is done
  
  // Test transition, did things really change? 
  if( digitalRead(encoderPinB) != B_set ) 
  {  // debounce once more
    B_set = !B_set;
    
    //  adjust counter - 1 if B leads A
    if( B_set && !A_set ) 
    { 
      encoderPos -= 1; 
      lastRotatedTime = millis();
    }
    rotating = false;
  }
}  // rotatingCCW()



/*
void encoderSetup() {

  pinMode(encoderPinA, INPUT_PULLUP); 
  pinMode(encoderPinB, INPUT_PULLUP); 

// encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoderA, CHANGE);

// encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoderB, CHANGE);

} // encoderSetup()
*/

/*
// main loop, work is done by interrupt service routines, this one only prints stuff
void checkEncoder() { 
  rotating = true;  // reset the debouncer

  if (lastReportedPos != encoderPos) {
    Serial.print("Index:");
    Serial.println(encoderPos, DEC);
    lastReportedPos = encoderPos;
  }
} 
*/

/*

// Interrupt on A changing state
void doEncoderA()
{
  // debounce
  if ( rotating ) delay (1);  // wait a little until the bouncing is done

  // Test transition, did things really change? 
  if( digitalRead(encoderPinA) != A_set ) 
  {  // debounce once more
    A_set = !A_set;

    // adjust counter + if A leads B
    if ( A_set && !B_set ) 
      encoderPos += 1;

    rotating = false;  // no more debouncing until loop() hits again
  }
} //doEncoderA()

// Interrupt on B changing state, same as A above
void doEncoderB()
{
  if ( rotating ) delay (1);
  if( digitalRead(encoderPinB) != B_set ) 
  {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if( B_set && !A_set ) 
      encoderPos -= 1;

    rotating = false;
  }
} //doEncoderB()

*/



