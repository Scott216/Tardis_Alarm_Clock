/*
Location: 
cd Dropbox/Arduino/Tardis_Alarm_Clock
  
 
Change Log
09/23/14  v2.00 - Added version
08/11/15  v2.01 - Changed LED pinouts.  LEDs on center board now use use same pins as LED in encoder handle. Changed ENC_B pin
                  Pressing encoder pushbutton will display alarm time on OLED (in addition to 7-seg display)
05/23/16  v2.02 - Changed accelerometer threshold from 300 to 600
05/25/16  v2.03 - Added #define SCREWDRIVER in Speaker.h
*/

#define VERSION "v2.02"

#include <EEPROM.h>
#include <I2C.h>                         // http://dsscircuits.com/articles/arduino-i2c-master-library.html
#include <RTClib_DSSI2C.h>               // http://github.com/Scott216/RTCLib_wo_wire
#include <Adafruit_LEDBackpack_I2cLib.h> // http://github.com/adafruit/Adafruit-LED-Backpack-Library modified by SRG to use I2C.h insead of wire.h
#include <Adafruit_GFX.h>                // http://github.com/adafruit/Adafruit-GFX-Library
#include <SSD1306_I2C_DSS.h>             // http://github.com/Scott216/SSD1306_I2C_DSS
#include "Speaker.h"
#include "OledMenu.h"
#include "Accel.h"
#include "Encoder.h"


// This gets rid of compiler warning: Only initialized variables can be placed into program memory area
#undef PROGMEM
#define PROGMEM __attribute__(( section(".progmem.data") ))

//  Pro-Mini pin assignments
#define ENC_A           2  // Encoder input (Interrupt)
#define ENC_B           3  // Encoder input (Interrupt)
#define PUSHBTN         4  // Pushbutton in encoder
#define SHAKE           5  // Shake sensor (future use)
#define SND_DATA        7  // Sound card data input
#define SND_BUSY        8  // Sound card busy output
#define LED_RED         9  // RGB LEDs on middle PCB Board and encoder pushbutton (PWM)
#define LED_GRN        10  // RGB LEDs on middle PCB Board and encoder pushbutton (PWM)
#define LED_BLU        11  // RGB LEDs on middle PCB Board and encoder pushbutton (PWM)
#define OLED_RST       13  // OLED reset pin


#define MENU_BTN       A0   // Analog 0, Menu pushbutton up 520, dn 693, select 1020
#define ALARM_ON_PIN   A1   // Alarm On/Off switch
#define SND_RST        A2   // Sound card reset input
#define SND_CLK        A3   // Sound card clock input
// I2C SDA             A4
// I2C SCL             A5
//                     A6   // Spare, can't be used as digital output.  Could use this for MENU_BTN input or SND_BUSY input if you want to free up a digital pin
// Photo sensor TBD    A7   // Future use if you want to have a light sensor in order to dim the display


#define PB_ON        HIGH   // Encoder pushbutton is on when HIGH
#define ALARM_ENABLED LOW   // Alarm is enabled when input is low
#define DISP_CLOCK      1   // Display clock time on 7-segment display
#define DISP_ALARM      2   // Display alarm time on 7-segment display
#define DISP_ALARM_OFF  3   // Alarm is off, display "OFF"

#define ADDR_ALM_HR     0   // EEPROM address for alarm Hour
#define ADDR_ALM_MN     1   // EEPROM address for alarm minute
#define ADDR_SOUND      2   // EEPROM address for alarm sound ID

#define I2C_TIMEOUT 30000  // 30 second I2C timeout



// create objects
RTC_DS1307 RTC;
ClockAccel Accel;
AlarmSpeaker Speaker(SND_CLK, SND_DATA, SND_RST, SND_BUSY);  
Adafruit_SSD1306 display(OLED_RST, I2C_TIMEOUT);
Adafruit_7segment led_display = Adafruit_7segment();
DateTime now;
Encoder rotEncoder;


uint8_t alarmHour;
uint8_t alarmMinute;
uint8_t snoozeHour;
uint8_t snoozeMinute;
uint8_t soundId;      // Current sound ID to use for alarm
bool isAlarmSilenced; // Keeps alarm from going off after user silenced it
uint16_t ACCEL_THRESHOLD = 600;  // Accelerometer threshold, the hight the number the harder you have to shake to turn off alarm

// Function Prototypes
void updateLcdDisplay(uint8_t whichTimeToShow);
void startMenu();
void setAlarm(uint8_t alarmH, uint8_t alarmM);
void pulseRGBLEDs(bool ledsAreOn);

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
  Serial.print("Version ");
  Serial.println(VERSION);
  
  pinMode(PUSHBTN, INPUT);              // Rotory encoder pushbutton, needs a PULL-DOWN resistor, P/B is on when HIGH
  pinMode(ALARM_ON_PIN, INPUT_PULLUP);  // Alarm enable switch
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GRN, OUTPUT);
  pinMode(LED_BLU, OUTPUT);
  pinMode(SHAKE, INPUT_PULLUP);
  

  I2c.begin();             // Join the bus as a master
  I2c.timeOut(I2C_TIMEOUT);
  I2c.pullup(0);           // Disable internal pullups so I2C lines are not pulled up to 5 volts - accelerometer is not compatible
  Accel.begin();           // intialize the MMA8452 accelerometer
  RTC.begin();             // Initialize real time clock
  Speaker.begin();         // Initialize sound card
  led_display.begin(0x70); // Initialize 7-segment LED display
  Menu::init(display);     // Initialize the OLED display (using namespace)
  rotEncoder.begin(ENC_A, ENC_B); // Initialize rotary encoder

  // following line sets the RTC to the date & time this sketch was compiled
