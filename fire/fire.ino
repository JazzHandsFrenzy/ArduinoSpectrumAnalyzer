#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 6

Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 – 500 Ohm resistor on first pixel’s data input
// and minimize distance between Arduino and first pixel. Avoid connecting
// on a live circuit…if you must, connect GND first.

void setup() {
// This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
// End of trinket special code

strip.begin();
strip.show(); // Initialize all pixels to ‘off’
}

void loop() {
int r = 150;
int g = 34.5;
int b = 13.3;

for(int x = 0; x <40; x++)
{
int flicker = random(0,50);
int r1 = r-flicker;
int g1 = g-flicker;
int b1 = b-flicker;
if(g1<0) g1=0;
if(r1<0) r1=0;
if(b1<0) b1=0;
strip.setPixelColor(x,r1,g1, b1);
}
strip.show();
delay(random(50,150));
}
