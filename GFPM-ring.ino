/*
   Heavily modified from https://learn.adafruit.com/animated-neopixel-gemma-glow-fur-scarf

   By: Costyn van Dongen

   Future ideas:
   - choose 1 color, brightenall to max, then fade to min
   - heartbeat pulse
   - color rain https://www.youtube.com/watch?v=nHBImYTDZ9I
   - two "faders" moving back and forth
*/

#include <FastLED.h>
#include <TaskScheduler.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>

// Uncomment for debug output to Serial.
// #define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)       Serial.print (x)
#define DEBUG_PRINTDEC(x)    Serial.print (x, DEC)
#define DEBUG_PRINTLN(x)     Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#endif

#define LED_PIN     12   // which pin your Neopixels are connected to
#define NUM_LEDS    60   // how many LEDs you have
#define MAX_BRIGHT  150  // 0-255, higher number is brighter. 
#define SATURATION  255   // 0-255, 0 is pure white, 255 is fully saturated color
#define STEPS       2   // How wide the bands of color are.  1 = more like a gradient, 10 = more like stripes
#define BUTTON_PIN  3   // button is connected to pin 3 and GND
#define COLOR_ORDER GRB  // Try mixing up the letters (RGB, GBR, BRG, etc) for a whole new world of color combinations
#define LOOPSTART 0

#define LEDMODE_SELECT_DEFAULT_INTERVAL 50 // default scheduling time for LEDMODESELECT
#define PALETTE_SPEED  15                 // Default How fast the palette colors move.   Higher delay = slower movement.
#define FIRE_SPEED  85                    // Default Fire Speed; delay in millseconds. Higher delay = slower movement.
#define CYLON_SPEED 20                    // Default Cylon Speed; delay in millseconds. Higher delay = slower movement.
#define FADEGLITTER_SPEED 10              // Default delay in millseconds. Higher delay = slower movement.
#define DISCOGLITTER_SPEED 20             // Default delay in millseconds. Higher delay = slower movement.

//#define WHITESTRIPE
#ifdef WHITESTRIPE
#define WHITESTRIPE_SPEED 5   // how fast white stripe goes
#endif

CRGB leds[NUM_LEDS];
unsigned long lastButtonChange = 0; // button debounce timer.


byte ledMode = 23 ; // Which mode do we start with

const char *routines[] = {
  "rb",         // 0
  "rb_stripe",  // 1
  "ocean",      // 2
  "heat",       // 3
  "party",      // 4
  "cloud",      // 5
  "forest",     // 6
  "twirl1",     // 7
  "twirl2",     // 8
  "twirl3",     // 9
  "twirl4",     // 10
  "twirl5",     // 11
  "twirl6",     // 12
  "fglitter",   // 16
  "dglitter",   // 17
  "pulse2",     // 18
  "pulsestatic",// 19
  "racers",     // 20
  "wave",       // 21
  "shakeit",    // 22
  "strobe2",    // 23
  "gled",       // 24
  "black"       // 25
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
void getYPRAccel() ; // prototype method
Task taskGetDMPData( 3, TASK_FOREVER, &getDMPData);
Task taskGetYPRAccel( 10, TASK_FOREVER, &getYPRAccel);

#ifdef DEBUG
void printDebugging() ; // prototype method
Task taskPrintDebugging( 100, TASK_FOREVER, &printDebugging);
#endif






void setup() {
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  MAX_BRIGHT );
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), shortKeyPress, RISING);

#ifdef DEBUG
  Serial.begin(115200) ;
  DEBUG_PRINT( F("Starting up. Numroutines = ")) ;
  DEBUG_PRINTLN( NUMROUTINES ) ;

#endif

  /* Start the scheduler */
  runner.init();
  runner.addTask(taskLedModeSelect);
  taskLedModeSelect.enable() ;

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
  mpu.setXAccelOffset(-287);
  mpu.setYAccelOffset(-4);
  mpu.setZAccelOffset(1560);
  mpu.setXGyroOffset(29);
  mpu.setYGyroOffset(-4);
  mpu.setZGyroOffset(15);

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
  runner.addTask(taskGetYPRAccel);
  taskGetYPRAccel.enable() ;

#ifdef DEBUG
  //  runner.addTask(taskPrintDebugging);
  //  taskPrintDebugging.enable() ;
#endif

}



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
}





