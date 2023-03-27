#ifndef CRC
#define CRC

#include <stdint.h>

uint8_t calculate_crc(void *input) {
    uint8_t *data = (uint8_t*)input;
    uint8_t crc = 0xFF;
    uint8_t polynomial = 0x31;

    for (int i = 0; i < 2; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
        crc &= 0xFF;
    }

    return crc;
}


int check_crc(uint8_t data[]) {
    uint8_t crc = 0xFF;
    uint8_t polynomial = 0x31;

    for (int i = 0; i < 3; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
        crc &= 0xFF;
    }

    return crc == 0;
}

#endif