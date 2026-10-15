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

uint32_t simdi=0,simdims=0;

uint8_t n=0;

#define BAUD 1200


int main(void){
 
  init_timer0();

  Usart.init(BAUD);

  Modbus.modbus_init(BAUD);

  sei();

  DDRB |= (1<<DDB4) | (1<<DDB0) | (1<<DDB1) | (1<<DDB2) | (1<<DDB3) | (1<<DDB4);


 
  while(1){

    simdi=getTickinMicro(); //anlık zaman

    simdims=getTickTime();


    while(Usart.ring_buffer_not_empty()){ //rb de bayt varsa
      Modbus.modbus_bayt_al(Usart.ring_buffer_read_byte(),simdi); // baytı oku eğer okunan bayt 8i aştıysa overflow olur bayt okunan saniyeyi alır 
    }


    n=Modbus.modbus_isle(simdi,cevap,simdims); //overflow yoksa 

    if(n) Usart.usart_send_data(cevap,n);

     

  }

  return 0;
}