#define P_MAX_POS_ACCEL 3000
#define STEPS       3   // How wide the bands of color are.  1 = more like a gradient, 10 = more like stripes

void FillLEDsFromPaletteColors(uint8_t paletteIndex ) {
  static uint8_t startIndex = 15;  // initialize at start
  static int flowDir = 1 ;

  const CRGBPalette16 palettes[] = { RainbowColors_p,
#ifdef RT_P_RB_STRIPE
                                     RainbowStripeColors_p,
#endif
#ifdef RT_P_OCEAN
                                     OceanColors_p,
#endif
#ifdef RT_P_HEAT
                                     HeatColors_p,
#endif
#ifdef RT_P_LAVA
                                     LavaColors_p,
#endif
#ifdef RT_P_PARTY
                                     PartyColors_p,
#endif
#ifdef RT_P_CLOUD
                                     CloudColors_p,
#endif
#ifdef RT_P_FOREST
                                     ForestColors_p
#endif
                                   } ;

  if ( isMpuUp() ) {
    flowDir = 1 ;
  } else if ( isMpuDown() ) {
    flowDir = -1 ;
  }

  startIndex += flowDir ;

  uint8_t colorIndex = startIndex ;

  for ( uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( palettes[paletteIndex], colorIndex, MAX_BRIGHT, LINEARBLEND );
    colorIndex += STEPS;
  }
  addGlitter(80);

  FastLED.setBrightness( map( constrain(aaRealZ, 0, P_MAX_POS_ACCEL), 0, P_MAX_POS_ACCEL, MAX_BRIGHT, 0 )) ;
  FastLED.show();

  taskLedModeSelect.setInterval( beatsin16( tapTempo.getBPM(), 1500, 50000) ) ;
}


#ifdef RT_FADE_GLITTER
void fadeGlitter() {
  addGlitter(90);
  FastLED.show();
  fadeToBlackBy(leds, NUM_LEDS, 200);
}
#endif

#ifdef RT_DISCO_GLITTER
void discoGlitter() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  addGlitter(map( constrain( activityLevel(), 0, 3000), 0, 3000, 100, 255 ));
  FastLED.show();
}
#endif

#ifdef RT_STROBE1
#define FLASHLENGTH 20
void strobe1() {
  if ( tapTempo.beatProgress() > 0.95 ) {
    fill_solid(leds, NUM_LEDS, CHSV( map( yprX, 0, 360, 0, 255 ), 255, MAX_BRIGHT)); // yaw for color
  } else if ( tapTempo.beatProgress() > 0.80 and tapTempo.beatProgress() < 0.85 ) {
    fill_solid(leds, NUM_LEDS, CHSV( 0, 0, MAX_BRIGHT)); // yaw for color
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black); // yaw for color
  }
  FastLED.show();
}
#endif

#ifdef RT_STROBE2
#define S_SENSITIVITY 3500  // lower for less movement to trigger accelerometer routines

void strobe2() {
  if ( activityLevel() > S_SENSITIVITY ) {
    fill_solid(leds, NUM_LEDS, CHSV( map( yprX, 0, 360, 0, 255 ), 255, MAX_BRIGHT)); // yaw for color
  } else {
    fadeall(120);
  }
  FastLED.show();
}
#endif

/*
   pick a spot x
   gradient1: x-10, x
   gradient2: x, x+10
   if brightness < MAX_BRIGHT then fadeup = true
*/

