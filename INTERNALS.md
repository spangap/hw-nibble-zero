# hw-nibble-zero — INTERNALS

Maintainer reference for the Retia Nibble Zero board HAL. For what the straddle
is and how to build with it, read [README.md](README.md) first.

## 1. Bring-up: the CS park, and why it runs in the `start:` band

The only firmware this straddle owns is `NibbleZeroBoard::onStart`
([esp-idf/src/nibblezero.cpp](esp-idf/src/nibblezero.cpp)). It configures the
SX1262's chip-select (`CONFIG_LORA0_CS_PIN`, GPIO 10) as an output and drives it
HIGH — deselected — so the radio does not drive the shared MISO line before
`loraInit()` (in [iface-lora](../iface-lora)) claims the SPI pins.

It runs in the `start:` band, **before** `spangapInit()`, because the radio must
be parked before any SPI bus setup touches those pins. The dispatcher that calls
it is generated into the `reticulous/reticulous` buildable from this straddle's
`services:` declaration — there is no hand-written `app_main` here.

The CS park is guarded by `#if defined(CONFIG_LORA0_CS_PIN)`, so the source still
compiles if the board is ever staged without `iface-lora` (the symbol is defined
only when that straddle is in the build).

Unlike the Heltec V4 there is **no** Vext-style peripheral power rail to bring
up, and unlike the T-Deck the SX1262 is on its own SPI bus (not shared with the
flash or an SD card), so there is no power sequencing and no shared-bus probe to
race — the CS park is the whole of bring-up.

## 2. Everything else is Kconfig VALUES, not sources

A board straddle is **non-buildable**: a `sdkconfig.defaults` here would be
ignored under `--with`. So the hardware profile lives entirely in
`straddle.yaml`'s `kconfig:` block, collected into the buildable's staged
fragments. Two groups:

- **Memory** — `CONFIG_ESPTOOLPY_FLASHSIZE_4MB`, `CONFIG_SPIRAM_MODE_QUAD`,
  `CONFIG_SPANGAP_MAX_FIRMWARE_KB=3072`. These are values for symbols the
  platform/IDF own. The firmware floor keeps a `/state` partition alive on the
  4 MB chip (see README's memory table).
- **LoRa** — the `CONFIG_LORA*` pins and radio flags, owned by
  [iface-lora](../iface-lora). This board only supplies values; it defines no
  new symbols.

## 3. Sourcing & pitfalls

The pin map and memory profile were reconstructed from Retia's product pages and
the community `nibble-connect` Meshtastic variant
(`Sparkling-Ice/firmware`, branch `nibble-connect`), which targets the
PlatformIO board `esp32-s3-zero`. Nobody has flashed reticulous on a Nibble Zero
through this straddle yet. Known things to confirm on real hardware:

- **4 MB flash is tight.** app+fixed is ~2.8 MB; `/state` gets ~1 MB and there is
  no room for an A/B OTA pair. If your module is larger, raise the flash-size and
  firmware-floor Kconfig together.
- **NeoPixel pin ambiguity** — GPIO 21 (variant) vs GPIO 17 (Retia blog). Left
  unwired; resolve before enabling.
- **Nibble revisions** — the "Nibble", "Nibble Connect" and "Nibble Zero" share a
  lineage but not necessarily an identical pinout. Re-verify per revision.
