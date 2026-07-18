# hw-nibble-zero — Retia Nibble Zero board HAL

**hw-nibble-zero** is the board-support straddle for the **Retia Nibble Zero** —
an ESP32-S3 LoRa mesh node built around the **ESP32-S3-Zero** module
(ESP32-S3FH4R2: 4 MB flash, 2 MB **quad** PSRAM) carrying one Semtech
**SX1262** (a Seeed **Wio-SX1262** module) on its own SPI bus. It makes the board
usable by an application: it owns the LoRa CS park and publishes the board's pin
map and hardware tuning as Kconfig. Board reference:
<https://retia.io/products/nibble-zero-kit>.

It is a **non-buildable** component — it decides nothing about what the device
*does*. A buildable assembler (`reticulous/reticulous`) adds it and inherits the
board: `spangap build reticulous/reticulous --with spangap/hw-nibble-zero`. The
mesh stack, the IP/web platform, `app_main`, the partition layout, the update
story and the browser SPA all come from the buildable and its other straddles —
not from here.

This board has no color LCD, GNSS or SD card wired: it builds **headless**
automatically (there is no LCD straddle in its dependency set, so nothing pulls
in `spangap-lcd`). The Nibble Zero's SSD1306 OLED, single NeoPixel, six buttons
and I2C sensor header exist on the hardware but are deliberately left unwired
here; only LoRa is brought up.

## ⚠️ Verify before trusting

Everything below was assembled from Retia's product pages and the community
`nibble-connect` Meshtastic variant, **not** from a board in hand. Confirm
against your actual unit before an RF or partition run:

- **Flash size.** The PlatformIO target for this board is `esp32-s3-zero`, i.e.
  the Waveshare ESP32-S3-Zero module = **4 MB flash / 2 MB quad PSRAM**. 4 MB is
  tight for the reticulous buildable (app+fixed is ~2.8 MB, leaving only ~696 KB
  for `/state` and no room for an A/B OTA pair). If your unit carries a larger
  module (e.g. N8R2 / N16R8), bump `CONFIG_ESPTOOLPY_FLASHSIZE_*` and
  `CONFIG_SPANGAP_MAX_FIRMWARE_KB` in `straddle.yaml`.
- **LoRa pin map.** Taken from the `nibble-connect` variant (see below). If your
  board is a different Nibble revision, re-check every `CONFIG_LORA*` pin.
- **NeoPixel pin.** The variant places the single NeoPixel on **GPIO 21**;
  Retia's product blog cites **GPIO 17**. Left unwired here either way — resolve
  before enabling it.
- **SPI host.** `CONFIG_LORA_SPI_HOST=2` (SPI3) is chosen for parity with the
  Heltec V4 board; any free host works since the radio is on its own bus.

## What it does, and how it fits

The board contributes one hook that the buildable's generated init dispatcher
calls. There is nothing to call by hand: if the straddle is in the build, the
board comes up automatically.

| Hook | Band | Present when | Brings up |
|---|---|---|---|
| `NibbleZeroBoard::onStart` | start | always | LoRa CS park HIGH |

`onStart` runs in the `start:` band, **before** `spangapInit()`. It is
bare-hardware bring-up: it parks the SX1262's CS line HIGH so the radio does not
drive MISO before `loraInit()` (in [iface-lora](../iface-lora)) claims the pin.
There is no `init:`-band companion — there is no on-device UI in this build. The
Nibble Zero has no gated peripheral power rail (unlike the Heltec V4's Vext), so
unlike that board there is nothing else to power up at boot.

The LoRa radio engine, the IP/web platform and the mesh stack are owned by other
straddles ([iface-lora](../iface-lora), [spangap-core](../spangap-core),
[spangap-net](../spangap-net), [rns](../rns)); this board only supplies the
SX1262's pins (below) and the CS glue.

## Hardware & pin map

Retia Nibble Zero — **ESP32-S3-Zero** module (4 MB flash, 2 MB **quad** PSRAM,
selected via `CONFIG_SPIRAM_MODE_QUAD`). A single SX1262 LoRa modem sits on
**SPI host 2** on its own bus, separate from the flash bus.

### LoRa SX1262 (owned by iface-lora, pins published here)

| Signal | GPIO | | Signal | GPIO |
|---|---|---|---|---|
| NSS / CS | 10 | | RST | 6 |
| SCK | 13 | | BUSY | 5 |
| MOSI | 11 | | DIO1 | 4 |
| MISO | 12 | | | |

The SX1262 drives **DIO2** as its own RF antenna switch
(`CONFIG_LORA0_DIO2_RF_SWITCH=y`) and **DIO3** supplies the 1.8 V TCXO
(`CONFIG_LORA0_TCXO_MV=1800`). One radio (`CONFIG_LORA_COUNT=1`),
`CONFIG_LORA0_RADIO_SX1262=y`.

### On-board peripherals present but NOT wired here (for reference)

| Signal | GPIO | Notes |
|---|---|---|
| I2C SDA | 8 | SSD1306 OLED + BME280-style sensor header |
| I2C SCL | 7 | |
| User button | 0 | active-low (pull-up); board has six top buttons total |
| Debug LED | 1 | |
| NeoPixel data | 21 | single NeoPixel (see verify note — product blog says GPIO 17) |
| UART TX / RX | 43 / 44 | |

### Memory / flash (published from `kconfig:`)

A non-buildable straddle has no `sdkconfig.defaults` of its own — it would be
ignored under `--with` — so every value that describes this hardware is
published from `straddle.yaml`'s `kconfig:` block and consumed by the owning
straddle / IDF:

| Key | Value | Why |
|---|---|---|
| `CONFIG_ESPTOOLPY_FLASHSIZE_4MB` | `y` | 4 MB flash (ESP32-S3-Zero) — **verify** |
| `CONFIG_SPANGAP_MAX_FIRMWARE_KB` | `3400` | state floor at 3.32 MB: the ~2.55 MB reticulous binary fits the 0x290000 `app` slot below it with ~73 KB margin; `/state` fills the remaining ~696 KB. Without it `app` eats all 4 MB — leaving **no `/state`** |
| `CONFIG_SPIRAM_MODE_QUAD` | `y` | the S3FH4R2 carries 2 MB PSRAM in **quad** mode, not octal |

The platform's usual "octal PSRAM" assumption (the T-Deck's S3R8) does **not**
hold here — watch internal-DRAM headroom for DMA/WiFi/lwIP, as on the Heltec V4.

## Storage variables

This board defines no storage keys of its own. Runtime LoRa parameters live at
`s.lora.*` ([iface-lora](../iface-lora)).

## Dependencies

- [spangap-core](../spangap-core) — base runtime (storage, log, CLI, fs, ITS).
- [iface-lora](../iface-lora) — owns the SX1262 radio engine; this board parks
  its CS and supplies its pins via Kconfig.

## Read next

- [INTERNALS.md](INTERNALS.md) — the CS-park bring-up, the start-band ordering
  rule, and the board pitfalls.
