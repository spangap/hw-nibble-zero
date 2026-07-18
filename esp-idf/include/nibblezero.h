/**
 * nibblezero.h — Retia Nibble Zero board support for reticulous.
 *
 * The Nibble Zero is built around the ESP32-S3-Zero module (ESP32-S3FH4R2:
 * 2 MB *quad* PSRAM, 4 MB flash) carrying one Semtech SX1262 (Seeed Wio-SX1262
 * module) on its own SPI bus. This first cut wires only the radio — no OLED,
 * no NeoPixel, no buttons, no GNSS (the "no LCD for now" build). See
 * nibblezero.cpp for the implementation and the board reference:
 * https://retia.io/products/nibble-zero-kit
 *
 * What this module provides:
 *   - The always-on board bring-up entry point NibbleZeroBoard::onStart().
 *
 * The Nibble Zero has no gated peripheral power rail (unlike the Heltec V4's
 * Vext), so there is no board-owned power pin here. The SX1262's pins
 * (NSS/SCK/MOSI/MISO/RST/BUSY/DIO1, TCXO, DIO2 RF switch) are NOT wired here:
 * they belong to iface-lora's CONFIG_LORA* knobs, set as board VALUES in this
 * straddle's straddle.yaml `kconfig:` block.
 */
#pragma once

#include "sdkconfig.h"
#include "service.h"

#define BOARD_NAME              "Retia Nibble Zero"

/**
 * Board bring-up, as a registered Service. NibbleZeroBoard::onStart is the
 * always-on hardware bring-up: it parks the LoRa radio's CS line HIGH so the
 * SX1262 doesn't drive MISO before the LoRa interface claims it. It runs in the
 * start band, before spangapInit(). There is no on-device UI in this build, so
 * there is no onInit companion.
 */
class NibbleZeroBoard : public Service {
public:
    void onStart() override;
};
