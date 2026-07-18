# hw-nibble-zero board HAL — an ordinary spangap component (not a buildable).
#
# Compiles the Retia Nibble Zero board code: the LoRa CS park (nibblezero.cpp).
# No on-device UI.
#
# app_main is NOT here — it's generated into the reticulous/reticulous buildable
# that adds this board with `--with`. This component just exports the board
# start: hook the generated dispatcher calls.
include(${CMAKE_CURRENT_LIST_DIR}/spangap_requires.cmake)
idf_component_register(
    SRCS
        "src/nibblezero.cpp"
        ${SPANGAP_CONDITIONAL_SRCS}
    INCLUDE_DIRS "include" ${SPANGAP_CONDITIONAL_INCLUDE_DIRS}
    REQUIRES
        ${SPANGAP_REQUIRES}
        driver
        esp_event
        esp_netif
        esp_wifi
        lwip
)
