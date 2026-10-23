#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "uart.h"
#include "timer_config.h"
#include "modbus.h"
#include "crc.h"
#include "eeprom.h"
#include "adc.h"

#define MAX_BOYUT 128
char buffer[MAX_BOYUT]; //UART'dan okunan karakterleri tutacak tampon

uint8_t cevap[21];

uint32_t simdi_us=0;

uint8_t n=0;

uint8_t adc_durum, adc_init_durum;

uint16_t adc_deger;
 
#define BAUD 115200
#define EE_SLAVE_ADDR 1000

void guncellik_callback(uint8_t yeni);

void pin_H(void);

void pin_L(void);

void pin_H(void){

  PORTD|=(1<<PORTD2);

}
void pin_L(void){

  PORTD&=~(1<<PORTD2);

}

void guncellik_callback(uint8_t yeni){

  eeprom_guncelle(EE_SLAVE_ADDR,yeni);

}



int main(void){
  DDRD |= (1<<DDD6) | (1<<DDD7) | (1<<DDD2) | (1<<DDD5) | (1<<DDD4);

  PORTD|=(1<<PORTD7); PORTD&=~(1<<PORTD7); //resetlenme kontrolü 

  init_timer0(adc_olcum_baslat_);

  Usart.init(BAUD,DATA_EVEN_PARITY_1STOP,getTickinMicro,pin_H,pin_L);

  Modbus.modbus_init(BAUD,eeprom_oku(EE_SLAVE_ADDR),guncellik_callback); 

  adc_init_durum=ADC0.init(PRESCALER_128,RIGHT_ADJUST,VREF_AVCC,ADC_0,ADC_INT_EN,getTickinMicro);

  if(adc_init_durum!=ADC_INIT_OK) {PORTD|=(1<<PORTD4);PORTD&=~(1<<PORTD4);} //olmadıysa hata döndür.
  
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


    uint8_t durum=ADC0.adc_isr_get_value(&adc_deger);

    if(durum==ADC_OK) Modbus.modbus_adc_al(&adc_deger);

    getTick_Overflow(&ms,&tasma_sayaci); //anlık ms ve taşma var mı taşma=49.7 gün

    simdi_us=(uint16_t)getTickinMicro(); //anlık zaman

    n=Modbus.modbus_isle(simdi_us,cevap,ms,tasma_sayaci); //overflow yoksa 

    if(n) Usart.usart_send_data(cevap,n);

  }

  return 0;
}