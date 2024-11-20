#include <sx1278.h>

constexpr Sx1278::config_t SX1278_CONFIG = {
    .dio0_pin = GPIO_NUM_17,
    .rst_pin = GPIO_NUM_16,
    .spi = {
        .host = SPI2_HOST,
        .clock_freq = 9 * 1000 * 1000,
        .mosi_pin = GPIO_NUM_13,
        .miso_pin = GPIO_NUM_12,
        .nss_pin = GPIO_NUM_15,
        .sck_pin = GPIO_NUM_14,
    },
};

const int BACKOFF_START_TIME_MS = 10;
const int BACKOFF_FACTOR = 2;
