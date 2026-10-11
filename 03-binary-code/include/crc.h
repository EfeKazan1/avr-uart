#ifndef CRC_H
#define CRC_H

#include <stdint.h>

/**
 * CRC-16/MODBUS
 * Başlangıç: 0xFFFF, bölen: 0x8005 (aynalanmış hâli 0xA001)
 * Telde önce düşük bayt, sonra yüksek bayt gönderilir.
 *
 * Test değerleri:
 *   {0x01, 0x01, 0x01}  → 0x50E0
 *   {0x00, 0x02}        → 0x7180
 *   "123456789"         → 0x4B37  (standart kontrol değeri)
 */
uint16_t crc16(const uint8_t *veri, uint8_t uzunluk);

#endif