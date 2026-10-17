#ifndef MODBUS_H
#define MODBUS_H

// modbus.h
#include <stdint.h>
#include "crc.h"
#include <avr/io.h>
#include <string.h>

#define REGISTER_VALUE 6

typedef enum{

    GECERSIZ_FONK=0x01,
    GECERSIZ_ADRES=0x02,
    GECERSIZ_DEGER=0x03
};



typedef void (*AdresDegisti_t)(uint8_t yeni_adres);

typedef struct{
    void (*modbus_init)(uint32_t,uint8_t,AdresDegisti_t);
    void (*modbus_bayt_al)(uint8_t ,uint16_t );
    uint8_t (*modbus_isle)(uint16_t,uint8_t *,uint32_t,uint16_t);
    uint8_t (*modbus_adres)(void);

}Modbus_t;

extern const Modbus_t Modbus;

#endif