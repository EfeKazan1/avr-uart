#ifndef EEPROM_H
#define EEPROM_H


#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t eeprom_oku(uint16_t adres);
void    eeprom_yaz(uint16_t adres, uint8_t veri);
void eeprom_guncelle(uint16_t adres, uint8_t veri);



#endif