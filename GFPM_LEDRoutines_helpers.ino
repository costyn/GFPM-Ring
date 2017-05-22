void fillGradientRing( int startLed, CRGB startColor, int endLed, CRGB endColor ) {
  if( startLed >= NUM_LEDS or startLed < 0 ) {
    startLed = startLed % NUM_LEDS ;
  }

  if ( endLed >= NUM_LEDS ) {
    int restLed = endLed % NUM_LEDS  ;
    int ratio = 0 ;
    ratio = round( (1 - restLed / (endLed - startLed)) * 255 ) ;

    CRGB ringEndColor = blend(startColor, endColor, ratio);

    fill_gradient_RGB(leds, startLed, startColor, NUM_LEDS - 1, ringEndColor);
    fill_gradient_RGB(leds, 0, ringEndColor, restLed, endColor);

  } else {
    fill_gradient_RGB(leds, startLed, startColor, endLed, endColor);
  }
}

#define OFFSET 8   // offset for aligning gyro "bottom" with LED "bottom"

int lowestPoint() {
  Quaternion quat;        // [w, x, y, z]         quaternion container
  VectorFloat gravity;    // [x, y, z]            gravity vector
  float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

  mpu.dmpGetQuaternion(&quat, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &quat);
  mpu.dmpGetYawPitchRoll(ypr, &quat, &gravity);

  float myYprP = (ypr[1] * 180 / M_PI) ;  // Convert from radians to degrees:
  float myYprR = (ypr[2] * 180 / M_PI) ;

  int targetLedPos = 0 ;
  static int currentLedPos = 0 ;
  int ratio = 0 ;
  int mySpeed = round( (abs(myYprP) + abs(myYprR) ) * 100 );
  taskLedModeSelect.setInterval( map( mySpeed, 0, 9000, 25, 5) ) ;

  if ( myYprR < 0 and myYprP < 0 ) {
    ratio =  (abs( myYprP ) / (abs(myYprP) + abs(myYprR))) * 100 ;
    targetLedPos = map( ratio, 0, 100, 0 , 14 );

  } else if ( myYprR > 0 and myYprP < 0 ) {
    ratio =  (abs( myYprR ) / (abs(myYprP) + abs(myYprR))) * 100 ;
    targetLedPos = map( ratio, 0, 100, 15 , 29 );

  } else if ( myYprR > 0 and myYprP > 0 ) {
    ratio =  (abs( myYprP ) / (abs(myYprP) + abs(myYprR))) * 100 ;
    targetLedPos = map( ratio, 0, 100, 30 , 44 );

  } else if ( myYprR < 0 and myYprP > 0 ) {
    ratio =  (abs( myYprR ) / (abs(myYprP) + abs(myYprR))) * 100 ;
    targetLedPos = map( ratio, 0, 100, 45 , 60 );
  } else {
    DEBUG_PRINT(F("\tWTF\t")) ;  // This should never happen
  }
  targetLedPos = ( (targetLedPos + OFFSET) % NUM_LEDS ) ;

  if ( currentLedPos != targetLedPos ) {
    bool goClockwise = true ;

    // http://stackoverflow.com/questions/7428718/algorithm-or-formula-for-the-shortest-direction-of-travel-between-two-degrees-on

    if ((targetLedPos - currentLedPos + NUM_LEDS) % NUM_LEDS < NUM_LEDS/2) {
      goClockwise = true ;
    } else {
      goClockwise = false  ;
    }

    if ( goClockwise ) {
      currentLedPos++ ;
      if ( currentLedPos > NUM_LEDS-1 ) {
        currentLedPos = 0 ;
      }
    } else {
      currentLedPos-- ;
      if ( currentLedPos < 0 ) {
        currentLedPos = NUM_LEDS-1 ;
      }
    }

  }

  return currentLedPos ;


/*
  DEBUG_PRINT(myYprP) ;
  DEBUG_PRINT("\t") ;
  DEBUG_PRINT(myYprR) ;
  DEBUG_PRINT("\t") ;
  DEBUG_PRINT(ratio) ;
  DEBUG_PRINT("\t") ;
  DEBUG_PRINT(targetLedPos) ;
  DEBUG_PRINT("\t") ;
  DEBUG_PRINT(currentLedPos) ;
  DEBUG_PRINT("\t") ;
  DEBUG_PRINT(mySpeed) ;
  DEBUG_PRINTLN() ;
*/

}


void fadeall(uint8_t fade_all_speed) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(fade_all_speed);
  }
}

void brightall(uint8_t bright_all_speed) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] += leds[i].scale8(bright_all_speed) ;
  }
}


void addGlitter( fract8 chanceOfGlitter)
{
  for ( int i = 0 ; i < 5 ; i++ ) {
    if ( random8() < chanceOfGlitter) {
      leds[ random16(NUM_LEDS) ] += CRGB::White;
    }
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



