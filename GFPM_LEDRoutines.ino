#define MAX_NEG_ACCEL -3000
#define MAX_POS_ACCEL 3000

void FillLEDsFromPaletteColors() {
  static uint8_t startIndex = 0;  // initialize at start
  static byte flowDir = 1 ;

  const CRGBPalette16 palettes[] = { RainbowColors_p, RainbowStripeColors_p, OceanColors_p, HeatColors_p, PartyColors_p, CloudColors_p, ForestColors_p } ;

  if ( isMpuUp() ) {
    flowDir = 1 ;
  } else if ( isMpuDown() ) {
    flowDir = -1 ;
  }

  startIndex += flowDir ;

  uint8_t colorIndex = startIndex ;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( palettes[ledMode], colorIndex, MAX_BRIGHT, NOBLEND );
    colorIndex += STEPS;
  }
  addGlitter(80);

  FastLED.setBrightness( map( constrain(aaRealZ, 0, MAX_POS_ACCEL), 0, MAX_POS_ACCEL, MAX_BRIGHT, 0 )) ;

  FastLED.show();
}


/*
  // Not used anywhere, but feel free to replace addGlitter with addColorGlitter in FillLEDsFromPaletteColors() above
  void addColorGlitter( fract8 chanceOfGlitter)
  {
  for ( int i = 0 ; i < 4 ; i++ ) {
    if ( random8() < chanceOfGlitter) {
      leds[ random16(NUM_LEDS) ] = CHSV( random8(), 255, MAX_BRIGHT);
    }
  }
  }
*/

void fadeGlitter() {
  addGlitter(90);
  FastLED.show();
  fadeall(200);
}

void discoGlitter() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  addGlitter(map( constrain( activityLevel(), 0, 3000), 0, 3000, 100, 255 ));
  FastLED.show();
}


#define S_SENSITIVITY 3000  // lower for less movement to trigger accelerometer routines

void strobe2() {
  if ( activityLevel() > S_SENSITIVITY ) {
    fill_solid(leds, NUM_LEDS, CHSV( map( yprX, 0, 360, 0, 255 ), 255, MAX_BRIGHT)); // yaw for color
  } else {
    fadeall(150);
  }
  FastLED.show();
}

#define MIN_BRIGHT 10

void pulse2() {
  int middle ;
  static int startP ;
  static int endP ;
  static uint8_t hue ;
  static int brightness = 0 ;
  static int bAdder = 1;
  static bool flowDir = 1;
  static bool sequenceEnd = true ;

  if ( brightness < MIN_BRIGHT ) {
    sequenceEnd = true ;
  }

  if ( not sequenceEnd ) {
    if ( flowDir ) {
      endP-- ;
      startP = endP - 20 ;
    } else {
      startP++ ;
      endP = startP + 20 ;
    }

    if ( startP == 89 or endP == 1 ) {
      sequenceEnd = true ;
    }

    middle = endP - round( (endP - startP) / 2 ) ;

    startP = constrain(startP, 0, NUM_LEDS - 1) ;
    middle = constrain(middle, 0, NUM_LEDS - 1) ;
    endP = constrain(endP, 0, NUM_LEDS - 1) ;

    brightness += bAdder ;
    brightness = constrain(brightness, 0, MAX_BRIGHT) ;
    if ( brightness >= 250 ) {
      bAdder = -10 ;
    }

    fill_gradient(leds, startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness), SHORTEST_HUES);
    fill_gradient(leds, middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0), SHORTEST_HUES);

  } else {

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    hue = random8(0, 60) ;
    brightness = MIN_BRIGHT ;
    bAdder = 15 ;
    flowDir = ! flowDir ; // flip it!
    sequenceEnd = false ;

    if ( flowDir ) {
      endP = random8(30, 70);
    } else {
      startP = random8(30, 70);
    }
  }

  FastLED.show();
}


