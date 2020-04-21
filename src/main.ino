/*
    Edited (C) 2020 André Geiger TimeOfDayReminder
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>
#include <userconfig.h>

#include <Adafruit_NeoPixel.h>

// =========Definitions==============
#define MICROSEC              1000000L
#define MILLISEC              1000L
#define SEC                   1L
#define MINUTE                (unsigned int) 60L*SEC
#define HOUR                  (unsigned int) 60L*MINUTE
#define DAY                   (unsigned long) 24L*HOUR

#ifndef MY_LANGUAGE
  #include "language/de-DE.h"
#else
  #define QUOTEME(x) QUOTEME_1(x)
  #define QUOTEME_1(x) #x
  #define INCLUDE_FILE(x) QUOTEME(language/x.h)
  #include INCLUDE_FILE(MY_LANGUAGE)
#endif

#ifdef NEOPIXEL
#include "neopixel.h"
#endif // NEOPIXEL
// ===============================

void colorWipe(uint32_t color, int wait);
void theaterChase(uint32_t color, int wait);
void theaterChaseRainbow(int wait);
void rainbow(int wait);

ESP8266WebServer server(80);
#include "wifi.h"

void setup(void) {
  Serial.begin(115200);
    // Wifi initialize
  wifiInit();

  led_segments.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  led_segments.show();            // Turn OFF all pixels ASAP
  led_segments.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop(void) {
  server.handleClient();
  
  // Fill along the length of the strip in various colors...
  colorWipe(led_segments.Color(255,   0,   0), 50); // Red
  colorWipe(led_segments.Color(  0, 255,   0), 50); // Green
  colorWipe(led_segments.Color(  0,   0, 255), 50); // Blue

  // Do a theater marquee effect in various colors...
  //theaterChase(led_segments.Color(127, 127, 127), 50); // White, half brightness
  //theaterChase(led_segments.Color(127,   0,   0), 50); // Red, half brightness
  //theaterChase(led_segments.Color(  0,   0, 127), 50); // Blue, half brightness

  rainbow(10);             // Flowing rainbow cycle along the whole strip
  theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<led_segments.numPixels(); i++) { // For each pixel in strip...
    led_segments.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    led_segments.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      led_segments.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<led_segments.numPixels(); c += 3) {
        led_segments.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      led_segments.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<led_segments.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / led_segments.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      led_segments.setPixelColor(i, led_segments.gamma32(led_segments.ColorHSV(pixelHue)));
    }
    led_segments.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      led_segments.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<led_segments.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / led_segments.numPixels();
        uint32_t color = led_segments.gamma32(led_segments.ColorHSV(hue)); // hue -> RGB
        led_segments.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      led_segments.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}