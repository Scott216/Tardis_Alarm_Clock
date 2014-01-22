To Do:
Test for changes made to encoder.cpp and .h. on 1/21/14 I moved A_set, B_set, rotating, encoderPinA, encoderPinB into .h file, protected section. 
 
Functionality:
Time is displayed on LCD, there is no AM/PM indication.
OLED 128x32 screen on the bottom for menu. Here you can set time/alarm and sounds. To activate OLED menu, press any pushbutton on bottom

Pressing encoder pushbutton on top will display the alarm time in military time if the alarm is enabled.  If it's disabled it will display "OFF"

When alarm comes on, sound plays from speaker and RGB LED in encoder will pulse. 
To turn alarm off you can press or knob on top or shaking the tardis.


To Do:
Snooze function - maybe


PCB changes:
Add level shifter for I2C
Test shake switch
Add a 2nd RGB LED to PCB
Add 5-pin jet for USB cable
Put 2nd interrupts on encoder since you'll free up 3 PWM pins


================
Accelerometer
================
Accelerometer Wiring  
MMA8452-------- Arduino
3.3V --------------- 3.3V
SDA ----------------- A4
SCL ----------------- A5
GND ----------------- GND
 
Because the accelerometer uses 3.3 volts and Arduino is 5 volts, disable internal pull-ups on I2C pins 

The MMA8452 Accelerometer is 3.3V so we recommend using 330 or 1k resistors between a 5V Arduino and the MMA8452 breakout.
The breakout board has 10k pull-up resistors to 3.3 volts for I2C so you do not need additional pull-ups.
Source: http://github.com/sparkfun/MMA8452_Accelerometer/tree/master/Firmware/MMA8452Q_BasicExample
Sparkfun link: http://www.sparkfun.com/products/10955
If you keep having I2C problems, you may want use this analog output accelerometer: http://www.adafruit.com/products/163 
 
============
Encoder
============
Sparkfun RGB rotary encoder with switch: http://www.sparkfun.com/products/10982
Breakout board: http://www.sparkfun.com/products/11722
Source for code: http://arduino.cc/playground/Main/RotaryEncoders#Example15
Pins B/C/A are encoder output 

========== 
RTC
==========



========== 
Sound card
==========
Sparkfun WTV020SD breakout http://www.sparkfun.com/products/11125
Audio Amp: Digikey SSM2305RMZ-REEL7CT-ND or 706-1168-1-ND

SD card should be 1 GB formatted as Fat 16
need to convert a wav file to 4-bit ADPCM @ 6-32kHz
one converter is WavePad: 22,050 Hz, 16 bit, Mono
Sparkfun audio converter: http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Widgets/USBRecordingNbk1.3.0.zip
files have to be named 0000.ad4,  0001.ad4 etc.
play file using sendCommand(0x0001);





