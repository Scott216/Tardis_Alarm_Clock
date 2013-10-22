#include "OledMenu.h"


// Initialize OLED Display
void Menu::init(Adafruit_SSD1306& oled)
{
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  oled.clearDisplay();   
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  Menu::writeToDisplay(oled, "*-*-*-*-*", "", 10, 9, 0, 0);
  delay(700);
  oled.clearDisplay();   
  oled.display();
} // init()



// If a menu button is pressed, then the main sketch will call this function to 
// and will get the menu process started
int8_t Menu::changeSettings(Adafruit_SSD1306& oled, uint8_t clockHr, uint8_t clockMin, uint8_t alarmHr, uint8_t alarmMin)
{
  int8_t btnPressed; 

  // Default time values for menu to display
  oledClockHour =   clockHr; 
  oledClockMinute = clockMin; 
  oledAlarmHour =   alarmHr;
  oledAlarmMinute = alarmMin;

  Speaker.play(SOFTWHISTLE); // Sound to play when menu is started

  // Turn on menu  
  menuSelection = 0; // Set menu to first choice - Set Alarm
  Menu::displayTopLevel(oled, menuSelection);
  
  // Program stays in this loop until finished with menu
  do
  {
    btnPressed = Menu::PollKey();  // wait for menu button to be pressed
    
    // No activity, menu timed out.  Exit menu process
    if (btnPressed == KEY_TIMEOUT)
    {
      Menu::finished(oled);
      return NOCHANGE;
    }
    
    // If no timeout, then a key was pressed
    if (btnPressed != KEY_TIMEOUT)
    {
      // Choose setting to change
      menuSelection = menuSelection + menuChange;
      if (menuSelection >= 3) {menuSelection = 0;}  // manage rollover
      if (menuSelection < 0 ) {menuSelection = 2;}  // manage rollover
      Menu::displayTopLevel(oled, menuSelection);
  
      // Selection was made, call function to change setting
      if (btnPressed == KEYSEL)
      {
        bool isNewSetting; // result flag for a new setting, true if new setting was completed
        switch (menuSelection)
        {
          case 0:  // Set Alarm 
            isNewSetting = Menu::setNewTime(oled, SETALARMTIME); 
            Menu::finished(oled);  // display departing message and turn off OLED
            if (isNewSetting )
            { return NEWALARM; }  // finished setting a alarm new time
            else
            { return NOCHANGE; } // setting new alarm time didn't finish
            break; 
            
          case 1: // Set Clock Time
            isNewSetting = Menu::setNewTime(oled, SETCLOCKTIME); 
            Menu::finished(oled);  // display departing message and turn off OLED
            if (isNewSetting )
            { return NEWTIME; }  // finished setting a clock time
            else
            { return NOCHANGE; } // setting new clock time didn't finish
            break;
            
          case 2: // Set Sound
            isNewSetting = Menu::setNewSound(oled);
            Menu::finished(oled);  // display departing message and turn off OLED
            return NEWSOUND; 
            break; 
        } // switch menuSelection
      } // if BtnPressed == KEYSEL
    } // if wasBtnPressed
  
  } while (btnPressed != KEY_TIMEOUT);
  return NOCHANGE;
} // Menu::changeSettings()



// OLED displays top level menu - scroll through the three choices
void Menu::displayTopLevel(Adafruit_SSD1306& oled, uint8_t selection)
{
  switch (selection)
  {
    case 0:
      Menu::writeToDisplay(oled, "Set", "Alarm", 0, 0, 0, 18);
      break;
    case 1:
      Menu::writeToDisplay(oled, "Set", "Clock", 0, 0, 0, 18);
       break;
    case 2:
      Menu::writeToDisplay(oled, "Change", "Sound", 0, 0, 0, 18);
      break;
  }
}  // Menu::DisplayTopLevel()



