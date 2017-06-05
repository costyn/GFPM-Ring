/*
   Inspiration from: https://learn.adafruit.com/animated-neopixel-gemma-glow-fur-scarf

   By: Costyn van Dongen

   Future ideas:
   - color rain https://www.youtube.com/watch?v=nHBImYTDZ9I  (for strip, not ring)

   Note: MAX LEDS: 255 (due to use of uint8_t in for loops)

   TODO:
     Get rid of MPU interrupt stuff. I don't need it, but removing it without breaking shit is tricky.
*/

// Turn on microsecond resolution; needed to sync some routines to BPM
#define _TASK_MICRO_RES


#include <FastLED.h>
#include <TaskScheduler.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include <ArduinoTapTempo.h>


// Uncomment for debug output to Serial.
//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)       Serial.print (x)
#define DEBUG_PRINTDEC(x)    Serial.print (x, DEC)
#define DEBUG_PRINTLN(x)     Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#endif

#define CHIPSET     WS2812B
#define LED_PIN     12   // which pin your Neopixels are connected to
#define NUM_LEDS    60   // how many LEDs you have
#define MAX_BRIGHT  200  // 0-255, higher number is brighter. 
#define SATURATION  255   // 0-255, 0 is pure white, 255 is fully saturated color
#define BUTTON_PIN  3   // button is connected to pin 3 and GND
#define BUTTON_LED_PIN 5   // pin to which the button LED is attached
#define COLOR_ORDER GRB  // Try mixing up the letters (RGB, GBR, BRG, etc) for a whole new world of color combinations

#define LEDMODE_SELECT_DEFAULT_INTERVAL 50 // default scheduling time for LEDMODESELECT, in microseconds

CRGB leds[NUM_LEDS];
// unsigned long lastButtonChange = 0; // button debounce timer.

// BPM and button stuff
boolean longPressActive = false;
ArduinoTapTempo tapTempo;


#define RT_P_RB_STRIPE
/*
  #define RT_P_OCEAN
  #define RT_P_HEAT
  #define RT_P_LAVA
  #define RT_P_PARTY
  #define RT_P_FOREST
  #define RT_TWIRL1
  #define RT_TWIRL2
  #define RT_TWIRL4
  #define RT_TWIRL6
  #define RT_TWIRL2_O
  #define RT_TWIRL4_O
  #define RT_TWIRL6_O
  #define RT_FADE_GLITTER
  #define RT_DISCO_GLITTER
  #define RT_RACERS
  #define RT_WAVE
  #define RT_SHAKE_IT
  */
  
#define RT_PULSE2  // TODO
// #define RT_PULSE_STATIC  // TODO
//#define RT_STROBE1
//#define RT_STROBE2
//#define RT_VUMETER  // TODO
//#define RT_GLED
//#define RT_HEARTBEAT
//#define RT_FASTLOOP
//#define RT_FASTLOOP2
//#define RT_PENDULUM
//#define RT_BOUNCEBLEND
//#define RT_JUGGLE_PAL
//#define RT_NOISE_LAVA
//#define RT_NOISE_PARTY
//#define RT_BLACK

byte ledMode = 0 ; // Which mode do we start with

