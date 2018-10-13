#include <Adafruit_Trellis.h>
#include <find_index.h>

void startupAnimation(Adafruit_Trellis& t);
void clearLEDS(Adafruit_Trellis& t, bool (&notesOn)[16]);
void flashAnimation(Adafruit_Trellis& t, int& flashTimer, int FLASHING_RATE);
void allOn(Adafruit_Trellis& t);
