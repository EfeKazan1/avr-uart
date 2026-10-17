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

uint8_t cevap[17];

uint32_t simdi_us=0;

uint8_t n=0;

#define BAUD 115200
#define EE_SLAVE_ADDR 1000

void guncellik_callback(uint8_t yeni);


void guncellik_callback(uint8_t yeni){

  eeprom_guncelle(EE_SLAVE_ADDR,yeni);

}

int main(void){
  DDRD |= (1<<DDD6) | (1<<DDD7);

  PORTD|=(1<<PORTD7); PORTD&=~(1<<PORTD7); //resetlenme kontrolü 

  init_timer0();

  Usart.init(BAUD,DATA_EVEN_PARITY_1STOP,getTickinMicro);

  Modbus.modbus_init(BAUD,eeprom_oku(EE_SLAVE_ADDR),guncellik_callback); 
  
  sei();

  DDRB |= (1<<DDB5) | (1<<DDB0) | (1<<DDB1) | (1<<DDB2) | (1<<DDB3) | (1<<DDB4);
  
  uint16_t tasma_sayaci=0;

  uint32_t ms=0;

  while(1){

    

    while(Usart.ring_buffer_not_empty()){ //rb de bayt varsa

      uint8_t b; uint16_t t; //b=okunan bayt t=baytın okunduğu saniye 

      Usart.ring_buffer_read_byte(&b,&t);

      Modbus.modbus_bayt_al(b,t); // baytı oku eğer okunan bayt 8i aştıysa overflow olur bayt okunan saniyeyi alır 

    }
      
    PORTD |=  (1 << PORTD6);
    PORTD &= ~(1 << PORTD6);              // "isle'yi çağırmak üzereyim"

    getTick_Overflow(&ms,&tasma_sayaci);

    simdi_us=(uint16_t)getTickinMicro(); //anlık zaman

    n=Modbus.modbus_isle(simdi_us,cevap,ms,tasma_sayaci); //overflow yoksa 

    if(n) Usart.usart_send_data(cevap,n);

  

  }

  return 0;
}