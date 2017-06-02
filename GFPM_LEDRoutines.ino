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

  for ( uint8_t i = 0; i < NUM_LEDS; i++) {
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

#define FLASHLENGTH 20

void strobe1() {
  if ( tapTempo.beatProgress() > 0.95 ) {
    fill_solid(leds, NUM_LEDS, CHSV( map( yprX, 0, 360, 0, 255 ), 255, MAX_BRIGHT)); // yaw for color
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black); // yaw for color
  }
  FastLED.show();
}


#define S_SENSITIVITY 3500  // lower for less movement to trigger accelerometer routines

void strobe2() {
  if ( activityLevel() > S_SENSITIVITY ) {
    fill_solid(leds, NUM_LEDS, CHSV( map( yprX, 0, 360, 0, 255 ), 255, MAX_BRIGHT)); // yaw for color
  } else {
    fadeall(150);
  }
  FastLED.show();
}

/*
   pick a spot x
   gradient1: x-10, x
   gradient2: x, x+10
   if brightness < MAX_BRIGHT then fadeup = true
*/


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
    fillGradientRing(middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0));

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
    fillGradientRing(middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0));

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
    fillGradientRing(middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0));

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
  int startLed = 0 ;

  if ( activityLevel() > SENSITIVITY ) {
    leds[startLed] = CHSV( map( yprX, 0, 360, 0, 255 ), 255, MAX_BRIGHT); // yaw for color
  } else {
    leds[startLed] = CHSV(0, 0, 0); // black
    //    leds[] = leds[NUM_LEDS - 1] ;  // uncomment for circular motion
  }

  for (int i = NUM_LEDS - 2; i >= 0 ; i--) {
    leds[i + 1] = leds[i];
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
  const CHSV chsvBlue = CHSV( 160, 255, MAX_BRIGHT ) ;
  const CHSV chsvRed = CHSV( 0, 255, MAX_BRIGHT ) ;
  //  int ledPos = lowestPoint() ;
  int ledPos = 20 ;
  fill_solid(leds, NUM_LEDS, CRGB::Black );
  fillGradientRing( ledPos, chsvBlue , ledPos + 10, chsvRed  ) ;
  fillGradientRing( ledPos + 11, chsvRed , ledPos + 20, chsvBlue ) ;
  FastLED.show();
}

void gradientBounce() {



}

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

// Todo: sync to BPM
void heartbeat() {
  const byte *hbTable[] = {
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
  } else {
    arrayIndex++ ;
  }
  FastLED.show();
}


// Todo: 
// - sync to BPM
// - spin faster and slower then turn around and spin faster and slower

#define FL_LENGHT 20   // how many LEDs should be in the "stripe"
#define FL_MIDPOINT FL_LENGHT / 2

void fastLoop() {
  static int startP = 0 ;  // start position
  static int hue = 0 ;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  fillGradientRing(startP, CHSV(hue, 255, 0), startPos + FL_MIDPOINT, CHSV(hue, 255, MAX_BRIGHT));
  fillGradientRing(startP + FL_MIDPOINT, CHSV(hue, 255, MAX_BRIGHT), startP + FL_LENGHT, CHSV(hue, 255, 0));
  FastLED.show();
  startP++ ;
  if( taskLedModeSelect.getRunCounter() % 2 ) == 0 ) {   // slow down the color change a bit
    hue++ ;
  }
}


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

  const CRGBPalette16 palettes[] = { RainbowColors_p, RainbowStripeColors_p, OceanColors_p, HeatColors_p, LavaColors_p, PartyColors_p, CloudColors_p, ForestColors_p } ;

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

  for (int i = 0; i < NUM_LEDS; i++) {
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

  for (int i = 0; i < NUM_LEDS; i++) {
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



// Todo: sync to BPM

void pendulum() {
  static int counter = 0 ;
  int hue = map( yprX, 0, 360, 0, 255 ) ; // yaw for color
  int sPos1 = map( cubicwave8(counter), 0, 255, 0, 30) ;
  int sPos2 = map( cubicwave8(counter), 0, 255, 30, 60 ) ;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  fillGradientRing(sPos1, CHSV(hue, 255, 0), sPos1 + 10, CHSV(hue, 255, MAX_BRIGHT));
  fillGradientRing(sPos1 + 10, CHSV(hue, 255, MAX_BRIGHT), sPos1 + 20, CHSV(hue, 255, 0));
  fillGradientRing(sPos2, CHSV(hue + 128, 255, 0), sPos2 + 10, CHSV(hue + 128, 255, MAX_BRIGHT));
  fillGradientRing(sPos2 + 10, CHSV(hue + 128, 255, MAX_BRIGHT), sPos2 + 20, CHSV(hue + 128, 255, 0));
  FastLED.show();
  counter += 6 ;  // increase for faster swinging. 
}