#ifdef RT_PULSE2
#define MIN_BRIGHT 10
void pulse2() {
  uint8_t middle ;
  static uint8_t startP ;
  static uint8_t endP ;
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
      startP = endP - 10 ;
    } else {
      startP++ ;
      endP = startP + 10 ;
    }

    if ( startP == NUM_LEDS - 1 or endP == 1 ) {
      sequenceEnd = true ;
    }

    middle = endP - round( (endP - startP) / 2 ) ;

    startP = constrain(startP, 0, NUM_LEDS - 1) ;
    middle = constrain(middle, 0, NUM_LEDS - 1) ;
    endP = constrain(endP, 0, NUM_LEDS - 1) ;

    brightness += bAdder ;
    brightness = constrain(brightness, 0, MAX_BRIGHT) ;
    if ( brightness >= MAX_BRIGHT ) {
      bAdder = -10 ;
    }

    fillGradientRing(startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness));
    fillGradientRing(middle + 1, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0));

  } else {

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    hue = random8(0, 60) ;
    brightness = MIN_BRIGHT ;
    bAdder = 15 ;
    flowDir = ! flowDir ; // flip it!
    sequenceEnd = false ;

    if ( flowDir ) {
      endP = random8(30, NUM_LEDS);
    } else {
      startP = random8(30, NUM_LEDS);
    }
  }

  FastLED.show();
}
#endif  // pulse2

// TODO
#ifdef RT_PULSE3
void pulse3() {
  uint8_t middle ;
  static uint8_t startP ;
  static uint8_t endP ;
  static uint8_t hue ;
  static int brightness = 0 ;
  static int bAdder = 1;
  static bool sequenceEnd = true ;

  if ( brightness < MIN_BRIGHT ) {
    sequenceEnd = true ;
  }

  if ( not sequenceEnd ) {
    startP++ ;
    endP = startP + 10 ;

    if ( startP == NUM_LEDS - 1 or endP == 1 ) {
      sequenceEnd = true ;
    }

    middle = endP - round( (endP - startP) / 2 ) ;

    startP = constrain(startP, 0, NUM_LEDS - 1) ;
    middle = constrain(middle, 0, NUM_LEDS - 1) ;
    endP = constrain(endP, 0, NUM_LEDS - 1) ;

    brightness += bAdder ;
    brightness = constrain(brightness, 0, MAX_BRIGHT) ;
    if ( brightness >= MAX_BRIGHT ) {
      bAdder = -10 ;
    }

    fillGradientRing(startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness));
    fillGradientRing(middle + 1, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0));

  } else {

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    hue = random8(0, 60) ;
    brightness = MIN_BRIGHT ;
    bAdder = 15 ;
    sequenceEnd = false ;

    startP = random8(0, NUM_LEDS);
  }

  FastLED.show();
}
#endif

#ifdef RT_PULSE_STATIC
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
    if ( brightness >= MAX_BRIGHT ) {
      bAdder = -5 ;
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    fillGradientRing(startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness));
    fillGradientRing(middle + 1, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0));

    //    fill_gradient(leds, startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness), SHORTEST_HUES);
    //    fill_gradient(leds, middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0), SHORTEST_HUES);
    FastLED.show();
  }

  if ( sequenceEnd ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    hue = random8(0, 60) ;
    brightness = MIN_BRIGHT + 1 ;
    bAdder = 10 ;
    startP = random8(1, NUM_LEDS - 20);
    endP = startP + 20 ;
    sequenceEnd = false ;
    taskLedModeSelect.setInterval(random16(200, 700)) ;
  }
}
#endif


/*
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
  for ( uint8_t i = 0; i < FIRELEDS; i++) {
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

  //  "Reverse" Mapping needed:
  //    ledindex 44 = heat[0]
  //    ledindex 43 = heat[1]
  //    ledindex 42 = heat[2]
  //    ...
  //    ledindex 1 = heat[43]
  //    ledindex 0 = heat[44]

  for ( int j = 0; j <= FIRELEDS; j++) {
    int ledIndex = FIRELEDS - j ;
    CRGB color = HeatColor( heat[j]);
    leds[ledIndex] = color;
  }

  FastLED.show();
  }
*/

