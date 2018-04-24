/* Deeply indebted to the project
 * Mini Untztrument, Arduino Style by gdsports
 *
 * Repo here: https://github.com/gdsports/miniuntz
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Trellis.h>
#include <MIDIUSB.h>

#define LED     LED_BUILTIN // Pin for heartbeat LED (shows code is working)
#define CHANNEL 1           // MIDI channel number

Adafruit_Trellis trellis;


#define ANALOG_INPUT

#ifdef ANALOG_INPUT
uint8_t       rateMod;
uint8_t       pitch;
uint8_t       fxc;
uint8_t       rate;
#endif

int velocity = 100;
uint8_t       heart        = 0;  // Heartbeat LED counter
unsigned long prevReadTime = 0L; // Keypad polling timer

uint8_t note[] = {
  48, 49, 50, 51,
  44, 45, 46, 47,
  40, 41, 42, 43,
  36, 37, 38, 39
};
// Wires would be here on the Trellis

int findIndex(int x) {
  int row = 12 - (x / 4);
  int column = x % 4;
  return row * 4 + column;
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, (byte)(0x90 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, (byte)(0x80 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
// Currently not in use
void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, (byte)(0xB0 | channel)-1, control, value};
  MidiUSB.sendMIDI(event);
}
void pitchBend(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0E, (byte)(0xE0 | channel)-1, control, value};
  MidiUSB.sendMIDI(event);
}

void startupAnimation(Adafruit_Trellis t) {
  int speed = 30;
  for (int i=0; i<16; ++i) {
    t.setLED(i);
    t.writeDisplay();
    delay(speed);
  }
  for (int i=0; i<16; ++i) {
    t.clrLED(i);
    t.writeDisplay();
    delay(speed);
  }
}

void setup() {
  pinMode(LED, OUTPUT);
  trellis.begin(0x70); // Pass I2C address

  startupAnimation(trellis);

  trellis.clear();
  trellis.writeDisplay();

  #ifdef ANALOG_INPUT
  rateMod = map(analogRead(0), 0, 1023, 0, 127);
  pitch = map(analogRead(1), 0, 1023, 0, 127);

  controlChange(CHANNEL,  0, rateMod);
  pitchBend(CHANNEL, 0, pitch);
  #endif

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

    #ifdef ANALOG_INPUT

    uint8_t newRate = map(analogRead(0), 0, 1023, 0, 127);
    if(rateMod != newRate) {
      rateMod = newRate;
      controlChange(CHANNEL, 0, rateMod);
    }

    uint8_t newPitchBend = map(analogRead(1), 0, 1023, 0, 127);
    if(pitch != newPitchBend) {
      pitch = newPitchBend;
      pitchBend(CHANNEL, 0, pitch);
    }

    #endif

    // Read midi in
    midiEventPacket_t rx;
    do {
      rx = MidiUSB.read();
      if (rx.header != 0) {
        if (rx.header == 9) {
          trellis.setLED(findIndex(rx.byte2));
        }
        else {
          trellis.clrLED(findIndex(rx.byte2));
        }
      }
    } while (rx.header != 0);
    trellis.writeDisplay();

    prevReadTime = t;
    digitalWrite(LED, ++heart & 32); // Blink = alive
    MidiUSB.flush();
  }
}
