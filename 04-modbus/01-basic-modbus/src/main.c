#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "uart.h"
#include "timer_config.h"
#include "modbus.h"
#include "crc.h"

#define MAX_BOYUT 128
char buffer[MAX_BOYUT]; //UART'dan okunan karakterleri tutacak tampon

uint8_t cevap[15];

uint32_t simdi=0;

uint8_t n=0;

int main(void){

  init_timer0();

  usart_init();

  sei();

  DDRB |= (1<<5);

  while(1){

    simdi=getTickTime(); //anlık zaman


    while(rb_veri_var()){ //rb de bayt varsa
      modbus_bayt_al(rb_oku(),simdi); // baytı oku eğer okunan bayt 8i aştıysa overflow olur bayt okunan saniyeyi alır 
    }


    n=modbus_isle(simdi,cevap); //overflow yoksa 

    if(n) usart_send_bytes(cevap,n);


  }

  return 0;
}