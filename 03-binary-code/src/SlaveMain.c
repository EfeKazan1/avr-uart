#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "uart.h"
#include "timer_config.h"
#include "binary_packet.h"
#include "crc.h"

#define MAX_BOYUT 128
char buffer[MAX_BOYUT]; //UART'dan okunan karakterleri tutacak tampon

typedef enum{
  BAGLANTI_BEKLE,
  BAGLANTI_ONAYI,
  DURUM_SORGUSU_BEKLENIYOR,
  DURUM_GONDER,
  KOMUT_BEKLENIYOR,
  KOMUT_ONAYLANDI,
  DURUM_SORGUSU_GELMEDI,
  BAGLANTI_ISTE
  
}StateMachine_t;

uint32_t gonderim_ms = 0;

int main(void){

  usart_init(); 

  init_timer0(); // Timer başlat

  sei(); //Global interrupt enable

  Paket_t paket;

  uint8_t cikis[37];

  StateMachine_t SlaveState=BAGLANTI_BEKLE;

  DDRB |= (1 << 5); 

  volatile uint8_t paket_geldi=0;

  uint8_t baglanti_sayaci=0;

  volatile uint8_t ilk_defa =1;

  volatile uint8_t led_durumu;

  volatile uint8_t baglanti_ilk_defa=1;

  while(1){

   // _delay_ms(150);
    uint32_t simdi=getTickTime();
    paket_geldi=0;

    if(rb_veri_var()){
      if(binary_paket_oku(rb_oku(),&paket,getTickTime())){
        paket_geldi=1;

        
      }
    }
    if (paket_geldi && paket.veri_tipi == TIP_BEGIN) {SlaveState = BAGLANTI_ONAYI;baglanti_sayaci=0;}
        //YAZMA
        switch (SlaveState)
        {
        
          case BAGLANTI_BEKLE:{

            if(baglanti_ilk_defa){
                gonderim_ms=simdi;
                baglanti_ilk_defa=0;
            }

          //BURADA 100MS BEKLE VERI GELMEZSE DİYEMEYİZ ÇÜNKÜ SİSTEM DAHA MASTERINDAN BAĞLANTI TALEBİ ALMADI

          if(paket_geldi && paket.veri_tipi==TIP_BEGIN){

            SlaveState=BAGLANTI_ONAYI;

            baglanti_ilk_defa=0;

            baglanti_sayaci=0;

          }

          else if(simdi - gonderim_ms >=100){
            baglanti_sayaci++;
            gonderim_ms = simdi; 

            if(baglanti_sayaci>=3) SlaveState=BAGLANTI_ISTE;
          }

          break;}
        
        case BAGLANTI_ONAYI:{

        //BURADA BİZ ACK GÖNDERMELİYİZ
        
         uint8_t n = binary_paket_olustur(TIP_ACK,0,NULL,cikis);

         usart_send_bytes(cikis,n);

         SlaveState=DURUM_SORGUSU_BEKLENIYOR;
          
         gonderim_ms=simdi; //gönderdik şimdi 100ms oldu mu kontrolü yapacağız 

         break;}

        case DURUM_SORGUSU_BEKLENIYOR:

            //100ms içinde
            if(paket_geldi && paket.veri_tipi==TIP_LED_INT){

                SlaveState=DURUM_GONDER;
                
                baglanti_sayaci=0;
            }
            else if(simdi - gonderim_ms >=100){ //ilk 100ms değiliz 

                baglanti_sayaci++;
                gonderim_ms = simdi; 
                SlaveState=(baglanti_sayaci>=3) ? BAGLANTI_ISTE : DURUM_SORGUSU_GELMEDI;

            }
            
            break;
        
        case BAGLANTI_ISTE:{

            uint8_t n=binary_paket_olustur(TIP_LED_SLAVE_WANTS_BEGIN,0,NULL,cikis);

            usart_send_bytes(cikis,n);

            SlaveState=BAGLANTI_BEKLE;

            baglanti_sayaci=0;

            gonderim_ms=simdi;

            break;

        }
            
        
        case DURUM_SORGUSU_GELMEDI:{

            uint8_t n= binary_paket_olustur(TIP_LED_INT_HASNT_COME,0,NULL,cikis);
            usart_send_bytes(cikis,n);

            SlaveState=DURUM_SORGUSU_BEKLENIYOR;
            gonderim_ms=simdi;

            break;}
        case DURUM_GONDER:{
            
            if(PINB & (1<<5)) led_durumu=1;
            else led_durumu=0;
               
            uint8_t n=binary_paket_olustur(TIP_LED_STATE,1,&led_durumu,cikis);

            usart_send_bytes(cikis,n);

            SlaveState=KOMUT_BEKLENIYOR;

            gonderim_ms=simdi;

            break;
        }
        
        case KOMUT_BEKLENIYOR:
          
          if(paket_geldi && paket.veri_tipi==TIP_LED_SET){


            //burada led ayarı yapmalıyız 

            if(paket.veriler[0]) PORTB|=(1<<5) ;

            else PORTB&=~(1<<5);

            SlaveState=KOMUT_ONAYLANDI;

            baglanti_sayaci=0;

          }

          else if(simdi - gonderim_ms >=600){ //BURADA MASTERDAN 500MS DE BİR KOMUT GELECEK O YÜZDEN EĞER 600 SN KOMUT GELMEZSE DİYELİM 

            baglanti_sayaci++;
            gonderim_ms = simdi; 
            if(baglanti_sayaci>=3) {
                SlaveState=BAGLANTI_BEKLE;
                baglanti_sayaci=0;}

            else SlaveState= DURUM_GONDER;
          
            }
          break;

        case KOMUT_ONAYLANDI:{
          
          int n=binary_paket_olustur(TIP_LED_OK,0,NULL,cikis);

          usart_send_bytes(cikis,n);

          gonderim_ms=simdi;

          SlaveState=KOMUT_BEKLENIYOR;

          break;}

        default:
          break;

        }
          
    
    binary_paket_zaman_asimi(getTickTime());
    }
}
        
