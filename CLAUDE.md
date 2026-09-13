# Atmo — Air Quality Companion (Wio Terminal)

This file is the handoff brief for Claude Code working in this folder
(`P1- Atmo/`). It's distilled from the design/decision doc Javi and Claude
kept in the "HW" claude.ai Project while planning this build (there's no
live sync between that Project and Claude Code, so this file is the actual
source of truth going forward — update it here as the build evolves,
not there).

## What this project is

A desk companion that reads indoor air quality and expresses it through an
animated face on the Wio Terminal's screen, instead of a numeric dashboard.
Ambient, affective UX rather than a data readout.

Explicitly an *environmental air-quality* project, not a breath-analysis
one. Avoid "nose"/"sniff" framing in naming, comments, or animation ideas
unless Javi deliberately says otherwise.

Name: **Atmo** — both the project and the on-screen character.

## How Javi wants this built — read this before writing any code

Javi hasn't coded in a long time and wants this to be a genuine learning
experience, not Claude Code handing him a finished result he doesn't
understand. This is the most important instruction in this file and applies
to every step of the build:

- **Build incrementally, in small reviewable steps.** Never generate the
  whole implementation, or even a whole file, in one go. Break work into
  conceptual chunks — e.g. first just the library imports/includes, then
  sensor initialization, then variables/state, then the reading loop, then
  the face rendering — one chunk at a time, pausing for Javi to review and
  run it before moving to the next.
- **Comment code for a learner, in natural language.** Every non-trivial
  section needs a clear comment explaining what it does and *why*, so Javi
  can actually read and understand the code, not just copy-paste something
  that happens to work.
- **After each step, act as a teacher: give a concrete test.** Not "test it
  now" — the actual action and the actual expected result. E.g. after
  adding library imports: "compile/upload now and check the serial monitor
  — you should see [specific expected output]." After wiring + initializing
  a sensor: "connect it to the Grove port and run [specific sketch] — you
  should see [expected reading/behavior]." Every step ends with something
  Javi can concretely run and check himself.

This working style applies to the whole Atmo build, not just the first
milestone.

## GitHub

Decided 2026-09-13:

- GitHub account: **Javimetro**
- Repo: **`atmo`**, public — `https://github.com/Javimetro/atmo`
- Commit identity (set *locally* to this repo, not globally — see commands
  below): name `Javi Jorganes`, email `jorganes@etik.com` (deliberately not
  Javi's Proton address, since this repo is public and commit emails are
  scrapable forever).

If the repo hasn't been created/pushed yet when you (Claude Code) start
working here, do it before the first commit:

```
git init
git config user.name "Javi Jorganes"
git config user.email "jorganes@etik.com"
git remote add origin https://github.com/Javimetro/atmo.git
```

The GitHub-side repo itself (`Javimetro/atmo`) needs to exist before the
first `git push` — if `git push -u origin main` fails because it doesn't,
tell Javi to create an empty repo (no README/`.gitignore`/license, so there's
no history conflict) at github.com/new named `atmo` under Javimetro, or run
`gh repo create Javimetro/atmo --public --source=. --push` if the `gh` CLI
is installed and authenticated on his machine.

Never put a password, personal access token, or SSH private key in this
file or in any commit — auth is whatever Javi already has set up locally
(`gh auth login` or an SSH key).

Sensible first commit once there's something worth committing: this
`CLAUDE.md` plus a `.gitignore` for the Arduino/PlatformIO build (e.g.
`.pio/`, `*.hex`, `*.bin`, `build/`) if the toolchain generates one.

## What we're building first (Milestone 1 scope)

Wio Terminal + Battery Chassis + Grove I2C Hub. No wireless networking, no
Raspberry Pi — plenty of headroom to run I2C sensor reads + Bosch's BSEC
library + the face renderer entirely on the Wio Terminal alone.

**Sensors**: BME688 (Grove single-sensor board) + SGP30, both through the
Grove I2C Hub into one Wio Terminal Grove port.
- SGP30 (VOC + eCO2) — I2C address `0x58`.
- BME688 (temp/humidity/pressure/gas, with BSEC AI gas classification) —
  I2C address `0x76` or `0x77` depending on hardware SDO pull.
- No address conflict between these two.
- BME680 (same board family as BME688, no onboard AI classification) is
  owned as a spare but *not* wired in for this build — it very likely
  shares BME688's address family, so don't try to run both on the bus at
  once without confirming a hardware address change is possible.