// Routine Palette Rainbow is always included - a safe routine
const char *routines[] = {
  "p_rb",
#ifdef RT_P_RB_STRIPE
  "p_rb_stripe",
#endif
#ifdef RT_P_OCEAN
  "p_ocean",
#endif
#ifdef RT_P_HEAT
  "p_heat",
#endif
#ifdef RT_P_LAVA
  "p_lava",
#endif
#ifdef RT_P_PARTY
  "p_party",
#endif
#ifdef RT_TWIRL1
  "twirl1",     // 4
#endif
#ifdef RT_TWIRL2
  "twirl2",     // 5
#endif
#ifdef RT_TWIRL4
  "twirl4",     // 6
#endif
#ifdef RT_TWIRL6
  "twirl6",     // 7
#endif
#ifdef RT_TWIRL2_O
  "twirl2o",    // 8
#endif
#ifdef RT_TWIRL4_O
  "twirl4o",    // 9
#endif
#ifdef RT_TWIRL6_O
  "twirl6o",    // 10
#endif
#ifdef RT_FADE_GLITTER
  "fglitter",   // 11
#endif
#ifdef RT_DISCO_GLITTER
  "dglitter",   // 12
#endif
#ifdef RT_RACERS
  "racers",     // 13
#endif
#ifdef RT_PULSE2
  "pulse2",     // 13
#endif
#ifdef RT_PULSE_STATIC
  "pulsestatic",     // 13
#endif
#ifdef RT_WAVE
  "wave",       // 14
#endif
#ifdef RT_SHAKE_IT
  "shakeit",    // 15
#endif
#ifdef RT_STROBE1
  "strobe1",    // 16
#endif
#ifdef RT_STROBE2
  "strobe2",    // 17
#endif
#ifdef RT_GLED
  "gled",       // 18
#endif
#ifdef RT_HEARTBEAT
  "heartbeat",  // 19
#endif
#ifdef RT_FASTLOOP
  "fastloop",   // 20
#endif
#ifdef RT_FASTLOOP2
  "fastloop2",  // 21
#endif
#ifdef RT_PENDULUM
  "pendulum",   // 22
#endif
#ifdef RT_VUMETER
  "vumeter",   // 22
#endif
#ifdef RT_NOISE_LAVA
  "noise_lava", // 23
#endif
#ifdef RT_NOISE_PARTY
  "noise_party", // 23
#endif
#ifdef RT_BOUNCEBLEND
  "bounceblend", // 23
#endif
#ifdef RT_JUGGLE_PAL
  "jugglepal",
#endif
#ifdef RT_BLACK
  "black", // 23
#endif

};

#define NUMROUTINES (sizeof(routines)/sizeof(char *)) //array size  


/* Scheduler stuff */
#define LEDMODE_SELECT_DEFAULT_INTERVAL 50000 // default scheduling time for LEDMODESELECT, in microseconds
void ledModeSelect() ; // prototype method
Scheduler runner;
Task taskLedModeSelect( LEDMODE_SELECT_DEFAULT_INTERVAL, TASK_FOREVER, &ledModeSelect); // routine which adds/removes tasks according to ledmode

#define TASK_CHECK_BUTTON_PRESS_INTERVAL 10000   // in microseconds
void checkButtonPress() ; // prototype method
Task taskCheckButtonPress( TASK_CHECK_BUTTON_PRESS_INTERVAL, TASK_FOREVER, &checkButtonPress); // routine which adds/removes tasks according to ledmode


// ==================================================================== //
// ===                      MPU6050 stuff                         ===== //
// ==================================================================== //


// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

MPU6050 mpu;

#define INTERRUPT_PIN 2  // use pin 2 on Arduino Uno & most boards

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer


int aaRealX = 0 ;
int aaRealY = 0 ;
int aaRealZ = 0 ;
int yprX = 0 ;
int yprY = 0 ;
int yprZ = 0 ;

void getDMPData() ; // prototype method
Task taskGetDMPData( 3000, TASK_FOREVER, &getDMPData);


#ifdef DEBUG
void printDebugging() ; // prototype method
Task taskPrintDebugging( 100, TASK_FOREVER, &printDebugging);
#endif




void setup() {
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  MAX_BRIGHT );
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LED_PIN, OUTPUT);
  digitalWrite(BUTTON_LED_PIN, HIGH);

  //  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), shortKeyPress, RISING);

  set_max_power_in_volts_and_milliamps(5, 1500);

#ifdef DEBUG
  Serial.begin(115200) ;
  DEBUG_PRINT( F("Starting up. Numroutines = ")) ;
  DEBUG_PRINTLN( NUMROUTINES ) ;