//  RTC.adjust(DateTime(__DATE__, __TIME__));
  
  // Read alarm time from EEPROM and set variables
  alarmHour = EEPROM.read(ADDR_ALM_HR);
  alarmMinute = EEPROM.read(ADDR_ALM_MN);
  
  // Validate alarm values
  if (alarmHour > 23)
  { 
    alarmHour = 8; 
    EEPROM.write(ADDR_ALM_HR, alarmHour);
  }
  if (alarmMinute > 59 )
  { 
    alarmMinute = 0;
    EEPROM.write(ADDR_ALM_MN, alarmMinute);
  }
  
  
  // Read sound ID from EEPROM  
  soundId = EEPROM.read(ADDR_SOUND);
  
  isAlarmSilenced = false;  // flag to tell when user silenced alarm
  pulseRGBLEDs(false);      // turn LEDs off

}  // setup()


//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
void loop()
{  
  static bool isAlarmSouding; 
  static uint32_t dispRefreshTimer = millis() + 1000; // Initialize 7seg refresh timer
  
  // Once a second update LCD display with current time
  if ( (long)(millis() - dispRefreshTimer) > 0 )
  { 
    now = RTC.now(); // update time
    updateLcdDisplay(DISP_CLOCK); 
    dispRefreshTimer = millis() + 1000;
    
  }

  // Check for menu button press
  if ( Menu::KeyScan() != NOKEY )
  { startMenu(); } 


  // If encoder pushbutton is pressed and alarm is off, then
  // display alarm time  
  if ( digitalRead(PUSHBTN) == PB_ON && Speaker.isBusy() == false )
  {
    if ( digitalRead(ALARM_ON_PIN) == ALARM_ENABLED ) 
    { updateLcdDisplay(DISP_ALARM); }      // alarm is enabled, display alarm time
    else
    { updateLcdDisplay(DISP_ALARM_OFF); }  // alarm is disabled, display "OFF"
    Menu::displayAlarmTime(display);  // displays time on OLED
    delay(2000); 
  }
  
  // check for alarm, it will stay on for one minute if not silenced
  if ( now.hour() == alarmHour && now.minute() == alarmMinute && isAlarmSilenced == false && digitalRead(ALARM_ON_PIN) == ALARM_ENABLED )
  {
    Serial.println("Alarm has triggered");  //srgg
    Serial.println(Speaker.isBusy());
    Serial.println(soundId);
    
    if(Speaker.isBusy() == false)
    {
      Speaker.play(soundId); 
      delay(50);
      rotEncoder.resetValue();  // Reset encoder value. Will need to reset if you ever use encoder to determine snooze time
    }
    isAlarmSouding = true;
  }

  // Silence alarm
  // User can turn alarm off by:
  //   1 Encoder pushbutton
  //   2 Turn encoder knob
  //   3 Shake Tardis
  //   4 Turn off alarm switch on bottom
  // isMovoing has a threshold parameter.  The higher it is the harder you need to shake alarm
  // rotEncoder.isTurning() takes mS as a paremeter.  For example 100mS would return true if knob was turned in the last 100 mS
  if ( isAlarmSouding == true )
  {
    if ( digitalRead(PUSHBTN) == PB_ON ||  Accel.isMoving(ACCEL_THRESHOLD) || rotEncoder.isTurning(100) || digitalRead(ALARM_ON_PIN) != ALARM_ENABLED )  
    {
      Speaker.stop();  // turn off sound
      isAlarmSilenced = true; // flag to prevent alarm from coming right back on 
      isAlarmSouding = false;
      pulseRGBLEDs(false);
    } 
    else
    { pulseRGBLEDs(true); }
  }   

  // After alarm time has passed, turn alarm off (if it's on) and reset flag 
  if ( now.hour() != alarmHour || now.minute() != alarmMinute )
  { 
    // If the alarm is still on, then turn it off
    if( Speaker.isBusy() == true )
    { Speaker.stop(); }
    isAlarmSilenced = false; 
    isAlarmSouding = false;
    pulseRGBLEDs(false);
  }
  
} // end loop()



