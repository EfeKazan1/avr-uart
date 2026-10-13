#include "crc.h"

uint16_t crc16(const uint8_t *veri, uint8_t uzunluk) {
    uint16_t crc = 0xFFFF;

    for (uint8_t i = 0; i < uzunluk; i++) {
        crc ^= veri[i];
        for (uint8_t b = 0; b < 8; b++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}