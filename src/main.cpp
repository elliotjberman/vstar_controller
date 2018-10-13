/* Deeply indebted to the project
 * Mini Untztrument, Arduino Style by gdsports
 *
 * Repo here: https://github.com/gdsports/miniuntz
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Trellis.h>
#include <MIDIUSB.h>
#include <midi_operations.h>
#include <trellis_animations.h>
#include <find_index.h>

#define LED      LED_BUILTIN // Pin for heartbeat LED (shows code is working)
#define CHANNEL  1           // MIDI channel number

Adafruit_Trellis trellis;

uint8_t          rateMod;
uint8_t          pitch;
bool             clicked;

uint8_t       heart        = 0;  // Heartbeat LED counter
unsigned long prevReadTime = 0L; // Keypad polling timer

bool notesOn[16];
const int FLASHING_RATE = 4;
int flashTimer = FLASHING_RATE;

uint8_t velocity = 100;
uint8_t note[] = {
  39, 38, 37, 36,
  43, 42, 41, 40,
  47, 46, 45, 44,
  51, 50, 49, 48,
};

void setup() {
  pinMode(LED, OUTPUT);
  trellis.begin(0x70); // Pass I2C address

  startupAnimation(trellis);

  trellis.clear();
  trellis.writeDisplay();

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
  unsigned long t = millis();
  if((t - prevReadTime) >= 20L) { // 20ms = min Trellis poll time
    if(trellis.readSwitches()) {  // Button state change?

      for(uint8_t i=0; i<16; i++) { // For each button...
        if(trellis.justPressed(i)) {
          noteOn(CHANNEL, note[i], velocity);

        } else if(trellis.justReleased(i)) {
          noteOn(CHANNEL, note[i], 0);
        }
      }
    }

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

    // PS4 Joytick Button animations
    bool newClick = getReading(2) < 1;
    if(clicked != newClick) {
      clicked = newClick;
      noteOn(CHANNEL, 1, velocity);
    }

    // Read midi in
    midiEventPacket_t rx;
    do {
      rx = MidiUSB.read();
      if (rx.header != 0) {
        if (rx.header == 9) {
          notesOn[findIndex(rx.byte2)] = true;

          if (rx.byte2 == 1) {
            clicked = true;
          }
        }
        else {
          notesOn[findIndex(rx.byte2)] = false;

          if (rx.byte2 == 1) {
            clicked = false;
          }
        }
      }
    } while (rx.header != 0);

    for (int i=0; i<16; ++i) {
      if (notesOn[i])
        trellis.setLED(i);
      else
        trellis.clrLED(i);
    }

    if (clicked)
      flashAnimation(trellis, flashTimer, FLASHING_RATE);
    else
      clearLEDS(trellis, notesOn);

    trellis.writeDisplay();

    prevReadTime = t;
    digitalWrite(LED, ++heart & 32); // Blink = alive
    MidiUSB.flush();
  }
}
