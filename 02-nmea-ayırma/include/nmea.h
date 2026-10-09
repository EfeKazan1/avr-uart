#ifndef NMEA_H
#define NMEA_H
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    // 4 bayt
    float enlem, boylam;            // derece, güney/batı negatif
    float hiz;                      // km/s
    float hdop, pdop, vdop;
    float yukseklik;                // metre
    // 2 bayt
    uint16_t hareket_yonu;          // derece, 0xFFFF = bilinmiyor
    // 1 bayt
    uint8_t saat, dakika, saniye;
    uint8_t gun, ay, yil;
    char kuzey_guney, dogu_bati;    // 'N'/'S', 'E'/'W'
    char mod;                       // 'A', 'D', 'N'
    uint8_t konum_kalite;           // GGA: 0, 1, 2, 6
    uint8_t konum_tipi;             // GSA: 1, 2, 3
    uint8_t uydu_sayisi;
    uint8_t gps_id;
    uint32_t son_fix_ms;   // son geçerli konumun geldiği an (ms)
} Gps_t;

typedef enum{
  NMEA_BILINMEYEN=0,
  NMEA_RMC=1,
  NMEA_GGA=2,
  NMEA_GSA=3,
  NMEA_GLL=4
}Nmea_tipi;



uint8_t hex_deger(uint8_t c); //Bir karakterin hexadecimal değerini döndürür. Örn: 'A' -> 10, 'F' -> 15, '0' -> 0

uint8_t nmea_dogrula(const char *s); //NMEA cümlesinin doğruluğunu kontrol eder. Doğru ise 1, yanlış ise 0 döndürür.

uint8_t nmea_parsing(char *s, char *alan[],uint8_t max); //NMEA cümlesini alanlara ayırır ve alan sayısını döndürür.

void read_gps(char *alan[],uint8_t n,Gps_t *gps,Nmea_tipi *nmea_tipi);//NMEA cümlesinden GPS verilerini okur ve Gps_t yapısına kaydeder.


uint8_t iki_basamak(const char *s); //İki basamaklı bir sayıyı döndürür. Örn: "09" -> 9, "30" -> 30

float nmea_derece(float ddmm); //NMEA formatındaki derece ve dakika değerini ondalık dereceye çevirir. Örn: 3750.88 -> 37.848

uint8_t which_gps_mode(char *alan[]); //NMEA cümlesinin tipini döndürür. Örn: RMC, GGA, GSA, GLL

uint8_t gps_detect(char *alan[],uint8_t n,Nmea_tipi *nmea_tipi); //NMEA cümlesinde uydu verisi olup olmadığını kontrol eder. Varsa 1, yoksa 0 döndürür.

#endif