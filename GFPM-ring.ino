/*
   Heavily modified from https://learn.adafruit.com/animated-neopixel-gemma-glow-fur-scarf

   By: Costyn van Dongen

   Future ideas:
   - color rain https://www.youtube.com/watch?v=nHBImYTDZ9I

   Note: MAX LEDS: 255 (due to use of uint8_t in for loops)
*/

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
#define STEPS       3   // How wide the bands of color are.  1 = more like a gradient, 10 = more like stripes
#define BUTTON_PIN  3   // button is connected to pin 3 and GND
#define BUTTON_LED_PIN 5   // pin to which the button LED is attached
#define COLOR_ORDER GRB  // Try mixing up the letters (RGB, GBR, BRG, etc) for a whole new world of color combinations
#define LOOPSTART 0

#define LEDMODE_SELECT_DEFAULT_INTERVAL 50 // default scheduling time for LEDMODESELECT
#define PALETTE_SPEED  15                 // Default How fast the palette colors move.   Higher delay = slower movement.
#define FIRE_SPEED  85                    // Default Fire Speed; delay in millseconds. Higher delay = slower movement.
#define CYLON_SPEED 13                    // Default Cylon Speed; delay in millseconds. Higher delay = slower movement.
#define FADEGLITTER_SPEED 10              // Default delay in millseconds. Higher delay = slower movement.
#define DISCOGLITTER_SPEED 20             // Default delay in millseconds. Higher delay = slower movement.

//#define WHITESTRIPE
#ifdef WHITESTRIPE
#define WHITESTRIPE_SPEED 5   // how fast white stripe goes
#endif

CRGB leds[NUM_LEDS];
// unsigned long lastButtonChange = 0; // button debounce timer.
boolean longPressActive = false;

ArduinoTapTempo tapTempo;


byte ledMode = 0 ; // Which mode do we start with

const char *routines[] = {
  "rb",         // 0
  "ocean",      // 1
  "heat",       // 2
  "party",      // 3
  "twirl1",     // 4
  "twirl2",     // 5
  "twirl4",     // 6
  "twirl6",     // 7
  "twirl2o",    // 8
  "twirl4o",    // 9
  "twirl6o",    // 10
  "fglitter",   // 11
  "dglitter",   // 12
  "racers",     // 13
  "wave",       // 14
  "shakeit",    // 15
  "strobe1",    // 16
  "strobe2",    // 17
  "gled",       // 18
  "heartbeat",  // 19
  "fastloop",   // 20
  "fastloop2",  // 21
  "pendulum",   // 22
  "noise_lava", // 23
};
#define NUMROUTINES (sizeof(routines)/sizeof(char *)) //array size  


/* Scheduler stuff */
void ledModeSelect() ; // prototype method
Scheduler runner;
Task taskLedModeSelect( LEDMODE_SELECT_DEFAULT_INTERVAL, TASK_FOREVER, &ledModeSelect); // routine which adds/removes tasks according to ledmode

#ifdef WHITESTRIPE
void whiteStripe() ; // prototype method
Task taskWhiteStripe( WHITESTRIPE_SPEED, TASK_FOREVER, &whiteStripe); // routine which adds/removes tasks according to ledmode
#endif



//#define _TASK_SLEEP_ON_IDLE_RUN

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
//void checkButtonPress() ; // prototype method
Task taskGetDMPData( 3000, TASK_FOREVER, &getDMPData);
// Task taskCheckButtonPress( 50, TASK_FOREVER, &checkButtonPress);

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

  //runner.addTask(taskCheckButtonPress);
  //taskCheckButtonPress.enable() ;


#ifdef WHITESTRIPE
  runner.addTask(taskWhiteStripe);
  taskWhiteStripe.enable() ;
#endif

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

  /*
    mpu.setXAccelOffset(-287);
    mpu.setYAccelOffset(-4);
    mpu.setZAccelOffset(1560);
    mpu.setXGyroOffset(29);
    mpu.setYGyroOffset(-4);
    mpu.setZGyroOffset(15);
  */

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

#ifdef DEBUG
  //  runner.addTask(taskPrintDebugging);
  //  taskPrintDebugging.enable() ;
#endif

  //  tapTempo.setMaxBPM( 180 ) ;
  //  tapTempo.setMinBPM( 90 ) ;



}  // end setup()



#define HEARTBEAT_INTERVAL 1000  // 1 second

void loop() {
  /*
    #ifdef DEBUG
    static long lastHeartbeat = millis() ;
    static unsigned int heartBeatNumber = 0 ;

    if ( millis() - lastHeartbeat > HEARTBEAT_INTERVAL ) {
      DEBUG_PRINT(F(".")) ;
      DEBUG_PRINTLN(heartBeatNumber) ;
      lastHeartbeat = millis() ;
      heartBeatNumber++ ;
    }
    #endif
  */
  runner.execute();

  checkButtonPress() ;
}



