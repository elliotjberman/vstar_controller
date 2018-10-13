#include <trellis_animations.h>

void startupAnimation(Adafruit_Trellis& t) {
  int speed = 30;
  for (int i=36; i<52; ++i) {
    t.setLED(findIndex(i));
    t.writeDisplay();
    delay(speed);
  }
  for (int i=36; i<52; ++i) {
    t.clrLED(findIndex(i));
    t.writeDisplay();
    delay(speed);
  }
}

void clearLEDS(Adafruit_Trellis& t, bool (&notesOn)[16]) {
  for (int i=0; i<16; ++i) {
    if (!notesOn[i])
      t.clrLED(i);
  }
}

void flashAnimation(Adafruit_Trellis& t, int& flashTimer, int FLASHING_RATE) {
  for (int i=0; i<16; ++i) {
    if (flashTimer > FLASHING_RATE/2)
      t.setLED(i);
    else
      t.clrLED(i);
  }
  flashTimer = flashTimer > 0 ? flashTimer - 1 : FLASHING_RATE;
}

void allOn(Adafruit_Trellis& t) {
  for (int i=0; i < 16; ++i) {
    t.setLED(i);
  }
}
