#include "Accel.h"


// Constructor
ClockAccel::ClockAccel()
{
}



// Destructor
ClockAccel::~ClockAccel()
{
}

// Returns true if accelerometer is moving.  
// You can set the threshold at which this occurs
// Note Vibrations from the speaker effect this
bool ClockAccel::isMoving(uint16_t threshold)
{
  int currentAccelVal[] = {0,0,0};  // Stores the 12-bit signed value
  int minAccelVal[] = {0,0,0};      // minimum value for accelerometer values
  int maxAccelVal[] = {0,0,0};      // maximum value for accelerometer values
  uint16_t maxAxisChange = 0;       // value of the axis with the maximum accelerometer value

  // Initialize min/max accelerometer values to the current values
  readAccelData(currentAccelVal);   
  for ( uint8_t i = 0; i < 3; i++ )
  { minAccelVal[i] = maxAccelVal[i] = currentAccelVal[i]; }

  // Take another accelerometer reading.  Accelerometer is a littls slow to
  // respond, so there is time for value to change between this read on the initial one. 
  readAccelData(currentAccelVal);   
  for ( uint8_t i = 0; i < 3; i++ )
    {
      if( currentAccelVal[i] < minAccelVal[i] )
      { minAccelVal[i] = currentAccelVal[i]; }
      if( currentAccelVal[i] > maxAccelVal[i] )
      { maxAccelVal[i] = currentAccelVal[i]; }
    }

  // see which axis had the most change
  for ( uint8_t i = 0; i < 3; i++ )
  {
    if ( abs(maxAccelVal[i] - minAccelVal[i]) >  maxAxisChange )
    { maxAxisChange = abs(maxAccelVal[i] - minAccelVal[i]); }
  }
/*
// srg debug output
Serial.print(maxAxisChange);
Serial.print("\t");
if(currentAccelVal[0] > -1) { Serial.print(" ");}
Serial.print(currentAccelVal[0]);
Serial.print("\t");
if(currentAccelVal[0] > -1) { Serial.print(" ");}
Serial.print(currentAccelVal[1]);
Serial.print("\t");
if(currentAccelVal[0] > -1) { Serial.print(" ");}
Serial.print(currentAccelVal[2]);
Serial.println();
*/
  // If any axis was greater then the threshold then we're moving
  if ( maxAxisChange > threshold )
  { return true; }
  else
  { return false; }

} // isMoving()


uint16_t ClockAccel::smoothMaxAxis(float filterVal, int numSamples )
{
  int currentAccelVal[] = {0,0,0};  // Stores the 12-bit signed value
  int filteredAccelVal[] = {0,0,0};   // filter accelerometer values
  uint16_t maxAxisChange = 0;       // value of the axis with the maximum accelerometer value
  uint16_t smoothedMaxChange = 0; 
  
  for(int k = 0; k < numSamples; k++)  // take multiple samples
  {
    readAccelData(currentAccelVal);   // Read accelerometer values
    // Filter the valuse.  Numbers added to currentAccelVal try to normalize to zero when still
    filteredAccelVal[0] = ( (currentAccelVal[0] +   48) * (1 - filterVal)) + (filteredAccelVal[0] * filterVal); 
    filteredAccelVal[1] = ( (currentAccelVal[1] -   15) * (1 - filterVal)) + (filteredAccelVal[1] * filterVal); 
    filteredAccelVal[2] = ( (currentAccelVal[2] - 1000) * (1 - filterVal)) + (filteredAccelVal[2] * filterVal); 
  }
  
  // see which axis had the most change
  for ( int i = 0; i < 3; i++ )
  {
    filteredAccelVal[i] = abs(filteredAccelVal[i]); // make everything positive
    if ( filteredAccelVal[i] > smoothedMaxChange )
    { smoothedMaxChange = filteredAccelVal[i]; }
  }
  
  return smoothedMaxChange;
  
} // smoothMaxAxis()