// Set time, either alarm or clock.  For alarm time, alarmTime parameter is true, for clcok time this is false
// function returns true if time was set, false if menu timed out
bool Menu::setNewTime(Adafruit_SSD1306& oled, bool alarmTime)
{
  const bool SET_HOURS = true;
  const bool SET_MINUTES = false;
  int8_t newHr, newMin; 
  
  if (alarmTime == SETALARMTIME)
  { // Change alarm time
    newHr = oledAlarmHour;
    newMin = oledAlarmMinute;
  }
  else 
  { // change clock time
    newHr = oledClockHour;
    newMin = oledClockMinute;
  }
  
  Menu::displayTime(oled, newHr, newMin, SET_HOURS);  // display on OLED current time to be changed. Last parameter is true if changing hours, false for minutes
  
  //--------------------------  
  // Change hour
  int8_t btnPressed = NOKEY;  
  do
  {
    btnPressed = Menu::PollKey(); // wait for menu button to be pressed
    newHr += menuChange;  
    if (newHr > 23) 
    { newHr = 0; }
    if (newHr < 0) 
    { newHr = 23; }
    Menu::displayTime(oled, newHr,  newMin, SET_HOURS); 

  } while (btnPressed != KEYSEL && btnPressed != KEY_TIMEOUT);
  
  // Menu timed out, get out of here
  if (btnPressed == KEY_TIMEOUT)
  {return false;}
  
  //--------------------------  
  // Change Minutes
  Menu::displayTime(oled, newHr, newMin, SET_MINUTES);  // display current time to be changed
  btnPressed = NOKEY;  
  do
  {
    btnPressed = Menu::PollKey(); // wait for menu button to be pressed
    newMin += menuChange;  
    if (newMin > 59) 
    { newMin = 0; }
    if (newMin < 0) 
    { newMin = 59; }
    Menu::displayTime(oled, newHr,  newMin, SET_MINUTES); 

  } while (btnPressed != KEYSEL && btnPressed != KEY_TIMEOUT);
 
  // Menu timed out, get out of here
  if (btnPressed == KEY_TIMEOUT)
  { return false;}
 
  // Finished changing hours and minutes
  // Update hour and minutes variables
  // these can be called by Menu::GetHour() and Menu::GetMinute() functions so main sketch can update the time
  if (alarmTime == SETALARMTIME)
  { // update the alarm time
    oledAlarmHour = newHr;
    oledAlarmMinute = newMin;
    Menu::writeToDisplay(oled, "New alarm", "time set", 0, 0, 0, 18);
  }
  else 
  { // update the clock time
    oledClockHour = newHr;
    oledClockMinute = newMin;
    Menu::writeToDisplay(oled, "New clock", "time set", 0, 0, 0, 18);
  }
  return true;
} // Menu::setNewTime()



// Displays the change time screen.  
// chaningHours Parameter is true if chaning hours, false if changing minutes
void Menu::displayTime(Adafruit_SSD1306& oled, int8_t newHour, int8_t newMinute, bool changingHours)
{
  uint8_t newHour12;
  char timebuf[10]; // character buffer for time
  char buf[3]; // character buffer for hours and minutes
  
  if (newHour == 0)
  { newHour12 = 12; }  
  else if (newHour > 12)
  { newHour12 = newHour - 12; } 
  else
  { newHour12 = newHour; }

  itoa(newHour12, buf, 10);
  if (newHour12 < 10)
  { 
    strcpy(timebuf, "0"); 
    strcat(timebuf, buf); 
  }
  else
  { strcpy(timebuf, buf); }
  
  strcat(timebuf, ":");  
  itoa(newMinute, buf, 10);
  if (newMinute < 10)
  { strcat(timebuf, "0");}
  strcat(timebuf, buf);
  
  if (newHour < 12)
  { strcat(timebuf, " AM"); }
  else
  { strcat(timebuf, " PM"); }
  
  if (changingHours)
  { Menu::writeToDisplay(oled, "Change Hr", timebuf, 0, 0, 13, 18); }
  else
  { Menu::writeToDisplay(oled, "Change Min", timebuf, 0, 0, 13, 18); }

} // Menu::displayTime()