void pulse_static() {
  int middle ;
  static int startP ;
  static int endP ;
  static uint8_t hue ;
  static int bAdder ;

  static int brightness = 0 ;
  static bool sequenceEnd ;

  if ( brightness < MIN_BRIGHT ) {
    sequenceEnd = true ;
  }

  // while brightness is more than MIN_BRIGHT, keep increasing brightness etc.
  // If brightness drops below MIN_BRIGHT, we start a new sequence at a new position
  if ( not sequenceEnd ) {
    if ( bAdder < 0 and startP < endP ) {
      startP++ ;
      endP-- ;
      if ( startP == endP ) {
        sequenceEnd = true ;
      }
    }
    if ( bAdder > 0  and ( endP - startP < 30 ) ) {
      startP-- ;
      endP++ ;
    }
    middle = endP - round( (endP - startP) / 2 ) ;

    startP = constrain(startP, 0, NUM_LEDS - 1) ;
    middle = constrain(middle, 0, NUM_LEDS - 1) ;
    endP = constrain(endP, 0, NUM_LEDS - 1) ;

    brightness += bAdder ;
    brightness = constrain(brightness, 0, MAX_BRIGHT) ;
    if ( brightness >= 250 ) {
      bAdder = -5 ;
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    fill_gradient(leds, startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness), SHORTEST_HUES);
    fill_gradient(leds, middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0), SHORTEST_HUES);
    FastLED.show();
  }

  if ( sequenceEnd ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    hue = random8(0, 60) ;
    brightness = MIN_BRIGHT + 1 ;
    bAdder = 10 ;
    startP = random8(1, 70);
    endP = startP + 30 ;
    sequenceEnd = false ;
    taskLedModeSelect.setInterval(random16(200, 700)) ;
  }
}




#define COOLING  55
#define SPARKING 120
#define FIRELEDS round( NUM_LEDS / 2 )

// Adapted Fire2012. This version starts in the middle and mirrors the fire going down to both ends.
// Works well with the Adafruit glow fur scarf.
// FIRELEDS defines the position of the middle LED.

void Fire2012()
{
  // Array of temperature readings at each simulation cell
  static byte heat[FIRELEDS];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < FIRELEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / FIRELEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = FIRELEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = FIRELEDS; j < NUM_LEDS; j++) {
    int heatIndex = j - FIRELEDS ;
    CRGB color = HeatColor( heat[heatIndex]);
    leds[j] = color;
  }

  /*  "Reverse" Mapping needed:
      ledindex 44 = heat[0]
      ledindex 43 = heat[1]
      ledindex 42 = heat[2]
      ...
      ledindex 1 = heat[43]
      ledindex 0 = heat[44]
  */
  for ( int j = 0; j <= FIRELEDS; j++) {
    int ledIndex = FIRELEDS - j ;
    CRGB color = HeatColor( heat[j]);
    leds[ledIndex] = color;
  }

  FastLED.show();
}

void racingLeds() {
  //  static long loopCounter = 0 ;
  static uint8_t racer[] = {0, 1, 2, 3}; // Starting positions
  static int racerDir[] = {1, 1, 1, 1}; // Current direction
  static int racerSpeed[] = { random8(1, 4), random8(1, 4) , random8(1, 4), random8(1, 4) }; // Starting speed
  const CRGB racerColor[] = { CRGB::Red, CRGB::Blue, CRGB::White, CRGB::Orange }; // Racer colors

#define NUMRACERS sizeof(racer) //array size  

  fill_solid(leds, NUM_LEDS, CRGB::Black);    // Start with black slate

  for ( int i = 0; i < NUMRACERS ; i++ ) {
    leds[racer[i]] = racerColor[i]; // Assign color

    // If taskLedModeSelect.getRunCounter() is evenly divisible by 'speed' then check if we've reached the end (if so, reverse), and do a step
    if ( ( taskLedModeSelect.getRunCounter() % racerSpeed[i]) == 0 ) {
      if ( racer[i] + racerDir[i] >= NUM_LEDS) {
        racer[i] = 0 ;
      } else {
        racer[i] += racerDir[i] ;
      }
      /*
            if ( (racer[i] + racerDir[i] >= NUM_LEDS) or (racer[i] + racerDir[i] <= 0) ) {
              racerDir[i] *= -1 ;
            }
            racer[i] += racerDir[i] ;
      */
    }

    if ( (taskLedModeSelect.getRunCounter() % 40 ) == 0 ) {
      racerSpeed[i] = random8(2, 6) ;  // Randomly speed up or slow down
    }
  }

  //  loopCounter++ ;

  FastLED.show();
}

