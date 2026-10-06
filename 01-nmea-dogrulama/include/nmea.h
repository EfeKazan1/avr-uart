#ifndef NMEA_H
#define NMEA_H
#include <stdint.h>


uint8_t hex_deger(uint8_t c); //Bir karakterin hexadecimal değerini döndürür. Örn: 'A' -> 10, 'F' -> 15, '0' -> 0

uint8_t nmea_dogrula(const char *s); //NMEA cümlesinin doğruluğunu kontrol eder. Doğru ise 1, yanlış ise 0 döndürür.

#endif