// Returns true if the accelerometer is upside down.  
bool ClockAccel::isUpsideDown(uint8_t mountingPosition)
{
  int accelCount[] = {0,0,0};  // Stores the 12-bit signed value
  readAccelData(accelCount);  // Read the x/y/z adc values

  switch( mountingPosition )
  {
    case 1: // horizantal
      if ( accelCount[2] < -700 )
      { return true; }
      else
      { return false; }
      break;
    case 2: // vertical pins on top 
      if ( accelCount[0] > 700 )
      { return true; }
      else
      { return false; }
      break;
    case 3: // vertical pins on bottom 
      if ( accelCount[0] < -700 )
      { return true; }
      else
      { return false; }
      break;
    case 4: // vertical pins on right side - if looking at top of PCB 
      if ( accelCount[1] > 700 )
      { return true; }
      else
      { return false; }
      break;
    case 5: // vertical pins on left side - if looking at top of PCB 
      if ( accelCount[1] < -700 )
      { return true; }
      else
      { return false; }
      break;
  }
}  // isUpsideDown



// Initialize the MMA8452 registers 
// See the many application notes for more info on setting all of these registers:
// http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=MMA8452Q
bool ClockAccel::begin()
{
  if ( readRegister(0x0D) != 0x2A ) // WHO_AM_I should always be 0x2A
  { return false; } // Could not connect to MMA8452Q

  MMA8452Standby();  // Must be in standby to change registers

  // Set up the full scale range to 2, 4, or 8g.
  writeRegister(0x0E, 0);  // 0 for 2G (2nd parameter)

  //The default data rate is 800Hz and we don't modify it in this example code
  MMA8452Active();  // Set to active to start reading
  return true;
} // begin()



// Sets the MMA8452 to standby mode. It must be in standby to change most register settings
void ClockAccel::MMA8452Standby()
{
  uint8_t c = readRegister(0x2A);
  writeRegister(0x2A, c & ~(0x01)); //Clear the active bit to go into standby
} // MMA8452Standby()



// Sets the MMA8452 to active mode. Needs to be in this mode to output data
void ClockAccel::MMA8452Active()
{
  uint8_t c = readRegister(0x2A);
  writeRegister(0x2A, c | 0x01); //Set the active bit to begin detection
} // MMA8452Active()



// Read bytesToRead sequentially, starting at addressToRead into the dest byte array
void ClockAccel::readRegisters(uint8_t addressToRead, uint8_t bytesToRead, uint8_t * dest)
{
  I2c.read(MMA8452_ADDRESS, addressToRead, bytesToRead, dest);
} // readRegisters()



// Read a single byte from addressToRead and return it as a byte
uint8_t ClockAccel::readRegister(uint8_t addressToRead)
{
  uint8_t getByte[1];
  I2c.read(MMA8452_ADDRESS, addressToRead, (uint8_t) 1, &getByte[0]);
  return getByte[0];
} // readRegister()



// Writes a single byte (dataToWrite) into addressToWrite
void ClockAccel::writeRegister(uint8_t addressToWrite, uint8_t dataToWrite)
{
  I2c.write(MMA8452_ADDRESS, addressToWrite, dataToWrite);
} // writeRegister()



void ClockAccel::readAccelData(int *destination)
{
  uint8_t rawData[6];  // x/y/z accel register data stored here

  readRegisters(0x01, 6, rawData);  // Read the six raw data registers into data array

  // Loop to calculate 12-bit ADC and g value for each axis
  for(int i = 0; i < 3 ; i++)
  {
    int gCount = (rawData[i*2] << 8) | rawData[(i*2)+1];  //Combine the two 8 bit registers into one 12-bit number
    gCount >>= 4; //The registers are left align, here we right align the 12-bit integer

    // If the number is negative, we have to make it so manually (no 12-bit data type)
    if ( rawData[i*2] > 0x7F )
    {  
      gCount = ~gCount + 1;
      gCount *= -1;  // Transform into negative 2's complement #
    }

    destination[i] = gCount; //Record this gCount into the 3 int array
  }
} // readAccelData()

