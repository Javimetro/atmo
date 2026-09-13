// Atmo — toolchain sanity check.
//
// This isn't part of the real Atmo firmware yet. Its only job is to prove
// that the whole chain works end to end: PlatformIO can compile for the
// Wio Terminal, upload over USB, and we can see output back on the PC via
// the serial monitor. Once that's confirmed, we'll replace this file with
// the real Milestone 1 bring-up code (I2C sensor reads).

#include <Arduino.h>

// setup() runs once, right after the board powers on or resets.
void setup() {
  // Start a serial connection to the PC over USB at 115200 baud (bits per
  // second). The number just has to match on both ends — we'll set the
  // same 115200 in the serial monitor when we test this.
  Serial.begin(115200);
}

// loop() runs over and over, forever, after setup() finishes.
void loop() {
  Serial.println("Atmo says hi — toolchain works!");
  delay(1000); // wait 1000 milliseconds (1 second) before printing again
}
