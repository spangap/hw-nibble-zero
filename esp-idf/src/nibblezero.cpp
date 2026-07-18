/**
 * nibblezero.cpp — Retia Nibble Zero board support, end to end.
 *
 * Single owner of all Nibble Zero hardware bring-up. See nibblezero.h for the
 * API contract and the board reference: https://retia.io/products/nibble-zero-kit.
 * Layout:
 *
 *   1. LoRa CS park.
 *      Always compiled. Driven from NibbleZeroBoard::onStart() before
 *      spangapInit().
 *
 * The SX1262 (Seeed Wio-SX1262 module) lives on its own SPI bus, separate from
 * the flash bus, and is powered directly — so unlike the T-Deck there is no
 * shared-bus SD probe to race, and unlike the Heltec V4 there is no Vext rail to
 * bring up. We still park the radio's CS HIGH before loraInit() owns it, so the
 * deselected radio doesn't drive MISO before its driver claims the pin.
 */
#include "nibblezero.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* =========================================================================
 * 1. LoRa CS park
 * ========================================================================= */

static void nibblezeroPowerInit(void)
{
    /* Park the SX1262's CS HIGH (deselected) so it doesn't drive MISO before
     * loraInit() claims the pin. The LoRa radio CS pin comes from iface-lora's
     * Kconfig (CONFIG_LORA0_CS_PIN); defined only when iface-lora is staged. */
#if defined(CONFIG_LORA0_CS_PIN)
    gpio_config_t cs = {};
    cs.pin_bit_mask = 1ULL << CONFIG_LORA0_CS_PIN;
    cs.mode         = GPIO_MODE_OUTPUT;
    cs.pull_up_en   = GPIO_PULLUP_DISABLE;
    cs.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cs.intr_type    = GPIO_INTR_DISABLE;
    gpio_config(&cs);
    gpio_set_level((gpio_num_t)CONFIG_LORA0_CS_PIN, 1);
#endif
}

/* =========================================================================
 * Public API — the always-on board bring-up (see nibblezero.h).
 * ========================================================================= */

void NibbleZeroBoard::onStart() {
    nibblezeroPowerInit();   /* LoRa CS park */
}