#define MAX_NEG_ACCEL -5000
#define MAX_POS_ACCEL 5000
#define MIN_BRIGHT 20

void waveYourArms() {
  // Use yaw for color; use accelZ for brightness
  fill_solid(leds, NUM_LEDS, CHSV( map( yprX, 0, 360, 0, 255 ) , 255, map( constrain(aaRealZ, MAX_NEG_ACCEL, MAX_POS_ACCEL), MAX_NEG_ACCEL, MAX_POS_ACCEL, MIN_BRIGHT, MAX_BRIGHT )) );
  FastLED.show();
}


#define SENSITIVITY 2300  // lower for less movement to trigger

void shakeIt() {
  int startLed ;

  if ( isMpuDown() ) {  // Start near controller if down
    startLed = 0 ;
  } else if ( isMpuUp() ) {
    startLed = NUM_LEDS - 1 ;
  }

  if ( activityLevel() > SENSITIVITY ) {
    leds[startLed] = CHSV( map( yprX, 0, 360, 0, 255 ), 255, MAX_BRIGHT); // yaw for color
  } else {
    leds[startLed] = CHSV(0, 0, 0); // black
    //    leds[] = leds[NUM_LEDS - 1] ;  // uncomment for circular motion
  }

  if ( isMpuDown() ) {
    for (int i = NUM_LEDS - 2; i >= 0 ; i--) {
      leds[i + 1] = leds[i];
    }
  } else if ( isMpuUp() ) {
    for (int i = 0 ; i <= NUM_LEDS - 2 ; i++) {
      leds[i] = leds[i + 1];
    }
  }

  FastLED.show();
}

#ifdef WHITESTRIPE
#define STRIPE_LENGTH 5

void whiteStripe() {
  static CRGB patternCopy[STRIPE_LENGTH] ;
  static int startLed = 0 ;

  if ( taskWhiteStripe.getInterval() != WHITESTRIPE_SPEED ) {
    taskWhiteStripe.setInterval( WHITESTRIPE_SPEED ) ;
  }

  if ( startLed == 0 ) {
    for (int i = 0; i < STRIPE_LENGTH ; i++ ) {
      patternCopy[i] = leds[i];
    }
  }

  // 36 40   44 48 52 56   60

  leds[startLed] = patternCopy[0] ;
  for (int i = 0; i < STRIPE_LENGTH - 2; i++ ) {
    patternCopy[i] = patternCopy[i + 1] ;
  }
  patternCopy[STRIPE_LENGTH - 1] = leds[startLed + STRIPE_LENGTH] ;

  fill_gradient(leds, startLed + 1, CHSV(0, 0, 255), startLed + STRIPE_LENGTH, CHSV(0, 0, 255), SHORTEST_HUES);

  startLed++ ;

  if ( startLed + STRIPE_LENGTH == NUM_LEDS - 1) { // LED nr 90 is index 89
    for (int i = startLed; i < startLed + STRIPE_LENGTH; i++ ) {
      leds[i] = patternCopy[i];
    }

    startLed = 0 ;
    taskWhiteStripe.setInterval(random16(4000, 10000)) ;
  }

  FastLED.show();
}
#endif


// This routine needs pitch/roll information in floats, so we need to retrieve it separately
//  Suggestions how to fix this/clean it up welcome.


void gLed() {
  leds[lowestPoint()] = ColorFromPalette( PartyColors_p, taskLedModeSelect.getRunCounter(), MAX_BRIGHT, NOBLEND );
  FastLED.show();
  fadeall(200);
}

