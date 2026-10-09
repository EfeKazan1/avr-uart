#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "uart.h"
#include "nmea.h"
#include "timer_config.h"

#define MAX_BOYUT 128
char buffer[MAX_BOYUT]; //UART'dan okunan karakterleri tutacak tampon

Gps_t gps; //GPS verilerini tutacak yapı

typedef enum {
    GPS_VERI_YOK,     // hiç geçerli cümle gelmiyor
    GPS_FIX_YOK,      // cümle geliyor, konum geçersiz (V)
    GPS_FIX_VAR       // konum geçerli (A)
}GpsDurum_t;

uint32_t son_buffer_ms = 0; // son buffer alındığı zaman (ms)
uint32_t son_yazdirma_ms = 0; // son yazdırma zamanı (ms)

int main(void){

  usart_init(); 

  init_timer0(); // Timer başlat

  uint8_t reading,i=0;

  sei(); //Global interrupt enable

  GpsDurum_t gps_durumu= GPS_VERI_YOK; //Başlangıçta uydu yok
  ;


  while(1){

   // _delay_ms(150);

    while (rb_veri_var()) {
    reading = rb_oku();                                  // 1. halka tampondan al

    if (reading == '\r' || reading == '\n') {            // 2. satır bitti mi?
        if (i > 0) {
            buffer[i] = '\0';

            if (nmea_dogrula(buffer)) {                  // 3. önce doğrula
                usart_send_data("GECERLI:\r\n");
                son_buffer_ms = getTickTime(); // son buffer alındığı zamanı kaydet

                char *alan[20];
                uint8_t n = nmea_parsing(buffer, alan, 20);  //alan sayııs

                Nmea_tipi nmea_tipi= which_gps_mode(alan); //NMEA cümlesinin tipini bul

                //Şimdi uydu varsa diye başlayalım

                if(gps_detect(alan, n, &nmea_tipi))//Uydu varsa gps yapısına kaydet //if
                {
                  read_gps(alan, n, &gps,&nmea_tipi); //GPS verilerini oku ve gps yapısına kaydet eğer uydu varsa okur.

                  gps.son_fix_ms = getTickTime(); // son geçerli konumun geldiği zamanı kaydet
                  
                }    

            }

            else {
                usart_send_data("BOZUK\r\n"); //XOR Hata kontrolü
            }
            i = 0;
          }
        } 
        else if (i < MAX_BOYUT - 1) {
        buffer[i++] = reading;                           // satıra ekle
        }
        }

    uint32_t current_time = getTickTime();
    uint32_t buffer_verisi_taze= son_buffer_ms && (current_time - son_buffer_ms <= 7000); // son buffer alındığı zamandan itibaren 5 saniye geçti mi?
    uint32_t gps_verisi_taze= gps.son_fix_ms && (current_time - gps.son_fix_ms <= 5000); // son geçerli konumun geldiği zamandan itibaren 5 saniye geçti mi?
  
    GpsDurum_t yeni_gps_durumu;

    if(!buffer_verisi_taze) yeni_gps_durumu= GPS_VERI_YOK; // hiç geçerli cümle gelmiyor
    else if(!gps_verisi_taze) 
      yeni_gps_durumu= GPS_FIX_YOK; // cümle geliyor, konum geçersiz (V)
    else 
      yeni_gps_durumu= GPS_FIX_VAR; // konum geçerli (A)

    if(yeni_gps_durumu != gps_durumu){ // eğer gps durumu değiştiyse

      switch (yeni_gps_durumu) {
        case GPS_VERI_YOK:
            usart_send_data("GPS:VERI YOK (KABLO?)\r\n");
            break;
        case GPS_FIX_YOK:
            usart_send_data("GPS:UYDU ARANIYOR\r\n");
            break;
        case GPS_FIX_VAR:
            usart_send_data("GPS:KONUM VAR\r\n");
            
            break;
    }
        gps_durumu = yeni_gps_durumu; // yeni durumu kaydet
    
      }

      if (gps_durumu == GPS_FIX_VAR && (current_time - son_yazdirma_ms >= 1000)) {
        char t[16];
        usart_send_data("enlem=");   dtostrf(gps.enlem,  1, 6, t); usart_send_data(t);
        usart_send_data(" boylam="); dtostrf(gps.boylam, 1, 6, t); usart_send_data(t);
        usart_send_data(" hiz=");    dtostrf(gps.hiz,    1, 1, t); usart_send_data(t);
        usart_send_data(" km/s\r\n");
        son_yazdirma_ms = current_time;
      }
    }
}

