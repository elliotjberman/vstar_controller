/* Deeply indebted to the project
 * Mini Untztrument, Arduino Style by gdsports
 *
 * Repo here: https://github.com/gdsports/miniuntz
 */

#include <Arduino.h>
#include <Adafruit_NeoTrellis.h>

#include <MIDIUSB.h>
#include <midi_operations.h>
#include <find_index.h>

#include <wheel.h>
#include <trellis_animations.h>

#define LED      LED_BUILTIN // Pin for heartbeat LED (shows code is working)
#define CHANNEL  1           // MIDI channel number

Adafruit_NeoTrellis trellis;

uint8_t          rateMod;
uint8_t          pitch;
bool             clicked;

uint8_t       heart        = 0;  // Heartbeat LED counter
unsigned long prevReadTime = 0L; // Keypad polling timer

bool notesOn[16];
uint32_t colours[16];

uint8_t velocity = 100;
uint8_t note[] = {
  48, 49, 50, 51,
  44, 45, 46, 47,
  40, 41, 42, 43,
  36, 37, 38, 39,
};

//define a callback for key presses
TrellisCallback blink(keyEvent evt){
  // Check is the pad pressed?
  if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    noteOn(CHANNEL, note[evt.bit.NUM], velocity);
  } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
    noteOn(CHANNEL, note[evt.bit.NUM], 0);
  }

  return 0;
}

uint32_t dimColour(uint32_t colour, int factor) {
  uint32_t dimmed = colour;
  while(factor--) {
    dimmed = (dimmed & 0xfefefe) >> 1;
  }
  return dimmed;
}

void setup() {
  pinMode(LED, OUTPUT);
  trellis.begin(); // Pass I2C address

  startupAnimation(trellis);

  //activate all keys and set callbacks
  for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
    trellis.registerCallback(i, blink);
  }

  rateMod = map(analogRead(0), 0, 1023, 0, 127);
  pitch = map(analogRead(1), 0, 1023, 0, 127);
  clicked = map(analogRead(2), 0, 1023, 0, 127);


  controlChange(CHANNEL,  0, rateMod);
  pitchBend(CHANNEL, 0, pitch);
  controlChange(CHANNEL,  1, clicked);

}

uint8_t getReading(int pinNumber) {
  return map(analogRead(pinNumber), 0, 1023, 0, 127);
}

void loop() {
  clicked = map(analogRead(2), 0, 1023, 0, 127);
  bool switchDown = clicked == 0;

  trellis.read();
  unsigned long t = millis();
  if((t - prevReadTime) >= 20L) { // 20ms = min Trellis poll time

    uint8_t newRate = getReading(0);
    if(rateMod != newRate) {
      rateMod = newRate;
      controlChange(CHANNEL, 0, rateMod);
    }

    uint8_t newPitchBend = getReading(1);
    if(pitch != newPitchBend) {
      pitch = newPitchBend;
      pitchBend(CHANNEL, 0, pitch);
    }

    // Read midi in
    midiEventPacket_t rx;
    do {
      rx = MidiUSB.read();

      // Note on
      if (rx.header == 9) {
        notesOn[findIndex(rx.byte2)] = true;
      }
      // Note off
      else if (rx.header == 8) {
        notesOn[findIndex(rx.byte2)] = false;
      }

    } while (rx.header != 0);

    for (int i=0; i<16; ++i) {
      if (notesOn[i]) {
        colours[i] = Wheel(trellis, map(i, 0, trellis.pixels.numPixels(), 0, 255), switchDown);
      }
      else {
        colours[i] = dimColour(colours[i], 1);
      }
      trellis.pixels.setPixelColor(i, colours[i]);
    }

    trellis.pixels.show();

    prevReadTime = t;
    digitalWrite(LED, ++heart & 32); // Blink = alive
    MidiUSB.flush();
  }
}
