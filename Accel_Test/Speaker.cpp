#include "Speaker.h"

AlarmSpeaker::AlarmSpeaker(uint8_t clockPin, uint8_t dataPin, uint8_t resetPin, uint8_t busyPin)
{ // Make pin numbers availabe to class
  clockPin_ = clockPin;
  dataPin_  = dataPin;
  resetPin_ = resetPin;
  busyPin_  = busyPin;
}



// Initialize speaker pins.  Don't do this in constructor, locks up Arduino
void AlarmSpeaker::begin()
{
  pinMode(clockPin_, OUTPUT);
  pinMode(dataPin_,  OUTPUT);
  pinMode(resetPin_, OUTPUT);
  pinMode(busyPin_,   INPUT);
    
  digitalWrite(clockPin_, HIGH);
  digitalWrite(dataPin_,   LOW);
  
  // reset the module
  digitalWrite(resetPin_, HIGH);
  delay(100);
  digitalWrite(resetPin_,  LOW);
  delay(10);
  digitalWrite(resetPin_, HIGH);
  delay(100); 
}



void AlarmSpeaker::stop()
{
  AlarmSpeaker::sendCommand(0xFFFF);
} 



void AlarmSpeaker::pause()
{
  AlarmSpeaker::sendCommand(0xFFFE);
} 



// play sound file
void AlarmSpeaker::play(unsigned int soundIndex)
{
  AlarmSpeaker::sendCommand(soundIndex); 
  delay(25);
} 



bool AlarmSpeaker::isBusy()
{
  if (digitalRead(busyPin_) == LOW )
  { return false; }
  else
  { return true; } 
} 



void AlarmSpeaker::sendCommand(unsigned int command) 
{
  // start bit
  digitalWrite(clockPin_, LOW);
  delay(2);

  // bit15, bit14, ... bit0
  for (unsigned int mask = 0x8000; mask > 0; mask >>= 1) 
  {
    if (command & mask) 
    { digitalWrite(dataPin_, HIGH); }
    else 
    { digitalWrite(dataPin_, LOW);}
    
    // clock low
    digitalWrite(clockPin_, LOW);
    delayMicroseconds(200);

    // clock high
    digitalWrite(clockPin_, HIGH);
    delayMicroseconds(200);
  }
  // stop bit
  delay(2);
} 



AlarmSpeaker::~AlarmSpeaker() // destructor
{} 

