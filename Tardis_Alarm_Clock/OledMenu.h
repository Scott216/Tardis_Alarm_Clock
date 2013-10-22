/*
  Using namespace for the menu
  http://www.cprogramming.com/tutorial/namespaces.html


*/
#ifndef OLED_MENU_H
#define OLED_MENU_H

#include "Arduino.h"
#include "Speaker.h"

#include <SSD1306_I2C_DSS.h>  // http://github.com/Scott216/SSD1306_I2C_DSS

static uint8_t oledClockHour; 
static uint8_t oledClockMinute; 
static uint8_t oledAlarmHour;
static uint8_t oledAlarmMinute;

static uint8_t soundIndex;   // ID number of active alarm sound
static int8_t menuChange;    // -1 for down button, +1 for up, zero for select
static int8_t menuSelection; // Determines which item in menu user has selected

extern AlarmSpeaker Speaker;

#define KEYUP       10
#define KEYDOWN     20
#define KEYSEL      30
#define NOKEY       40 // No button pressed
#define KEY_TIMEOUT 50 // Menu times out because of no activity
#define MENU_BTN     0 // Menu pushbutton input
#define SETCLOCKTIME false
#define SETALARMTIME true


// Return codes forMenu_ChangeSettings
#define NOCHANGE  0
#define NEWTIME   1
#define NEWALARM  2
#define NEWSOUND  3

// Sound file indexes 
#define SOUND_TARDIS 0
#define SOUND_THEME 1
#define SOUND_EXTERMINATE 2

// For kicks set this up as a namespace
namespace Menu
{
// Function Prototypes
void    init(Adafruit_SSD1306& oled);
int8_t  changeSettings(Adafruit_SSD1306& oled, uint8_t clockHr, uint8_t clockMin, uint8_t alarmHr, uint8_t alarmMin);
bool    setNewTime(Adafruit_SSD1306& oled, bool alarmTime);
void    displayTopLevel(Adafruit_SSD1306& oled, uint8_t selection);
void    displayTime(Adafruit_SSD1306& oled, int8_t newHour, int8_t newMinute, bool changingHours);
void    writeToDisplay(Adafruit_SSD1306& oled, char line1[], char line2[], uint8_t Line1_x, uint8_t Line1_y, uint8_t Line2_x, uint8_t Line2_y);
void    finished(Adafruit_SSD1306& oled);

bool    setNewSound(Adafruit_SSD1306& oled);
uint8_t getSoundId();  // Returns the current sound ID number

uint8_t getClockHour();
uint8_t getClockMinute();

uint8_t getAlarmHour();
uint8_t getAlarmMinute();

int8_t  PollKey();
uint8_t KeyScan();
}

#endif
