# Atmo — technical reference

Atmo is a desk air-quality companion built on a Seeed Wio Terminal. A
BME688 gas sensor feeds Bosch's BSEC library, which produces an IAQ
(indoor air quality) score; that score picks one of four moods for an
animated pixel-art hippo shown on the Wio Terminal's screen, alongside
live temperature, humidity, and a 4-star air-quality readout.

This file describes the current, final state of the project — for the
build's backstory and how it was made, see the main `README.md`.

## Hardware

- **Seeed Wio Terminal** — SAMD51 (ARM Cortex-M4F), 320×240 LCD, built-in
  5-way joystick switch. Runs the whole thing standalone.
- **Seeed Wio Terminal Battery Chassis** — adds a battery and extra Grove
  ports, so the unit runs cordless.
- **BME688 Grove gas/environmental sensor** — wired directly to one of
  the Wio Terminal's Grove I2C ports (I2C address `0x76`). Reports
  temperature, humidity, pressure, and gas resistance; Bosch's BSEC
  library turns the gas reading into a calibrated IAQ score.

No soldering — everything connects over Grove's 4-pin I2C cables.

## How mood is picked

`starsFromIaq()` in `Atmo/Atmo.ino` maps the live BSEC IAQ value to one
of 4 mood bands:

| IAQ range | Mood | Meaning |
|---|---|---|
| 0–50 | 4/4 stars | Good |
| 51–100 | 3/4 stars | Average |
| 101–200 | 2/4 stars | Bad |
| 201+ | 1/4 stars | Very bad |

These are Bosch's own published IAQ bands. IAQ itself comes from how
much the BME688's internal gas-sensing element's resistance shifts —
volatile organic compounds (VOCs) in the air (from cleaning products,
alcohol, paint, cooking, breath, etc.) change that resistance, and BSEC
turns the shift into a single 0–500 score. Lower is cleaner air.

BSEC reports an **accuracy** level (0–3) alongside IAQ: 0 means it's
still calibrating (readings exist but aren't trustworthy yet) — this is
normal for the first few minutes after power-on and climbs on its own
with normal operation.

Whenever BSEC delivers a new reading (`newDataCallback` in `Atmo.ino`),
if the mood band changed, the hippo **hard-cuts** to the new mood's
resting pose and its animation — no fade or transition frame.

## Hold-to-peek IAQ readout

Holding the joystick straight down (`WIO_5S_PRESS`) shows the live IAQ
number and BSEC's accuracy level in a strip at the bottom of the screen,
for as long as it's held. Useful for testing — e.g. holding rubbing
alcohol near the sensor to see the number climb and the mood change.

## Software

**Libraries used** (all via Arduino Library Manager / the Seeed board
package):
- `Seeed_Arduino_LCD` (TFT_eSPI fork) — screen driver.
- `BME68x Sensor library` — Bosch's low-level BME688 driver.
- `bsec2` — Bosch's BSEC2 library, the calibration/AI layer that turns
  raw gas readings into an IAQ score.

**Face rendering.** The hippo and every animation frame are stored as
4-bit palette-indexed images (a 10-color palette shared by every mood,
2 pixels packed per byte) rather than raw 16-bit color — this was
necessary to fit everything in the board's flash: storing every mood's
full-color animation frames raw would have needed ~845KB against a
~496KB budget; palette-indexed storage cut that to ~130KB. A small
lookup table converts an index back to a real color, decoded into a
scratch buffer right before each screen draw.

Each mood's idle animation only redraws the *small region that actually
changed* between frames (an eye, an ear, a floating heart) rather than
the whole 200×200 face each time — cheaper on flash and faster to draw.

**Files:**
- `Atmo/Atmo.ino` — the real, flashable sketch: sensor code, mood logic,
  and all 4 moods' animation data and playback.
- `Atmo/ImageTest/ImageTest.ino` — a development sandbox used while
  building the animation system; not needed to run Atmo, kept for
  reference.
- `media/` — source photos and reference images used to build the pixel
  art and this documentation.

## Building and flashing

1. Install the **Seeeduino:samd** board package (Wio Terminal) in the
   Arduino IDE, and the three libraries listed above via Library
   Manager.
2. Open `Atmo/Atmo.ino`, select the **Seeeduino Wio Terminal** board,
   pick the right serial port, and upload.
3. Wire the BME688 to either Grove port on the Wio Terminal (or via the
   Battery Chassis' Grove ports).

No configuration needed beyond that — the sketch starts on a placeholder
mood and switches to a real one within a few seconds of the first BSEC
reading.