**Important wiring fact** (confirmed from the Wio Terminal's own schematic):
the two onboard Grove ports are **not** separate I2C buses — both tie to
the same `I2C1_SCL`/`I2C1_SDA` net. The Grove I2C Hub is a passive splitter
of that same bus, not a multiplexer. So plugging sensors into different
Grove ports does *not* avoid I2C address collisions; a real bus split would
need an active mux chip (e.g. TCA9548A), which this build doesn't use or
need since BME688 and SGP30 don't collide.

**Software stack**: Arduino framework for Wio Terminal (Seeed board
package); Bosch BSEC library for BME688 → calibrated IAQ index (0–500);
SGP30 library for TVOC/eCO2 (needs its own baseline calibration, ~12h to
fully settle — runs fine uncalibrated in the meantime, worth telling Javi
this explicitly when that code goes in).

**Face rendering**: draw the face as vector shapes (ellipse eyes, arc
eyebrows, arc/bezier mouth) redrawn each frame, rather than sprite art —
easiest way to animate and to tween between expressions on this display.

**Animation architecture** — keep two concerns separate from the start, so
adding idle animation later is additive, not a rewrite:
1. "Which mood is Atmo in" — driven purely by the air-quality reading.
2. "What is Atmo doing right now within that mood" — idle micro-animation.

Build (1) first as static poses per mood. Layer (2) on top once that's
working, without touching the mood-detection logic.

### Milestones

1. **Bring-up**: wire BME688 + SGP30 through the I2C Hub, print raw
   readings over serial.
2. Compute a single air-quality score/category from BSEC IAQ + SGP30
   TVOC/eCO2.
3. Build the face engine: static pose per mood first.
4. Map air-quality categories to expressions, with smooth (tweened, not
   hard-cut) transitions between moods. Draft expression set — still open
   to refinement with Javi:
   - **Content** (good air) — relaxed curved-up eyes, soft mouth, slow idle
     blink.
   - **Curious/alert** (moderate) — eyebrows raise slightly, eyes widen a
     touch.
   - **Concerned** (poor) — furrowed brow, mouth flattens/turns down,
     faster blink.
   - **Distressed** (hazardous) — eyes squint, mouth animates like a
     cough/sneeze, maybe a shake animation.
5. Polish: buzzer chirp/alert cues, dim the face at night using the onboard
   light sensor.

Idle micro-animations to layer on per mood once the static version works:
- Content — slow idle blink, gentle bob/sway like breathing, occasional
  happy glance side to side, maybe a bounce loop or stretch/yawn.
- Curious/alert — head-tilt, eyes darting as if "checking something," one
  eyebrow twitching up randomly.
- Concerned — brow twitch/pulse, small wince tic, creeping blink rate.
- Distressed — repeated cough loop, shake/vibrate effect, maybe a "watery
  eye" flourish.

Remember: per the working-style section above, break each milestone itself
into small, commented, individually testable steps when actually
implementing it — don't build a whole milestone in one shot either.

## Confirmed scope decisions (don't relitigate without asking Javi)

- Name: **Atmo** (project + character), locked 2026-09-12.
- First build's sensors: BME688 + SGP30 via the Grove I2C Hub. No TGS1820,
  no H2S/MQ modules, no 8x BME688 AI-Studio dev kit, no BME680 — those live
  under "Ideas for later" below instead.
- Compute: Wio Terminal (+ Battery Chassis) alone. No Raspberry Pi for this
  build.
- Still open: whether expression transitions are instant or tweened
  (current lean: tweened, per the animation architecture above, but not
  locked).

## Ideas for later (not in scope — don't build unless Javi asks)

- **Distributed sensor node**: add sensing on an ESP32 Feather (HUZZAH32),
  reporting over WiFi (MQTT, optionally via a Pi-hosted broker, or plain
  HTTP) to the Wio Terminal. Lets a sensor live elsewhere (e.g. the
  kitchen) while Atmo's face stays on the desk. Candidate for the Grove
  Multichannel Gas Sensor V2 (already owned, I2C address `0x55`,
  NO2/ethanol-VOC/general-VOC/CO) as a general "something's off" alarm.
- **Butane/LPG leak detection**: the right sensor for this is the **MQ-2**
  (analog, ~48h preheat/burn-in, needs a clean-air baseline) — the
  Multichannel Gas Sensor V2 does *not* cover LPG/propane. Natural pairing:
  its own kitchen-located Feather node, not Atmo's desk unit.
- **Remote monitoring / phone** — two separable ideas:
  1. Proactive alerting: push notification the moment a hazardous reading
     trips, straight from the Wio Terminal or a Feather's WiFi to something
     like ntfy.sh/Pushover/a Telegram bot webhook — no backend needed.
  2. Passive remote viewing: seeing Atmo's current face/mood live from a
     phone — needs a small backend + simple app/web page, bigger scope.
- **Sniffing nose / animal character** idle animation — touches the
  nose/sniff naming boundary above. Don't add without Javi explicitly
  deciding to revise that boundary; a bounce/ear-twitch/blink-and-tilt tic
  keeps the boundary intact as an alternative.
- **AI integration (Claude/MCP)**: give Atmo actual reasoning/conversation
  (e.g. comparing indoor readings against fetched outside weather and
  saying something proactive) rather than only reacting with a face.
  Physical path confirmed feasible: the Wio Terminal's 40-pin header is a
  real Raspberry-Pi-HAT connection (carries `RPI_5V`/`RPI_3V3` and
  GPIO/I2C/SPI/UART), so it can mount directly on a Raspberry Pi 5, which
  handles the internet-facing parts (weather API, Claude API/MCP — the Wio
  Terminal's own MCU can't run those) and pushes cues to the Wio Terminal
  over the wired link. Trade-off to weigh later: wired/stacked (simpler,
  no network hop, but ties Atmo to sitting on the Pi) vs. WiFi to a
  separate Pi/cloud backend (Atmo can sit anywhere).

## Portfolio angle

Frame it as an ambient affective interface for air-quality feedback: the
interesting part isn't the sensors, it's translating a boring index into an
emotionally legible companion. Document with short GIFs/video of each
expression, a wiring diagram, and the calibration process.

## Reference

Component datasheets (including converted `.md` versions for many sensors)
live one level up, in `../documentation/`, organized per component —
`huzzah32/`, `Nano 3.0 Atmega328/`, `Neuro PlayGround (NPG) Lite/`,
`raspberry-pi-5/`, `wio_terminal/`, and `sensors/<name>/` (including
`sensors/BME680/` and presumably `sensors/BME688/` and `sensors/SGP30/`).
Check there before asking Javi for a spec that's probably already saved.