#ifdef RT_RACERS
void racingLeds() {
  //  static long loopCounter = 0 ;
  static uint8_t racer[] = {0, 1, 2, 3}; // Starting positions
  static int racerDir[] = {1, 1, 1, 1}; // Current direction
  static int racerSpeed[] = { random8(1, 4), random8(1, 4) , random8(1, 4), random8(1, 4) }; // Starting speed
  const CRGB racerColor[] = { CRGB::Red, CRGB::Blue, CRGB::White, CRGB::Orange }; // Racer colors

#define NUMRACERS sizeof(racer) //array size  

  fill_solid(leds, NUM_LEDS, CRGB::Black);    // Start with black slate

  for ( uint8_t i = 0; i < NUMRACERS ; i++ ) {
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
#endif


#ifdef RT_WAVE
#define WAVE_MAX_NEG_ACCEL -5000
#define WAVE_MAX_POS_ACCEL 5000
#define MIN_BRIGHT 20

void waveYourArms() {
  // Use yaw for color; use accelZ for brightness
  fill_solid(leds, NUM_LEDS, CHSV( map( yprX, 0, 360, 0, 255 ) , 255, map( constrain(aaRealZ, WAVE_MAX_NEG_ACCEL, WAVE_MAX_POS_ACCEL), WAVE_MAX_NEG_ACCEL, WAVE_MAX_POS_ACCEL, MIN_BRIGHT, MAX_BRIGHT )) );
  FastLED.show();
}
#endif


#ifdef RT_SHAKE_IT
#define SENSITIVITY 2300  // lower for less movement to trigger

void shakeIt() {
  int startLed = 0 ;

  if ( activityLevel() > SENSITIVITY ) {
    leds[startLed] = CHSV( map( yprX, 0, 360, 0, 255 ), 255, MAX_BRIGHT); // yaw for color
  } else {
    leds[startLed] = CHSV(0, 0, 0); // black
    //    leds[] = leds[NUM_LEDS - 1] ;  // uncomment for circular motion
  }

  for (uint8_t i = NUM_LEDS - 2; i >= 0 ; i--) {
    leds[i + 1] = leds[i];
  }

  FastLED.show();
}
#endif

#ifdef WHITESTRIPE
#define STRIPE_LENGTH 5

void whiteStripe() {
  static CRGB patternCopy[STRIPE_LENGTH] ;
  static int startLed = 0 ;

  if ( taskWhiteStripe.getInterval() != WHITESTRIPE_SPEED ) {
    taskWhiteStripe.setInterval( WHITESTRIPE_SPEED ) ;
  }

  if ( startLed == 0 ) {
    for (uint8_t i = 0; i < STRIPE_LENGTH ; i++ ) {
      patternCopy[i] = leds[i];
    }
  }

  // 36 40   44 48 52 56   60

  leds[startLed] = patternCopy[0] ;
  for (uint8_t i = 0; i < STRIPE_LENGTH - 2; i++ ) {
    patternCopy[i] = patternCopy[i + 1] ;
  }
  patternCopy[STRIPE_LENGTH - 1] = leds[startLed + STRIPE_LENGTH] ;

  fill_gradient(leds, startLed + 1, CHSV(0, 0, 255), startLed + STRIPE_LENGTH, CHSV(0, 0, 255), SHORTEST_HUES);

  startLed++ ;

  if ( startLed + STRIPE_LENGTH == NUM_LEDS - 1) { // LED nr 90 is index 89
    for (uint8_t i = startLed; i < startLed + STRIPE_LENGTH; i++ ) {
      leds[i] = patternCopy[i];
    }

    startLed = 0 ;
    taskWhiteStripe.setInterval(random16(4000, 10000)) ;
  }

  FastLED.show();
}
#endif

/*
  void gLedOrig() {
  leds[lowestPoint()] = ColorFromPalette( PartyColors_p, taskLedModeSelect.getRunCounter(), MAX_BRIGHT, NOBLEND );
  FastLED.show();
  fadeToBlackBy(leds, NUM_LEDS, 200);
  }
*/

#ifdef RT_GLED

#define GLED_WIDTH 3
void gLed() {
  uint8_t ledPos = lowestPoint() ;
  static uint8_t hue = 0 ;
  //  fill_solid(leds, NUM_LEDS, CRGB::Black );
  fillGradientRing( ledPos, CHSV(hue, 255, 0) , ledPos + GLED_WIDTH , CHSV(hue, 255, 255) ) ;
  fillGradientRing( ledPos + GLED_WIDTH + 1, CHSV(hue, 255, 255), ledPos + GLED_WIDTH + GLED_WIDTH, CHSV(hue, 255, 0) ) ;
  FastLED.show();
  fadeall(250);
  hue++ ;
}
#endif

#ifdef RT_VUMETER
void vuMeter() {
  static int vuLength = 1 ;
  static bool growing = true ;

  int center = NUM_LEDS / 2 ;
  int left = center + vuLength ;
  int right = center - vuLength ;

  if ( growing ) {
    if ( vuLength < 23 ) {
      leds[left] = CRGB::DarkGreen ;
      leds[right] = CRGB::DarkGreen ;
    } else {
      leds[left] = CRGB::Red ;
      leds[right] = CRGB::Red ;
    }
  } else {
    leds[left - 1] = CRGB::Black ;
    leds[right + 1] = CRGB::Black;
  }

  FastLED.setBrightness( MAX_BRIGHT );
  FastLED.show();

  if ( growing ) {
    vuLength++ ;
  } else {
    vuLength-- ;
  }

  if ( vuLength >= (NUM_LEDS / 2)  or vuLength <= 0 ) {
    growing = ! growing ;
  }
}


void vuMeter2() {
  static int vuLength = 1 ;
  static bool growing = true ;
  fill_solid(leds, NUM_LEDS, CRGB::Black );
  fill_solid(leds + (NUM_LEDS / 2 - vuLength), vuLength, CRGB::Blue );
  fill_solid(leds + NUM_LEDS / 2, vuLength, CRGB::Red );

  FastLED.setBrightness( MAX_BRIGHT );
  FastLED.show();

  if ( growing ) {
    vuLength++ ;
  } else {
    vuLength-- ;
  }

  if ( vuLength >= (NUM_LEDS / 2) or vuLength <= 1 ) {
    growing = ! growing ;
  }
}

#endif


#ifdef RT_TWIRL1 || RT_TWIRL2 || RT_TWIRL4 || RT_TWIRL6 || RT_TWIRL2_O || RT_TWIRL4_O || RT_TWIRL6_O
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

  for (uint8_t i = 0 ; i < numTwirlers ; i++) {
    if ( (i % 2) == 0 ) {
      pos = (clockwiseFirst + round( NUM_LEDS / numTwirlers ) * i) % NUM_LEDS ;
      if ( leds[pos] ) { // FALSE if currently BLACK - don't blend with black
        leds[pos] = blend( leds[pos], clockwiseColor, 128 ) ;
      } else {
        leds[pos] = clockwiseColor ;
      }

      if ( pos == 0 ) { // We want LED 0 to be hit at every beat for the "even" LEDs
        syncToBPM() ;
      }

    } else {
      if ( opposing ) {
        int antiClockwiseFirst = NUM_LEDS - taskLedModeSelect.getRunCounter() % NUM_LEDS ; // normalized backwards counter
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

#endif


#ifdef RT_HEARTBEAT
// Todo: sync to BPM
void heartbeat() {
  const uint8_t hbTable[] = {
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

  static int arrayIndex = 0 ;
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.setBrightness( hbTable[arrayIndex] );
  if ( hbTable[arrayIndex] == 0 ) {
    arrayIndex = 0 ;
    syncToBPM() ;  // sync to BPM
  } else {
    arrayIndex++ ;
  }
  FastLED.show();
}
#endif


#ifdef RT_FASTLOOP || RT_FASTLOOP2

#define FL_LENGHT 20   // how many LEDs should be in the "stripe"
#define FL_MIDPOINT FL_LENGHT / 2
#define MAX_LOOP_SPEED 5

void fastLoop(bool reverse) {
  static int startP = 0 ;  // start position
  static int hue = 0 ;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  fillGradientRing(startP, CHSV(hue, 255, 0), startP + FL_MIDPOINT, CHSV(hue, 255, MAX_BRIGHT));
  fillGradientRing(startP + FL_MIDPOINT + 1, CHSV(hue, 255, MAX_BRIGHT), startP + FL_LENGHT, CHSV(hue, 255, 0));
  FastLED.show();
  if ( reverse ) {
    startP += map( cos8(hue % 255), 0, 255, -MAX_LOOP_SPEED, MAX_LOOP_SPEED + 1) ; // abuse the 'hue' counter
  } else {
    startP++ ;
    if ( startP == 0 ) {
      syncToBPM() ;  // only sync to BPM on forward loop
    }
  }
  hue++ ;

}
#endif


#ifdef RT_NOISE_LAVA || RT_NOISE_PARTY
// FastLED library NoisePlusPalette routine rewritten for 1 dimensional LED strip
// - speed determines how fast time moves forward.  Try  1 for a very slow moving effect,
// or 60 for something that ends up looking like water.

// - Scale determines how far apart the pixels in our noise array are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise will be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

// if current palette is a 'loop', add a slowly-changing base value

void fillnoise8(uint8_t currentPalette, uint8_t speed, uint8_t scale, boolean colorLoop ) {
  static uint8_t noise[NUM_LEDS];

  const CRGBPalette16 palettes[] = { LavaColors_p, PartyColors_p } ;

  static uint16_t x = random16();
  static uint16_t y = random16();
  static uint16_t z = random16();

  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".
  uint8_t dataSmoothing = 0;

  if ( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    int ioffset = scale * i;

    uint8_t data = inoise8(x + ioffset, y, z);

    // The range of the inoise8 function is roughly 16-238.
    // These two operations expand those values out to roughly 0..255
    // You can comment them out if you want the raw noise data.
    data = qsub8(data, 16);
    data = qadd8(data, scale8(data, 39));

    if ( dataSmoothing ) {
      uint8_t olddata = noise[i];
      uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
      data = newdata;
    }

    noise[i] = data;
  }

  z += speed;

  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;

  static uint8_t ihue = 0;

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    // We use the value at the i coordinate in the noise
    // array for our brightness, and a 'random' value from NUM_LEDS - 1
    // for our pixel's index into the color palette.

    uint8_t index = noise[i];
    uint8_t bri =   noise[NUM_LEDS - i];
    // uint8_t bri =  sin(noise[NUM_LEDS - i]);  // more light/dark variation

    // if this palette is a 'loop', add a slowly-changing base value
    if ( colorLoop) {
      index += ihue;
    }

    // brighten up, as the color palette itself often contains the
    // light/dark dynamic range desired
    if ( bri > 127 ) {
      bri = 255;
    } else {
      bri = dim8_raw( bri * 2);
    }

    CRGB color = ColorFromPalette( palettes[currentPalette], index, bri);
    leds[i] = color;
  }
  ihue += 1;

  FastLED.show();
}
#endif


#ifdef RT_PENDULUM
void pendulum() {
  uint8_t hue = map( yprX, 0, 360, 0, 255 ) ; // yaw for color
  uint8_t sPos1 = beatsin8( tapTempo.getBPM(), 0, 30 ) ;
  uint8_t sPos2 = beatsin8( tapTempo.getBPM(), 30, 60 ) ;
  fillGradientRing(sPos1, CHSV(hue, 255, 0), sPos1 + 10, CHSV(hue, 255, MAX_BRIGHT));
  fillGradientRing(sPos1 + 11, CHSV(hue, 255, MAX_BRIGHT), sPos1 + 20, CHSV(hue, 255, 0));
  fillGradientRing(sPos2, CHSV(hue + 128, 255, 0), sPos2 + 10, CHSV(hue + 128, 255, MAX_BRIGHT));
  fillGradientRing(sPos2 + 11, CHSV(hue + 128, 255, MAX_BRIGHT), sPos2 + 20, CHSV(hue + 128, 255, 0));
  FastLED.show();
} // end pendulum()
#endif



#ifdef RT_BOUNCEBLEND
void bounceBlend() {
  uint8_t speed = beatsin8( tapTempo.getBPM(), 0, 255);
  static uint8_t startLed = 1 ;
  CHSV endclr = blend(CHSV(0, 255, 255), CHSV(160, 255, 255) , speed);
  CHSV midclr = blend(CHSV(160, 255, 255) , CHSV(0, 255, 255) , speed);
  fillGradientRing(startLed, endclr, startLed + NUM_LEDS / 2, midclr);
  fillGradientRing(startLed + NUM_LEDS / 2 + 1, midclr, startLed + NUM_LEDS, endclr);

  FastLED.show();

  if ( (taskLedModeSelect.getRunCounter() % 5 ) == 0 ) {
    if ( startLed + 1 == NUM_LEDS ) {
      startLed == 0 ;
    } else {
      startLed++ ;
    }
  }
} // end bounceBlend()
#endif


/* juggle_pal
   Originally by: Mark Kriegsman
   Modified by: Andrew Tuline
   Modified further by: Costyn van Dongen
   Date: May, 2017
*/

#ifdef RT_JUGGLE_PAL
void jugglePal() {                                             // A time (rather than loop) based demo sequencer. This gives us full control over the length of each sequence.

  static uint8_t    numdots =   4;                                     // Number of dots in use.
  static uint8_t   thisfade =   2;                                     // How long should the trails be. Very low value = longer trails.
  static uint8_t   thisdiff =  16;                                     // Incremental change in hue between each dot.
  static uint8_t    thishue =   0;                                     // Starting hue.
  static uint8_t     curhue =   0;                                     // The current hue
  static uint8_t    thissat = 255;                                     // Saturation of the colour.
  static uint8_t thisbright = MAX_BRIGHT;                               // How bright should the LED/display be.
  static uint8_t   thisbeat =   35;                                     // Higher = faster movement.

  uint8_t secondHand = (millis() / 1000) % 60;                // Change '60' to a different value to change duration of the loop (also change timings below)
  static uint8_t lastSecond = 99;                             // This is our 'debounce' variable.

  if (lastSecond != secondHand) {                             // Debounce to make sure we're not repeating an assignment.
    lastSecond = secondHand;
    switch (secondHand) {
      case  0: numdots = 1; thisbeat = tapTempo.getBPM() / 2; thisdiff = 8;  thisfade = 8;  thishue = 0;   break;
      case  7: numdots = 2; thisbeat = tapTempo.getBPM() / 2; thisdiff = 4;  thisfade = 12; thishue = 0;   break;
      case 25: numdots = 4; thisbeat = tapTempo.getBPM() / 2; thisdiff = 24; thisfade = 50; thishue = 128; break;
      case 40: numdots = 2; thisbeat = tapTempo.getBPM() / 2; thisdiff = 16; thisfade = 50; thishue = 0; break;
      case 52: numdots = 4; thisbeat = tapTempo.getBPM() / 2; thisdiff = 24; thisfade = 80; thishue = 160; break;
    }
  }

  curhue = thishue;                                           // Reset the hue values.
  fadeToBlackBy(leds, NUM_LEDS, thisfade);

  for ( uint8_t i = 0; i < numdots; i++) {
    leds[beatsin16(thisbeat + i + numdots, 0, NUM_LEDS - 1)] += ColorFromPalette(RainbowColors_p, curhue, thisbright, LINEARBLEND); // Munge the values and pick a colour from the palette
    curhue += thisdiff;
  }

  FastLED.show();

} // end jugglePal()
#endif

