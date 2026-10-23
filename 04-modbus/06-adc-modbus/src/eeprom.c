#include "eeprom.h"


uint8_t eeprom_oku(uint16_t adres){

    if (adres > 1023) return 0xFF;

    while(EECR & (1<<EEPE));

    EEARH = (uint8_t)(adres >> 8);     
    EEARL = (uint8_t)(adres & 0xFF);   

    EECR|=(1<<EERE);

    return EEDR;
}

void   eeprom_yaz(uint16_t adres, uint8_t veri){

    if (adres > 1023) return;

    while(EECR & (1<<EEPE));

    EEARH = (uint8_t)(adres >> 8);     
    EEARL = (uint8_t)(adres & 0xFF);   

    EEDR=veri;

    uint8_t sreg=SREG;

    cli();

    EECR|=(1<<EEMPE);

    EECR|=(1<<EEPE);

    SREG=sreg;

}


void eeprom_guncelle(uint16_t adres, uint8_t veri){

    if (adres > 1023) return;

    PORTB|=(1<<PORTB2);
    uint8_t eeprom_okunan=eeprom_oku(adres);
    PORTB&=~(1<<PORTB2);

    if(eeprom_okunan!=veri){
        
        PORTB|=(1<<PORTB3);
        eeprom_yaz(adres,veri);
        PORTB&=~(1<<PORTB3);

    }

}
