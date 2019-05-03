#include <trellis_animations.h>
#include <wheel.h>

void startupAnimation(Adafruit_NeoTrellis& trellis) {
  for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
    trellis.pixels.setPixelColor(i, Wheel(trellis, map(i, 0, trellis.pixels.numPixels(), 0, 255), false));
    trellis.pixels.show();
    delay(20);
  }
  for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
    trellis.pixels.setPixelColor(i, 0x000000);
    trellis.pixels.show();
    delay(20);
  }
}
