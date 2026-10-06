#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>

#include "uart.h"
#include "nmea.h"


#define MAX_BOYUT 128
char buffer[MAX_BOYUT]; //UART'dan okunan karakterleri tutacak tampon


int main(void){

  usart_init(); 

  sei(); //Global interrupt enable

  uint8_t reading; //okunan karakteri tutacak değişken

  uint8_t i=0;

  while(1){

   // _delay_ms(150);

    while(rb_veri_var()){ //halka tampon boş değilse yani head!=tail

      reading=rb_oku(); //halka tampondan bir karakter oku

      if(reading=='\r' || reading=='\n'){ //OKUNAN BYTE SATIR SONU BYTE İSE

        if(i>0){
      
          buffer[i]='\0'; //null koyarak string bitirelim.
          if (nmea_dogrula(buffer)) usart_send_data("GECERLI: ");

          else                      usart_send_data("BOZUK:   ");

          //Sprintf ile buffer ın yanında ek karakterler de gönderebiliriz. Bunun için yeni buffer.
          char mesaj[100];

          sprintf(mesaj, "%s | %u byte uzunlugunda\r\n", buffer, i);
          usart_send_data(mesaj);

        
          i=0;

        }
      }
      else if(i<MAX_BOYUT - 1){ //EĞER TAMPON SONUNA ULAŞMADIYSAK VE SATIR SONU GELMEDİYSE

        buffer[i++]=reading; //TAMPONUN İ.KARAKTERİNE OKUNAN KARAKTERİ YAZ VE KARAKTERİ BİR ARTTIR.

      }

    }
  }
  
}


