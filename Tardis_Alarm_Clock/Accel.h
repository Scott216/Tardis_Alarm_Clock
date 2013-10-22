#ifndef ACCEL_H
#define ACCEL_H

#include <Arduino.h>
#include <I2C.h>

// The SparkFun breakout board defaults to 1, set to 0 if SA0 jumper on the bottom of the board is set
const uint8_t MMA8452_ADDRESS = 0x1D;  // 0x1D (29 dec) if SA0 is high, 0x1C if low

class ClockAccel
{
  public:
    ClockAccel();
    ~ClockAccel();
    bool begin();
    void readAccelData(int *destination);
    bool isMoving(uint16_t threshold);
    bool isUpsideDown(uint8_t mountingPosition);

  protected:
    void MMA8452Standby();
    void MMA8452Active();
    void readRegisters(uint8_t addressToRead, uint8_t bytesToRead, uint8_t * dest);
    uint8_t readRegister(uint8_t addressToRead);
    void writeRegister(uint8_t addressToWrite, uint8_t dataToWrite);
};

#endif // ACCEL_H
