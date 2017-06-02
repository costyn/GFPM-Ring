// Fill a gradient on a LED ring with any possible start positions.
// startLed and endLed may be negative (one or both), may be larger than NUM_LEDS (one or both)
// startLed cannot (yet) be > endLed

void fillGradientRing( int startLed, CHSV startColor, int endLed, CHSV endColor ) {
  if ( startLed > endLed ) {
    fill_gradient(leds, endLed, CHSV(0, 255, 255) , startLed, CHSV(0, 255, 255), SHORTEST_HUES ); // show RED for error!
    //    DEBUG_PRINTLN(F("GRMBL\t")) ;  // This should never happen
  } else {
    // Determine actual start and actual end (normalize using modulo):
    int actualStart = mod(startLed + NUM_LEDS, NUM_LEDS)  ;
    int actualEnd = mod(endLed + NUM_LEDS, NUM_LEDS) ;

    // If beginning is at say 50, and end at 10, then we split the gradient in 2:
    // * one from 50-59
    // * one from 0-10
    // To determine which color should be at 59 and 0 we use the blend function:
    if ( actualStart > actualEnd ) {
      float ratio = 1.0 - float(actualEnd) / float(endLed - startLed) ; // cast to float otherwise the division won't work
      int normalizedRatio = round( ratio * 255 ) ; // determine what ratio of startColor and endColor we need at LED 0
      CHSV colorAtLEDZero = blend(startColor, endColor, normalizedRatio);

      fill_gradient(leds, actualStart, startColor, NUM_LEDS - 1, colorAtLEDZero, SHORTEST_HUES);
      fill_gradient(leds, 0, colorAtLEDZero, actualEnd, endColor, SHORTEST_HUES);
      /*
            Serial.print(F("\t"));
            Serial.print(endLed - startLed) ;
            Serial.print(F("\t"));
            Serial.print(ratio) ;
            Serial.print(F("\t"));
            Serial.print(normRatio) ;
      */
    } else {
      fill_gradient(leds, actualStart, startColor, actualEnd, endColor, SHORTEST_HUES);
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
  targetLedPos = mod(targetLedPos + OFFSET, NUM_LEDS) ;

  if ( currentLedPos != targetLedPos ) {
    bool goClockwise = true ;

    // http://stackoverflow.com/questions/7428718/algorithm-or-formula-for-the-shortest-direction-of-travel-between-two-degrees-on

    if ( mod(targetLedPos - currentLedPos + NUM_LEDS,NUM_LEDS) < NUM_LEDS / 2) {   // custom modulo
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

// Custom mod which always returns a positive number
int mod(int x, int m) {
    return (x%m + m)%m;
}


// for debugging purposes
int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