//--------------------------------------------------------------------------------------------------------
// Update LCD display with the clock time or alarm time or "OFF"
//--------------------------------------------------------------------------------------------------------
void updateLcdDisplay(uint8_t whichTimeToShow)
{
  int dispTime;             // This is an integer with same digits as the time.  If time is 11:30, dispTime = 1130
  uint8_t brightness;       // Used to dim the display
  uint8_t dispHr, dispMin;  // holds either clock or alarm hours and minutes


  // Choose what to display on the LCD
  switch (whichTimeToShow)
  {
    case DISP_CLOCK: 
      // display clock time
      dispHr = now.hour();
      dispMin = now.minute();
      break;
    case DISP_ALARM:
      // need to get alarm time from global variables or EEPROM
      dispHr = alarmHour;
      dispMin = alarmMinute;
      break;
    case DISP_ALARM_OFF:  // Alarm is turned off, don't show alarm time, instead show
      // don't do anything here
      break;
  }
  
  // Set brightness based on hour of the day
  // Range 0 - 15
  if ( now.hour() >= 20 || now.hour() <= 6 )
  { brightness = 2; }  // Night time
  else
  { brightness = 15; } // Daytime
  
  // Format the time
  if( dispHr == 0 )
  { dispTime = 1200 + dispMin; }  // Midnight
  else if ( dispHr > 12 && whichTimeToShow != DISP_ALARM )
  { dispTime = (dispHr - 12) * 100 + dispMin; }  // PM
  else if ( dispHr > 12 && whichTimeToShow == DISP_ALARM )
  { dispTime = dispHr * 100 + dispMin; }  // PM, but display Alarm in military time
  else
  { dispTime = dispHr * 100 + dispMin; }  // AM (between 1:00 AM and noon)
  led_display.clear();
  led_display.setBrightness(brightness);

  if( whichTimeToShow == DISP_ALARM_OFF )
  {
    // Alarm is off, display "OFF"
    led_display.writeDigitNum(1, 0x0, false);  
    led_display.writeDigitNum(3, 0xF, false);
    led_display.writeDigitNum(4, 0xF, false);
    led_display.drawColon(false); 
  }
  else
  {
    // Show either Time or Alarm time
    led_display.print(dispTime);   // Send time to display
    led_display.drawColon(true);   // Turn on colon between hour and minutes
  }
  led_display.writeDisplay();     // refresh display with the time  
  
}  // end updateLcdDisplay


//--------------------------------------------------------------------------------------------------------
// Start config menu that runs on OLED screen on bottom
//--------------------------------------------------------------------------------------------------------
void startMenu()
{
  DateTime now = RTC.now();

  
  // Start menu process
  // need to pass the alarm time and clock time so the menu knows what to display if either is changed
  int8_t menuResponse = Menu::changeSettings(display, now.hour(), now.minute(), alarmHour, alarmMinute);
  switch (menuResponse)
  {
    case NOCHANGE:  // No changes were made with menu
      break;
    case NEWTIME:  // Uptdate time in RTC
      RTC.adjust(DateTime(now.year(), now.month(), now.day(), Menu::getClockHour(), Menu::getClockMinute(), 1 ));
      break;
    case NEWALARM:  // Change alarm time
      setAlarm(Menu::getAlarmHour(), Menu::getAlarmMinute());
      isAlarmSilenced = false; // reset alarm flag
      break;
    case NEWSOUND:  // changed alarm sound
      soundId = Menu::getSoundId();
      EEPROM.write(ADDR_SOUND, Menu::getSoundId());
      break;
    default:
      break;
  } // switch()

} // startMenu()


// Sets the global alarm variables alarmHour, alarmMinute
// Also sets stores the alarm time in EEPROM
// Alarm time is saved in military time
void setAlarm(uint8_t alarmH, uint8_t alarmM)
{
  // Set global variables
  alarmHour = alarmH;
  alarmMinute = alarmM; 
  
  // write to EEPROM
  EEPROM.write(ADDR_ALM_HR, alarmHour);
  EEPROM.write(ADDR_ALM_MN, alarmMinute);
} // setAlarm()



// Pulse RGB light in Encoder know and on PCB
// LED in encoder is off when PWM is 255
// LED in PCB is off when PWM is 255
void pulseRGBLEDs(bool ledsAreOn)
{
  const uint8_t LEDOFF =  0;
  const uint8_t LEDON = 255;
  
  static int countAmount;
  static uint8_t redPWMValue;
  static uint8_t bluPWMValue;
  static uint8_t grnPWMValue;

  if ( ledsAreOn )
  { // Adjust PWM amount 
    redPWMValue = redPWMValue + countAmount;
    bluPWMValue = bluPWMValue + countAmount;
    grnPWMValue = grnPWMValue + countAmount;
    if ( bluPWMValue > 250 )
    { countAmount = -5; }
    if ( bluPWMValue < 70 )
    { countAmount = 5; }
  }
  else
  { // Turn LEDs off
    countAmount = 5;
    redPWMValue = LEDOFF;
    bluPWMValue = LEDOFF;
    grnPWMValue = LEDOFF;
  } 

  // Make LED blue by turning off red and green
  redPWMValue = LEDOFF;
  grnPWMValue = LEDOFF;

  analogWrite(LED_RED, redPWMValue);
  analogWrite(LED_GRN, grnPWMValue);
  analogWrite(LED_BLU, bluPWMValue);
  
}  // end pulseRGBLEDs()