void gGradient() {
  const CRGB bgColor = CRGB::Blue ;
  int ledPos = lowestPoint() ;
  fill_solid(leds, NUM_LEDS, bgColor );
  fillGradientRing( ledPos, bgColor, ledPos+10, CRGB::Red ) ;
  fillGradientRing( ledPos + 11, CRGB::Red, ledPos + 20, bgColor ) ;
  FastLED.show();
}

void gradientBounce() {


  
}



/*
    Simple twirlers:
  void twirlersS(int numTwirlers) {
  int pos = 0 ;
  int first = taskLedModeSelect.getRunCounter() % NUM_LEDS ;

  fadeall(map( numTwirlers, 1, 6, 245, 130 ));
  //  fill_solid(leds, NUM_LEDS, CRGB::Black);

  for (int i = 0 ; i < numTwirlers ; i++) {
    pos = (first + round( NUM_LEDS / numTwirlers ) * i) % NUM_LEDS ;
    if ( (i % 2) == 0 ) {
      leds[pos] = CRGB::White ;
    } else {
      leds[pos] = CRGB::Red ;
    }
  }

  FastLED.show();
  }
*/

// Counter rotating twirlers with blending
void twirlers(int numTwirlers, bool opposing ) {
  int pos = 0 ;
  int clockwiseFirst = taskLedModeSelect.getRunCounter() % NUM_LEDS ;
  //  CRGB clockwiseColor = ColorFromPalette( PartyColors_p, taskLedModeSelect.getRunCounter() % 255, MAX_BRIGHT, NOBLEND );
  //  CRGB antiClockwiseColor = ColorFromPalette( PartyColors_p, (255 - taskLedModeSelect.getRunCounter()) % 255, MAX_BRIGHT, NOBLEND );
  const CRGB clockwiseColor = CRGB::White ;
  const CRGB antiClockwiseColor = CRGB::Red ;

  if ( opposing ) {
    fadeall(map( numTwirlers, 1, 6, 240, 180 ));
  } else {
    fadeall(map( numTwirlers, 1, 6, 240, 150 ));
  }

  for (int i = 0 ; i < numTwirlers ; i++) {
    if ( (i % 2) == 0 ) {
      pos = (clockwiseFirst + round( NUM_LEDS / numTwirlers ) * i) % NUM_LEDS ;
      if ( leds[pos] ) { // FALSE if currently BLACK - don't blend with black
        leds[pos] = blend( leds[pos], clockwiseColor, 128 ) ;
      } else {
        leds[pos] = clockwiseColor ;
      }

    } else {
      if ( opposing ) {
        int antiClockwiseFirst = NUM_LEDS - taskLedModeSelect.getRunCounter() % NUM_LEDS ;
        pos = (antiClockwiseFirst + round( NUM_LEDS / numTwirlers ) * i) % NUM_LEDS ;
      } else {
        pos = (clockwiseFirst + round( NUM_LEDS / numTwirlers ) * i) % NUM_LEDS ;
      }
      if ( leds[pos] ) { // FALSE if currently BLACK - don't blend with black
        leds[pos] = blend( leds[pos], antiClockwiseColor, 128 ) ;
      } else {
        leds[pos] = antiClockwiseColor ;
      }
    }
  }

  FastLED.show();
}

void heartbeat() {

  const int *hbTable[] = {
    25,
    61,
    105,
    153,
    197,
    233,
    253,
    255,
    252,
    243,
    230,
    213,
    194,
    149,
    101,
    105,
    153,
    197,
    216,
    233,
    244,
    253,
    255,
    255,
    252,
    249,
    243,
    237,
    230,
    223,
    213,
    206,
    194,
    184,
    174,
    162,
    149,
    138,
    126,
    112,
    101,
    91,
    78,
    69,
    62,
    58,
    51,
    47,
    43,
    39,
    35,
    29,
    25,
    22,
    19,
    15,
    12,
    9,
    6,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    0
  };

  static int counter = 0 ;
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.setBrightness( hbTable[counter] );
  if ( hbTable[counter] == 0 ) {
    counter = 0 ;
  } else {
    counter++ ;
  }
  FastLED.show();
}