void ledModeSelect() {

  if ( ledMode >= 0 and ledMode <= 3 ) {
    FillLEDsFromPaletteColors() ;
    taskLedModeSelect.setInterval( beatsin8( 30, 5000, 25000) ) ;
//    taskLedModeSelect.setInterval( 25000 ) ;
    taskGetDMPData.enableIfNot() ;

  } else if ( strcmp(routines[ledMode], "twirl1") == 0 ) {
    twirlers( 1, false ) ;
    taskGetDMPData.enableIfNot() ;

  } else if ( strcmp(routines[ledMode], "twirl2") == 0 ) {
    twirlers( 2, false ) ;

  } else if ( strcmp(routines[ledMode], "twirl4") == 0 ) {
    twirlers( 4, false ) ;

  } else if ( strcmp(routines[ledMode], "twirl6") == 0 ) {
    twirlers( 6, false ) ;

  } else if ( strcmp(routines[ledMode], "twirl2o") == 0 ) {
    twirlers( 2, true ) ;

  } else if ( strcmp(routines[ledMode], "twirl4o") == 0 ) {
    twirlers( 4, true ) ;

  } else if ( strcmp(routines[ledMode], "twirl6o") == 0 ) {
    twirlers( 6, true ) ;

    // Fade glitter
  } else if ( strcmp(routines[ledMode], "fglitter") == 0 ) {
    fadeGlitter() ;
    //    taskLedModeSelect.setInterval( FADEGLITTER_SPEED ) ;
    taskLedModeSelect.setInterval( map( constrain( activityLevel(), 0, 4000), 0, 4000, 20, 5 ) * 1000 ) ;
    taskGetDMPData.enableIfNot() ;
    FastLED.setBrightness( MAX_BRIGHT ) ;


    //  Disco glitter
  } else if ( strcmp(routines[ledMode], "dglitter") == 0 ) {
    discoGlitter() ;
    taskLedModeSelect.setInterval( map( constrain( activityLevel(), 0, 2500), 0, 2500, 40, 2 ) * 1000 ) ;
    taskGetDMPData.enableIfNot() ;
    FastLED.setBrightness( MAX_BRIGHT ) ;


    // Gravity LED
  } else if ( strcmp(routines[ledMode], "gled") == 0 ) {
    gLed() ;
    taskGetDMPData.enableIfNot() ;
    taskLedModeSelect.setInterval( 5000 ) ;

    // Black - off
  } else if ( strcmp(routines[ledMode], "black") == 0 ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    taskLedModeSelect.setInterval( 500 * 1000 ) ;  // long because nothing is going on anyways.
    taskGetDMPData.disable() ;

    /*
      } else if ( strcmp(routines[ledMode], "racers") == 0 ) {
        racingLeds() ;
        taskLedModeSelect.setInterval( 8 * 1000) ;
        taskGetDMPData.disable() ;
        FastLED.setBrightness( MAX_BRIGHT ) ;

      } else if ( strcmp(routines[ledMode], "wave") == 0 ) {
        waveYourArms() ;
        taskGetDMPData.enableIfNot() ;
        taskLedModeSelect.setInterval( 15 * 1000) ;
    */
  } else if ( strcmp(routines[ledMode], "shakeit") == 0 ) {
    shakeIt() ;
    taskLedModeSelect.setInterval( 10 * 1000 ) ;
    taskGetDMPData.enableIfNot() ;
    FastLED.setBrightness( MAX_BRIGHT ) ;

  } else if ( strcmp(routines[ledMode], "strobe2") == 0 ) {
    strobe2() ;
    taskLedModeSelect.setInterval( 10 * 1000 ) ;
    taskGetDMPData.enableIfNot() ;

  } else if ( strcmp(routines[ledMode], "strobe1") == 0 ) {
    strobe1() ;
    taskLedModeSelect.setInterval( 5 * 1000 ) ;
    taskGetDMPData.enableIfNot() ;

/*

  } else if ( strcmp(routines[ledMode], "heartbeat") == 0 ) {
    heartbeat() ;
    //    taskLedModeSelect.setInterval( 10 * 1000) ;
    taskGetDMPData.enableIfNot() ;

*/
    /*
      } else if ( strcmp(routines[ledMode], "vumeter") == 0 ) {
        vuMeter() ;
        taskLedModeSelect.setInterval( 8 * 1000) ;
        taskGetDMPData.disable() ;
    */

  } else if ( strcmp(routines[ledMode], "fastloop") == 0 ) {
    fastLoop( false ) ;
    //    taskLedModeSelect.setInterval( 5 * 1000) ;
    taskGetDMPData.enableIfNot() ;

  } else if ( strcmp(routines[ledMode], "fastloop2") == 0 ) {
    fastLoop( true ) ;
    taskLedModeSelect.setInterval( 10 * 1000) ;
    taskGetDMPData.disable() ;

  } else if ( strcmp(routines[ledMode], "pendulum") == 0 ) {
    pendulum() ;
    taskLedModeSelect.setInterval( 10 * 1000) ;
    taskGetDMPData.enableIfNot() ;

  } else if ( strcmp(routines[ledMode], "noise_lava") == 0 ) {
    fillnoise8( 0, 20, 30, 1); // pallette, speed, scale, loop
    taskLedModeSelect.setInterval( 10 * 1000) ;
    taskGetDMPData.disable() ;
    /*
      } else if ( strcmp(routines[ledMode], "noise_party") == 0 ) {
        fillnoise8( 1, 20, 30, 1); // pallette, speed, scale, loop
        taskLedModeSelect.setInterval( 10 * 1000 ) ;
        taskGetDMPData.disable() ;
    */
  }

}