// Pick a new alarm sound
// Return true if a new sound is selected
bool Menu::setNewSound(Adafruit_SSD1306& oled)
{
  int8_t soundMenuID = -2; // ID number of sound to choose.  Default to -2 and skip Pollkey() so display updates first time routine is run
  char soundbuf[11];  // character array to hold name of sound
  int8_t btnPressed = NOKEY;  
  
  // Loop until Select key is pressed
  do
  {
    if (soundMenuID != -2) // want display to update first time without waiting for key entry
    { btnPressed = Menu::PollKey(); } // wait for menu button to be pressed
    soundMenuID += menuChange;
    if (soundMenuID > 2) 
    { soundMenuID = 0; }
    if (soundMenuID < 0) 
    { soundMenuID = 2; }

    oled.clearDisplay();
    oled.setCursor(0,0);
    oled.println("Pick Sound"); 

    switch (soundMenuID)
    {
      case 0: // Exterminate
        Menu::writeToDisplay(oled, "Pick Sound", "Exterminat", 0, 0, 0, 18);
        soundIndex = SOUND_EXTERMINATE;
        break; 
      case 1: // Theme song
        Menu::writeToDisplay(oled, "Pick Sound", "Theme Song", 0, 0, 0, 18);
        soundIndex = SOUND_THEME;
        break; 
      case 2: // Tardis
        Menu::writeToDisplay(oled, "Pick Sound", "Tardis", 0, 0, 0, 18);
        soundIndex = SOUND_TARDIS;
        break; 
    }
  } while (btnPressed != KEYSEL && btnPressed != KEY_TIMEOUT);
  
  // Menu timed out, get out of here
  if (btnPressed == KEY_TIMEOUT)
  {return false;}
  
  switch (soundIndex)
  {
    case SOUND_EXTERMINATE: 
      Menu::writeToDisplay(oled, "You Picked", "Exterminat", 0, 0, 0, 18);
      break;
    case SOUND_THEME: 
      Menu::writeToDisplay(oled, "You Picked", "Theme Song", 0, 0, 0, 18);
      break;
    case SOUND_TARDIS: 
      Menu::writeToDisplay(oled, "You Picked", "Tardis", 0, 0, 0, 18);
      break;
  }
  return true;
} // Menu::setNewSound()



// Returns the current sound ID number
uint8_t Menu::getSoundId()  
{ 
  return soundIndex; 
}



// Finished with menu, display see ya text and shut off OLED
void Menu::finished(Adafruit_SSD1306& oled)
{
  delay(2000);
  Menu::writeToDisplay(oled, "See ya", "", 20, 9, 0, 0);
  delay(1000);
  oled.clearDisplay();   
  oled.display();
}



// Writes 2 lines to the OLED display
void Menu::writeToDisplay(Adafruit_SSD1306& disp, char line1[], char line2[], uint8_t Line1_x, uint8_t Line1_y, uint8_t Line2_x, uint8_t Line2_y )
{
  disp.clearDisplay();
  disp.setCursor(Line1_x, Line1_y);
  disp.println(line1); 
  // If Line2_y is not > 0, assume there is nothing to print for line 2.  
  if (Line2_y > 0)
  {
    disp.setCursor(Line2_x, Line2_y);
    disp.println(line2); 
  }
  disp.display();
}  // Menu::WriteToDisplay()



// Return the clock hour 
uint8_t Menu::getClockHour()
{ return oledClockHour;}




// Return the clock minutes 
uint8_t Menu::getClockMinute()
{ return oledClockMinute;}



// Return the Alarm hour 
uint8_t Menu::getAlarmHour()
{ return oledAlarmHour;}



// Return the Alarm minutes 
uint8_t Menu::getAlarmMinute()
{ return oledAlarmMinute;}



// Scan the key pin and determine which key was pressed. 
// Function can return NOKEY, KEYSEL, KEYDOWN, KEYUP
uint8_t Menu::KeyScan() 
{
   uint16_t which, which2, diff;
   uint8_t retVal;
   menuChange = 0; // Reset menu change
   which = analogRead(MENU_BTN);
   delay(20);
   which2 = analogRead(MENU_BTN);
   retVal = NOKEY;  // defualt 
   diff = abs(which - which2);
   
   if (diff < 12) 
   {
      if (which < 20) // Select button
      { 
        menuChange = 0;        
        retVal = KEYSEL;
      }
      
      if (which > 300 && which < 350)   // Down button
      {  
        menuChange = -1;        
        retVal = KEYDOWN;
      }
      
      if (which > 470 && which < 570) // Up button 
      { 
        menuChange = 1;        
        retVal = KEYUP;
      }
   }
   return retVal;
} // Menu::KeyScan()



// Repeatedly calls KeyScan until a valid key press occurs or timeout is reached
// Can retun: KEY_TIMEOUT, KEYSEL, KEYDOWN, KEYUP
int8_t Menu::PollKey() 
{
  uint8_t Whichkey;
  uint32_t menuBtnTmr = millis() + 15000;  // initialize timeout timer
  bool btnTimeOut = false;  // Turns true if menu times out because of no buttons being pressed
  
  do 
  {
     Whichkey = Menu::KeyScan();
     delay(30);
     if ((long) (millis() - menuBtnTmr) > 0)
     { btnTimeOut = true; }
  } while (Whichkey == NOKEY && btnTimeOut == false );

  if(btnTimeOut)
  { 
    menuChange = 0;
    return KEY_TIMEOUT;
  }
  
  Speaker.play(BIP3); // make little beep when buttons are pressed
  delay(80);
  return Whichkey;
} // Menu::PollKey()