void ledModeSelect() {

  if ( ledMode >= 0 and ledMode <= 6 ) {
    FillLEDsFromPaletteColors() ;
    taskLedModeSelect.setInterval( PALETTE_SPEED ) ;
    taskGetDMPData.enableIfNot() ;
#ifdef WHITESTRIPE
    taskWhiteStripe.enableIfNot() ;
#endif
    // Uses setBrightness


  } else if ( ledMode >= 7 and ledMode <= 13 ) {
    twirlers( ledMode - 6 ) ;
    taskLedModeSelect.setInterval( CYLON_SPEED ) ;
    taskGetDMPData.enableIfNot() ;
#ifdef WHITESTRIPE
    taskWhiteStripe.enableIfNot() ;
#endif

    // FastLED Fire2012 split down the middle, so the fire flows "down" from the neck of the scarf to the ends
  } else if ( strcmp(routines[ledMode], "fire2012") == 0 ) {
    Fire2012() ;
    FastLED.setBrightness( MAX_BRIGHT ) ;
    //    taskLedModeSelect.setInterval( FIRE_SPEED ) ;
#define FIRE_MAX_SPEED 15  // lower value for faster
#define FIRE_MIN_SPEED 100
    taskLedModeSelect.setInterval( map( yprZ, 0, 90, FIRE_MIN_SPEED, FIRE_MAX_SPEED )) ;
    taskGetDMPData.enableIfNot() ;
#ifdef WHITESTRIPE
    taskWhiteStripe.enableIfNot() ;
#endif

    
    // Fade glitter
  } else if ( strcmp(routines[ledMode], "fglitter") == 0 ) {
    fadeGlitter() ;
    //    taskLedModeSelect.setInterval( FADEGLITTER_SPEED ) ;
    taskLedModeSelect.setInterval( map( constrain( activityLevel(), 0, 4000), 0, 4000, 20, 5 )) ;
    taskGetDMPData.enableIfNot() ;
    FastLED.setBrightness( MAX_BRIGHT ) ;
#ifdef WHITESTRIPE
    taskWhiteStripe.enableIfNot() ;
#endif


    //  Disco glitter
  } else if ( strcmp(routines[ledMode], "dglitter") == 0 ) {
    discoGlitter() ;
    //    taskLedModeSelect.setInterval( DISCOGLITTER_SPEED ) ;
    taskLedModeSelect.setInterval( map( constrain( activityLevel(), 0, 4000), 0, 4000, 40, 5 )) ;
    taskGetDMPData.enableIfNot() ;
    FastLED.setBrightness( MAX_BRIGHT ) ;
#ifdef WHITESTRIPE
    taskWhiteStripe.enableIfNot() ;
#endif


    // With thanks to Hans for the strobe idea https://www.tweaking4all.nl/hardware/arduino/adruino-led-strip-effecten/#strobe
  } else if ( strcmp(routines[ledMode], "strobe") == 0 ) {
    //    setInterval is done in the subroutine itself
    strobe( 0, 10 ) ;
    taskGetDMPData.enableIfNot() ;
#ifdef WHITESTRIPE
    taskWhiteStripe.enableIfNot() ;
#endif

  } else if ( strcmp(routines[ledMode], "flashbpm") == 0 ) {
    strobe( 130, 2 ) ;
    taskGetDMPData.enableIfNot() ;
    FastLED.setBrightness( MAX_BRIGHT ) ;
#ifdef WHITESTRIPE
    taskWhiteStripe.enableIfNot() ;
#endif

  } else if ( strcmp(routines[ledMode], "gled") == 0 ) {
    gLed() ;
    taskGetDMPData.enableIfNot() ;
    
    
#ifdef WHITESTRIPE
    taskWhiteStripe.disable() ;
#endif

    // Black - off
  } else if ( strcmp(routines[ledMode], "black") == 0 ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    taskLedModeSelect.setInterval( 500 ) ;  // long because nothing is going on anyways.
    taskGetDMPData.disable() ;
#ifdef WHITESTRIPE
    taskWhiteStripe.disable() ;
#endif

    /*

      } else if ( strcmp(routines[ledMode], "pulse") == 0 ) {
        pulse() ;
        taskLedModeSelect.setInterval( 15 ) ;
        taskGetDMPData.disable() ;

    */
  } else if ( strcmp(routines[ledMode], "pulse2") == 0 ) {
    pulse2() ;
    taskLedModeSelect.setInterval( 10 ) ;
    taskGetDMPData.disable() ;


  } else if ( strcmp(routines[ledMode], "pulsestatic") == 0 ) {
    pulse_static() ;
    taskLedModeSelect.setInterval( 8 ) ;
    taskGetDMPData.disable() ;


  } else if ( strcmp(routines[ledMode], "racers") == 0 ) {
    racingLeds() ;
    taskLedModeSelect.setInterval( 8 ) ;
    taskGetDMPData.disable() ;
    FastLED.setBrightness( MAX_BRIGHT ) ;

  } else if ( strcmp(routines[ledMode], "wave") == 0 ) {
    waveYourArms() ;
    taskGetDMPData.enableIfNot() ;
    taskLedModeSelect.setInterval( 15 ) ;

  } else if ( strcmp(routines[ledMode], "shakeit") == 0 ) {
    shakeIt() ;
    taskLedModeSelect.setInterval( 10 ) ;
    taskGetDMPData.enableIfNot() ;
    FastLED.setBrightness( MAX_BRIGHT ) ;
#ifdef WHITESTRIPE
    taskWhiteStripe.disable() ;
#endif

  } else if ( strcmp(routines[ledMode], "strobe2") == 0 ) {
    strobe2() ;
    taskLedModeSelect.setInterval( 10 ) ;
    taskGetDMPData.enableIfNot() ;
#ifdef WHITESTRIPE
    taskWhiteStripe.disable() ;
#endif

  }
}




// interrupt triggered button press with a very simple debounce (discard multiple button presses < 300ms)
void shortKeyPress() {
  if ( millis() - lastButtonChange > 300 ) {
    ledMode++;
    DEBUG_PRINT(F("ledMode = ")) ;
    DEBUG_PRINT( routines[ledMode] ) ;
    DEBUG_PRINT(F(" mode ")) ;
    DEBUG_PRINTLN( ledMode ) ;

    if (ledMode >= NUMROUTINES ) {
      ledMode = 0;
    }

    lastButtonChange = millis() ;
  } else {
    //    DEBUG_PRINTLN(F("Too short an interval") ) ;
  }
}