#endif

  /* Start the scheduler */
  runner.init();
  runner.addTask(taskLedModeSelect);
  taskLedModeSelect.enable() ;

  runner.addTask(taskCheckButtonPress);
  taskCheckButtonPress.enable() ;


  // ==================================================================== //
  // ==================================================================== //

  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  mpu.initialize();
  devStatus = mpu.dmpInitialize();

  // Ring MPU
  // -287  4 1560  -29 -4  15
  // -381  209 1476  -24 0 -23
  mpu.setXAccelOffset(-381 );
  mpu.setYAccelOffset(209);
  mpu.setZAccelOffset(1476);
  mpu.setXGyroOffset(-24);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(-23);

  if (devStatus == 0) {
    mpu.setDMPEnabled(true);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();
    dmpReady = true;
    packetSize = mpu.dmpGetFIFOPacketSize();

  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    DEBUG_PRINT(F("DMP Initialization failed (code "));
    DEBUG_PRINT(devStatus);
    DEBUG_PRINTLN(F(")"));
  }

  runner.addTask(taskGetDMPData);
  taskGetDMPData.enable() ;

#ifdef DEBUG
  //  runner.addTask(taskPrintDebugging);
  //  taskPrintDebugging.enable() ;
#endif


}  // end setup()




void loop() {
  runner.execute();
}




