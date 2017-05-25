// Fill a gradient on a LED ring with any possible start positions.
// startLed and endLed may be negative (one or both), may be larger than NUM_LEDS (one or both)
// startLed cannot (yet) be > endLed 

void fillGradientRing( int startLed, CRGB startColor, int endLed, CRGB endColor ) {
  if ( startLed > endLed ) {
    fill_gradient_RGB(leds, endLed, CRGB::Red, startLed, CRGB::Red); // show RED for error!
  } else {

    // Determine actual start and actual end (normalize using modulo):
    int actualStart = (startLed + NUM_LEDS) % NUM_LEDS ;
    int actualEnd = (endLed + NUM_LEDS) % NUM_LEDS ;

    // If beginning is at say 50, and end at 10, then we split the gradient in 2:
    // * one from 50-59
    // * one from 0-10
    // To determine which color should be at 59 and 0 we use the blend function:
    if ( actualStart > actualEnd ) {
      int ratio = round( (1 - (actualEnd / (endLed - startLed))) * 255 ) ; // determine what ratio of startColor and endColor we need at LED 0
      CRGB colorAtLEDZero = blend(startColor, endColor, ratio);

      fill_gradient_RGB(leds, actualStart, startColor, NUM_LEDS - 1, colorAtLEDZero);
      fill_gradient_RGB(leds, 0, colorAtLEDZero, actualEnd, endColor);
    } else {
      fill_gradient_RGB(leds, actualStart, startColor, actualEnd, endColor);
    }
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

    if ((targetLedPos - currentLedPos + NUM_LEDS) % NUM_LEDS < NUM_LEDS / 2) {
      goClockwise = true ;
    } else {
      goClockwise = false  ;
    }

    if ( goClockwise ) {
      currentLedPos++ ;
      if ( currentLedPos > NUM_LEDS - 1 ) {
        currentLedPos = 0 ;
      }
    } else {
      currentLedPos-- ;
      if ( currentLedPos < 0 ) {
        currentLedPos = NUM_LEDS - 1 ;
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



