#include <midi_operations.h>
#include <Arduino.h>
#include <MIDIUSB.h>

// First event parameter is the event type (0x09 = note on, 0x08 = note off).
// Second event parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third event parameter is the note number (48 = middle C).
// Fourth event parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, (byte)(0x90 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, (byte)(0x80 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

// First event parameter is the event type (0x0B = control change).
// Second event parameter is the event type, combined with the channel.
// Third event parameter is the control number number (0-119).
// Fourth event parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, (byte)((0xB0 | channel) - 1), control, value};
  MidiUSB.sendMIDI(event);
}
void pitchBend(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0E, (byte)((0xE0 | channel) - 1), control, value};
  MidiUSB.sendMIDI(event);
}
