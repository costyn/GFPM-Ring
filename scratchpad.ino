/*
const char *routines[] = {
  "rb",         // 0
  //  "rb_stripe",  // 1
  "ocean",      // 1
  "heat",       // 2
  "party",      // 3
  //  "cloud",      // 5
  //  "forest",     // 6
  "twirl1",     // 7
  "twirl2",     // 8
  "twirl4",     // 9
  "twirl6",     // 10
  "twirl2o",    // 11
  "twirl4o",    // 12
  "twirl6o",    // 13
  "fglitter",   // 14
  "dglitter",   // 15
  //  "pulse2",     // 16
  //  "pulsestatic",// 17
  "racers",     // 18
  "wave",       // 19
  "shakeit",    // 20
  "strobe1",    // 21
  "strobe2",    // 22
  "gled",       // 23
  "heartbeat",  // 24
  //  "vumeter",    // 25
  "fastloop",   // 26
  "fastloop2",  // 27
  "pendulum",  // 27
  //  "noise_party",// 28
  "noise_lava", // 29
  "black"       // 30
};


*/




// Calculates difference between now and last time it was called
// and adjusts the interval of taskLedModeSelect accordingly
#ifdef SYNC_TO_BPM
void syncToBPM() {
  static unsigned long lastTimeStamp = 0 ;
  unsigned long currentTimeStamp = millis() ;

  DEBUG_PRINT( tapTempo.getBPM() ) ;
  DEBUG_PRINT( F("\t") ) ;
  DEBUG_PRINT( tapTempo.getBeatLength() ) ;
  DEBUG_PRINT( F("\t") ) ;
  DEBUG_PRINT( currentTimeStamp - lastTimeStamp ) ;
  DEBUG_PRINT( F("\t") ) ;
  DEBUG_PRINT( taskLedModeSelect.getInterval() ) ;

  unsigned long delta = currentTimeStamp - lastTimeStamp ;
  long syncFactor = ( delta - tapTempo.getBeatLength() ) * 5 ;
  syncFactor = constrain( syncFactor, -3000, 3000 ) ;
 
  DEBUG_PRINT( F("\t") ) ;
  DEBUG_PRINT( syncFactor ) ;
  DEBUG_PRINTLN() ;

  if ( delta != tapTempo.getBeatLength() ) {  // we're outta sync, try harder :)
    taskLedModeSelect.setInterval( taskLedModeSelect.getInterval() - syncFactor ) ;
  } else {
    DEBUG_PRINT( F("\tsync\t") ) ; // we're in sync, hooray!
    DEBUG_PRINTLN( tapTempo.getBPM() ) ; 
  }
  lastTimeStamp = millis() ;
}
#endif


#ifdef WORKINGCODE
void getYPRAccel() {
  // orientation/motion vars
  Quaternion quat;        // [w, x, y, z]         quaternion container
  VectorInt16 aa;         // [x, y, z]            accel sensor measurements
  VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
  VectorFloat gravity;    // [x, y, z]            gravity vector
  float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

  mpu.dmpGetQuaternion(&quat, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &quat);
  mpu.dmpGetYawPitchRoll(ypr, &quat, &gravity);
  mpu.dmpGetAccel(&aa, fifoBuffer);
  mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);

  yprX = (ypr[0] * 180 / M_PI) + 180;
  yprY = (ypr[1] * 180 / M_PI) + 90;
  yprZ = (ypr[2] * 180 / M_PI) + 90;

  aaRealX = aaReal.x ;
  aaRealY = aaReal.y ;
  aaRealZ = aaReal.z ;

  int maxXY    = max( aaReal.x, aaReal.y) ;
  int maxAccel = max( maxXY, aaReal.z) ;

  static int inBeat = false; // debounce variable

  if ( maxAccel > 7000 and inBeat == false ) {
    inBeat = true ;
    if ( longPressActive ) {   // only count BPM when button is held down
      tapTempo.update(true);
      digitalWrite(BUTTON_LED_PIN, HIGH); // Beat detected, light up to show beat detected
    }
  } else if ( maxAccel < 7000 ) {
    inBeat = false ;
    tapTempo.update(false);

    if ( longPressActive ) {
      digitalWrite(BUTTON_LED_PIN, LOW); // turn off LED, normal operation
    } else {  // normal operation; flash LED in time with BPM
      if ( tapTempo.beatProgress() > 0.95 ) {
        digitalWrite(BUTTON_LED_PIN, HIGH); // turn on LED, normal operation
      } else {
        digitalWrite(BUTTON_LED_PIN, LOW); // turn on LED, normal operation
      }
    }
  }
}
#endif
