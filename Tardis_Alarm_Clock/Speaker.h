/*
Works with Spartkfun breakout board WTV020: http://www.sparkfun.com/products/11125
You can't control the volume, it's set to max before going to sparkfun


Got code from By Shigeru Kobayashi
Source: http://yapan.googlecode.com/svn/trunk/arduino/examples/SOMO_14D_Test/SOMO_14D_Test.pde
Control a SOMO-14D module to play sounds
Other References
http://www.4dsystems.com.au/downloads/Audio-Sound-Modules/SOMO-14D/Docs/SOMO-14D-Product-Brief-REV1.pdf
http://www.4dsystems.com.au/product/10/117/Development/SOMO_14D/


SD card should be 1 GB formatted as Fat 16
need to convert a wav file to 4-bit ADPCM @ 6-32kHz
one converter is WavePad: 22,050 Hz, 16 bit, Mono
Sparkfun audio converter: http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Widgets/USBRecordingNbk1.3.0.zip
files have to be named 0000.ad4,  0001.ad4 and shouldn't have sequental numbers with no gaps.  

To play a sound: Speaker_Play(0x001); will play 0001.ad4.  
Sometimes you many need a short delay after you start a song.

  
*/

#ifndef SPEAKER_H
#define SPEAKER_H

#include "Arduino.h"

// Index number for sounds
// Make sure the files on the MicroSD card are numbered sequentially without any missing numbers
#define TARDIS      0x0000
#define THEMESONG   0x0001
#define EXTERMINATE 0x0002
#define SCREWDRIVER 0x0011

#define BIP1 0x0003 // orig file 26953__xinaesthete__bip1.wav.  Good for menu selection - NOT USED
#define BIP2 0x0004 // orig file 26953__xinaesthete__bip2.wav. Volume too low - NOT USED
#define BIP3 0x0005 // orig file 141121__eternitys__interface1.wav.  Sounds a bit like a click on glass
#define SOFTWHISTLE 0x0006 // orig file 26955__xinaesthete__bsoft1.wav.  Might be good for select button
#define CLICK1 0x0007 // 180821__empty-bell__beep.wav Loud metallick click - NOT USED

//  These sounds don't sound good on alarm speaker
#define HELLO_CLARA 0x0008  // Hello There Clara.mp3, from Rings of Akhaten timecode 2:30 
#define CLARA       0x0009  // Clara.mp3, Matt Smith saying "Clara", from Bells of St John, timecode 19:22
#define GOODNIGHT   0x0010  // Goodnight Clara.mp3, Matt Smith saying Goodnight Clara, from Rings of Akhaten timecode 42.29


class AlarmSpeaker
{
  public:
    AlarmSpeaker(uint8_t clockPin, uint8_t dataPin, uint8_t resetPin, uint8_t busyPin);
    ~AlarmSpeaker(); // destructor
    void begin(); // setup I/O pins
    void stop(); // stop playing sound
    void pause(); // pause sound
    void play(unsigned int soundIndex); // start playing sound
    bool isBusy(); // See if module is playing a sound

  private:
    void sendCommand(unsigned int command);
    uint8_t resetPin_;   // the pin number of the reset pin
    uint8_t clockPin_;   // the pin number of the clock pin
    uint8_t dataPin_;    // the pin number of the data pin
    uint8_t busyPin_;    // Audio busy, high when a song is playing

};


#endif
