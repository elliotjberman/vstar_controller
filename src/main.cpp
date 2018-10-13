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

#define LED      LED_BUILTIN // Pin for heartbeat LED (shows code is working)
#define CHANNEL  1           // MIDI channel number
#define ANALOG_IN

Adafruit_Trellis trellis;

uint8_t          rateMod;
uint8_t          pitch;
uint8_t          click;

uint8_t       heart        = 0;  // Heartbeat LED counter
unsigned long prevReadTime = 0L; // Keypad polling timer

uint8_t velocity = 100;
uint8_t note[] = {
  39, 38, 37, 36,
  43, 42, 41, 40,
  47, 46, 45, 44,
  51, 50, 49, 48,
};

int findIndex(int x) {
  int row = (x / 4) - 9;
  int column = 3 - x % 4;
  return row * 4 + column;
}

void startupAnimation(Adafruit_Trellis t) {
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

void setup() {
  pinMode(LED, OUTPUT);
  trellis.begin(0x70); // Pass I2C address

  startupAnimation(trellis);

  trellis.clear();
  trellis.writeDisplay();

  #ifdef ANALOG_IN
  rateMod = map(analogRead(0), 0, 1023, 0, 127);
  pitch = map(analogRead(1), 0, 1023, 0, 127);
  click = map(analogRead(2), 0, 1023, 0, 127);


  controlChange(CHANNEL,  0, rateMod);
  pitchBend(CHANNEL, 0, pitch);
  controlChange(CHANNEL,  1, click);
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

    #ifdef ANALOG_IN
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

    // uint8_t newClick = map(analogRead(2), 0, 1023, 0, 127);
    // if(click != newClick) {
    //   click = newClick;
    //   controlChange(CHANNEL, 1, click);
    // }

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