void ledModeSelect() {

  if ( strcmp(routines[ledMode], "p_rb") == 0  ) {
    FillLEDsFromPaletteColors(0) ;

#ifdef RT_P_RB_STRIPE
  } else if ( strcmp(routines[ledMode], "p_rb_stripe") == 0 ) {
    FillLEDsFromPaletteColors(1) ;
#endif

#ifdef RT_P_OCEAN
  } else if ( strcmp(routines[ledMode], "p_ocean") == 0 ) {
    FillLEDsFromPaletteColors(2) ;
#endif

#ifdef RT_P_HEAT
  } else if ( strcmp(routines[ledMode], "p_heat") == 0 ) {
    FillLEDsFromPaletteColors(3) ;
#endif

#ifdef RT_P_LAVA
  } else if ( strcmp(routines[ledMode], "p_lava") == 0 ) {
    FillLEDsFromPaletteColors(4) ;
#endif

#ifdef RT_P_PARTY
  } else if ( strcmp(routines[ledMode], "p_party") == 0 ) {
    FillLEDsFromPaletteColors(5) ;
#endif

#ifdef RT_P_CLOUD
  } else if ( strcmp(routines[ledMode], "p_cloud") == 0 ) {
    FillLEDsFromPaletteColors(6) ;
#endif

#ifdef RT_P_FOREST
  } else if ( strcmp(routines[ledMode], "p_forest") == 0 ) {
    FillLEDsFromPaletteColors(7) ;
#endif

#ifdef RT_TWIRL1
  } else if ( strcmp(routines[ledMode], "twirl1") == 0 ) {
    twirlers( 1, false ) ;
#endif

#ifdef RT_TWIRL2
  } else if ( strcmp(routines[ledMode], "twirl2") == 0 ) {
    twirlers( 2, false ) ;
#endif

#ifdef RT_TWIRL4
  } else if ( strcmp(routines[ledMode], "twirl4") == 0 ) {
    twirlers( 4, false ) ;
#endif

#ifdef RT_TWIRL6
  } else if ( strcmp(routines[ledMode], "twirl6") == 0 ) {
    twirlers( 6, false ) ;
#endif

#ifdef RT_TWIRL2_O
  } else if ( strcmp(routines[ledMode], "twirl2o") == 0 ) {
    twirlers( 2, true ) ;
#endif

#ifdef RT_TWIRL4_O
  } else if ( strcmp(routines[ledMode], "twirl4o") == 0 ) {
    twirlers( 4, true ) ;
#endif

#ifdef RT_TWIRL6_O
  } else if ( strcmp(routines[ledMode], "twirl6o") == 0 ) {
    twirlers( 6, true ) ;
#endif

#ifdef RT_FADE_GLITTER
  } else if ( strcmp(routines[ledMode], "fglitter") == 0 ) {
    fadeGlitter() ;
    taskLedModeSelect.setInterval( map( constrain( activityLevel(), 0, 4000), 0, 4000, 20, 5 ) * 1000 ) ;
#endif

#ifdef RT_DISCO_GLITTER
  } else if ( strcmp(routines[ledMode], "dglitter") == 0 ) {
    discoGlitter() ;
    taskLedModeSelect.setInterval( map( constrain( activityLevel(), 0, 2500), 0, 2500, 40, 2 ) * 1000 ) ;
#endif

#ifdef RT_GLED
    // Gravity LED
  } else if ( strcmp(routines[ledMode], "gled") == 0 ) {
    gLed() ;
    taskLedModeSelect.setInterval( 5000 ) ;
#endif

#ifdef RT_BLACK
  } else if ( strcmp(routines[ledMode], "black") == 0 ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    taskLedModeSelect.setInterval( 500 * 1000 ) ;  // long because nothing is going on anyways.
#endif

#ifdef RT_RACERS
  } else if ( strcmp(routines[ledMode], "racers") == 0 ) {
    racingLeds() ;
    taskLedModeSelect.setInterval( 8 * 1000) ;
#endif

#ifdef RT_WAVE
  } else if ( strcmp(routines[ledMode], "wave") == 0 ) {
    waveYourArms() ;
    taskLedModeSelect.setInterval( 15 * 1000) ;
#endif

#ifdef RT_SHAKE_IT
  } else if ( strcmp(routines[ledMode], "shakeit") == 0 ) {
    shakeIt() ;
    taskLedModeSelect.setInterval( 10 * 1000 ) ;
#endif

#ifdef RT_STROBE1
  } else if ( strcmp(routines[ledMode], "strobe1") == 0 ) {
    strobe1() ;
    taskLedModeSelect.setInterval( 5 * 1000 ) ;
#endif

#ifdef RT_STROBE2
  } else if ( strcmp(routines[ledMode], "strobe2") == 0 ) {
    strobe2() ;
    taskLedModeSelect.setInterval( 10 * 1000 ) ;
#endif

#ifdef RT_HEARTBEAT
  } else if ( strcmp(routines[ledMode], "heartbeat") == 0 ) {
    heartbeat() ;
#endif

#ifdef RT_VUMETER
  } else if ( strcmp(routines[ledMode], "vumeter") == 0 ) {
    vuMeter() ;
    taskLedModeSelect.setInterval( 8 * 1000) ;
#endif

#ifdef RT_FASTLOOP
  } else if ( strcmp(routines[ledMode], "fastloop") == 0 ) {
    fastLoop( false ) ;
#endif

#ifdef RT_FASTLOOP2
  } else if ( strcmp(routines[ledMode], "fastloop2") == 0 ) {
    fastLoop( true ) ;
    taskLedModeSelect.setInterval( 10 * 1000) ;
#endif

#ifdef RT_PENDULUM
  } else if ( strcmp(routines[ledMode], "pendulum") == 0 ) {
    pendulum() ;
    taskLedModeSelect.setInterval( 1500 ) ; // needs a fast refresh rate
#endif

#ifdef RT_BOUNCEBLEND
  } else if ( strcmp(routines[ledMode], "bounceblend") == 0 ) {
    bounceBlend() ;
    taskLedModeSelect.setInterval( 10 * 1000) ;
#endif

#ifdef RT_JUGGLE_PAL
  } else if ( strcmp(routines[ledMode], "jugglepal") == 0 ) {
    jugglePal() ;
    taskLedModeSelect.setInterval( 850 ) ; // fast refresh rate needed to not skip any LEDs
#endif

#ifdef RT_NOISE_LAVA
  } else if ( strcmp(routines[ledMode], "noise_lava") == 0 ) {
    fillnoise8( 0, beatsin8( tapTempo.getBPM(), 1, 25), 30, 1); // pallette, speed, scale, loop
    taskLedModeSelect.setInterval( 10 * 1000 ) ;
#endif

#ifdef RT_NOISE_PARTY
  } else if ( strcmp(routines[ledMode], "noise_party") == 0 ) {
    fillnoise8( 1, beatsin8( tapTempo.getBPM(), 1, 25), 30, 1); // pallette, speed, scale, loop
//    taskLedModeSelect.setInterval( beatsin16( tapTempo.getBPM(), 2000, 50000) ) ;
    taskLedModeSelect.setInterval( 10 * 1000 ) ;
#endif
  }

}

