#include <I2C.h>                         // http://dsscircuits.com/articles/arduino-i2c-master-library.html
#include "Speaker.h"
#include "Accel.h"


//  Pro-Mini pin assignments
#define ENC_A           2  // Encoder input (Interrupt)
#define PCBLEDBLU       3  // RGB LED on middle PCB Board to light up windows (PWM)
#define PUSHBTN         4  // Pushbutton in encoder
#define PCBLEDGRN       5  // RGB LED on middle PCB Board to light up windows (PWM)
#define PCBLEDRED       6  // RGB LED on middle PCB Board to light up windows (PWM)
#define SND_DATA        7  // Sound card data input
#define SND_BUSY        8  // Sound card busy output
#define ENCRED          9  // RGB LED in encoder (PWM)
#define ENCGRN         10  // RGB LED in encoder (PWM)
#define ENCBLU         11  // RGB LED in encoder (PWM)
#define ENC_B          12  // Encoder input (Interrupt)
#define OLED_RST       13  // OLED reset pin

         
#define MENU_BTN       0    // Analog 0, Menu pushbutton up 520, dn 693, select 1020
#define ALARM_ON_PIN   A1   // Alarm On/Off switch
#define SND_RST        A2   // Sound card Reset input
#define SND_CLK        A3   // Sound card clock input
// I2C SDA             A4
// I2C SCL             A5
//                     A6   // spare, can't be used as digital output.  Could use this for MENU_BTN input or SND_BUSY input if you want to free up a digital pin
// Photo sensor TBD    A7   // Future use if you want to have a light sensor in order to dim the display


#define PB_ON        HIGH   // Encoder pushbutton is on when HIGH
#define ALARM_ENABLED LOW   // Alarm is enabled when input is low
#define DISP_CLOCK      1   // display clock time on 7-segment display
#define DISP_ALARM      2   // display alarm time on 7-segment display
#define DISP_ALARM_OFF  3   // Alarm is off, display "OFF"

#define ADDR_ALM_HR     0   // EEPROM address for alarm Hour
#define ADDR_ALM_MN     1   // EEPROM address for alarm minute
#define ADDR_SOUND      2   // EEPROM address for alarm sound ID

#define I2C_TIMEOUT 30000  // 30 second I2C timeout

#define TARDIS      0x0000
#define THEMESONG   0x0001
#define EXTERMINATE 0x0002

ClockAccel Accel;
AlarmSpeaker Speaker(SND_CLK, SND_DATA, SND_RST, SND_BUSY);  


void setup()
{
  
  Serial.begin(9600);
  Serial.println("Accelerometer Smoothing Test");
  I2c.begin();             // Join the bus as a master
  I2c.timeOut(I2C_TIMEOUT);
  I2c.pullup(0);           // Disable internal pullups so I2C lines are not pulled up to 5 volts - accelerometer is not compatible
  Accel.begin();           // intialize the MMA8452 accelerometer
  Speaker.begin();         // Initialize sound card
  delay(1000);

  Speaker.play(TARDIS); 
  
}


void loop()
{
 
 uint32_t accelTime = millis();
 uint16_t smoothedAxis = Accel.smoothMaxAxis( 0.9, 15 );
 if (smoothedAxis > 20)
 { 
   Serial.print(smoothedAxis);
   Serial.print("\t");
   Serial.println(millis() - accelTime);
 }
  
  
}


