#ifndef MODBUS_H
#define MODBUS_H

// modbus.h
#include <stdint.h>
#include "crc.h"
#include <avr/io.h>
#include <string.h>

#define REGISTER_VALUE 5

#define ADRES 0xEF

typedef enum{

    GECERSIZ_FONK=0x01,
    GECERSIZ_ADRES=0x02,
    GECERSIZ_DEGER=0x03
};

void   modbus_bayt_al(uint8_t bayt, uint32_t simdi); //anlık byte okuyup bir sayac tutup taştı mı devam mı kontrolü ve tampona veri ekleme 
uint8_t modbus_isle(uint32_t simdi, uint8_t *cevap);

void modbus_reg_guncelle(uint16_t *registerlar,uint32_t simdi);

uint8_t modbus_reg_yazdir(uint16_t *registerlar,uint16_t deger,uint16_t adres,uint8_t *cevap);

uint8_t modbus_cevap_olustur(uint8_t adet,uint8_t *cevap);

uint8_t modbus_hata_olustur(uint8_t *cevap, uint8_t hata_kodu,uint8_t fonk_no);

#endif