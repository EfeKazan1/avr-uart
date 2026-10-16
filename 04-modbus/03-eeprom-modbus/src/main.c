#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "uart.h"
#include "timer_config.h"
#include "modbus.h"
#include "crc.h"
#include "eeprom.h"

#define MAX_BOYUT 128
char buffer[MAX_BOYUT]; //UART'dan okunan karakterleri tutacak tampon

uint8_t cevap[15];

uint32_t simdi=0,simdims=0;

uint8_t n=0;

#define BAUD 115200
#define EE_SLAVE_ADDR 1000


int main(void){
 
  init_timer0();

  Usart.init(BAUD);

  Modbus.modbus_init(BAUD,eeprom_oku(EE_SLAVE_ADDR)); 

  uint8_t son_kayit=eeprom_oku(EE_SLAVE_ADDR);

  sei();

  DDRB |= (1<<DDB5) | (1<<DDB0) | (1<<DDB1) | (1<<DDB2) | (1<<DDB3) | (1<<DDB4);
  DDRD |= (1<<DDD6) | (1<<DDD7);


 
  while(1){

    simdi=getTickinMicro(); //anlık zaman

    simdims=getTickTime();
    PORTD|=(1<<PORTD7);

    if(son_kayit!=Modbus.modbus_adres()){
     
      eeprom_guncelle(EE_SLAVE_ADDR,Modbus.modbus_adres());
      son_kayit=Modbus.modbus_adres();

    }
    PORTD&=~(1<<PORTD7);

    while(Usart.ring_buffer_not_empty()){ //rb de bayt varsa
   
      Modbus.modbus_bayt_al(Usart.ring_buffer_read_byte(),simdi); // baytı oku eğer okunan bayt 8i aştıysa overflow olur bayt okunan saniyeyi alır 
    }

    PORTD |=  (1 << PORTD6);
    PORTD &= ~(1 << PORTD6);              // "isle'yi çağırmak üzereyim"
    n=Modbus.modbus_isle(simdi,cevap,simdims); //overflow yoksa 

    if(n) Usart.usart_send_data(cevap,n);

  

  }

  return 0;
}