// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>

// #include "uart.h"
// #include "timer_config.h"
// #include "binary_packet.h"
// #include "crc.h"

// #define MAX_BOYUT 128
// char buffer[MAX_BOYUT]; //UART'dan okunan karakterleri tutacak tampon

// typedef enum{
//   BAGLANTI_KUR,
//   BAGLANTI_BEKLE,
//   DURUM_SOR,
//   DURUM_BEKLE,
//   KOMUT_GONDER,
//   KOMUT_BEKLE,
//   KOMUTLAR_ARASI,
//   DURUM_SORGUSU_GITMEDI
// }StateMachine_t;

// uint32_t gonderim_ms = 0;

// int main(void){

//   usart_init(); 

//   init_timer0(); // Timer başlat

//   sei(); //Global interrupt enable

//   Paket_t paket;

//   uint8_t cikis[37];

//   StateMachine_t MasterState=BAGLANTI_KUR;

//   DDRB |= (1 << 5); 

//   volatile uint8_t paket_geldi=0;

//   uint8_t baglanti_sayaci=0;

//   volatile uint8_t ilk_defa =1;

//   volatile uint8_t led_durumu;

//   uint8_t led_acik_kapali;
  

//   while(1){

//    // _delay_ms(150);
//     uint32_t simdi=getTickTime();
//     paket_geldi=0;

//     if(rb_veri_var()){
//       if(binary_paket_oku(rb_oku(),&paket,getTickTime())){
//         paket_geldi=1;

//         if(paket.veri_tipi==TIP_LED_STATE) led_durumu=paket.veriler[0];
//       }
//     }

//         //YAZMA
//         switch (MasterState)
//         {
        
//           case BAGLANTI_KUR:{
        
//           uint8_t n=binary_paket_olustur(TIP_BEGIN,0,NULL,cikis); //AMAÇ: VERİSİZ BU DURUMU YOLLAMAK

//           usart_send_bytes(cikis,n);

//           MasterState=BAGLANTI_BEKLE;

//           gonderim_ms=simdi;

//           baglanti_sayaci=0;

//           ilk_defa=1;

//           break;}
        
//         case BAGLANTI_BEKLE:

//             if(paket_geldi && paket.veri_tipi==TIP_ACK){

//               baglanti_sayaci=0;
//               MasterState=DURUM_SOR;
//               gonderim_ms=simdi;
              
//             }

//             else if(simdi - gonderim_ms >=300)//300ms oldu direkt bağlantı kura gidelim  
//             { 
              
//               MasterState=BAGLANTI_KUR;

//             }

          
          
//           break;

//         case DURUM_SOR:{
            
//           uint8_t n=binary_paket_olustur(TIP_LED_INT,0,NULL,cikis); //AMAÇ: VERİSİZ BU DURUMU YOLLAMAK

//           usart_send_bytes(cikis,n);

//           MasterState=DURUM_BEKLE;

//           gonderim_ms=simdi;

//           break;}
        
//         case DURUM_BEKLE:

//           if(paket_geldi){

//             if(paket.veri_tipi==TIP_LED_STATE) MasterState=KOMUT_GONDER;

//             else if(paket.veri_tipi==TIP_LED_INT_HASNT_COME) MasterState=DURUM_SOR;

//             else if(paket.veri_tipi==TIP_LED_SLAVE_WANTS_BEGIN) MasterState=BAGLANTI_KUR;
          
//              baglanti_sayaci=0;
//              gonderim_ms=simdi;
//           }

//           else if(simdi - gonderim_ms >=100)//100ms oldu ve hala gelmediyse 
//             { 
//               baglanti_sayaci++;
//               gonderim_ms = simdi; 
//               MasterState=(baglanti_sayaci>=3) ? BAGLANTI_KUR : DURUM_SOR;

//             }


//           break;
        
//         case KOMUT_GONDER:{
//           if (ilk_defa) {
//           led_acik_kapali = !led_durumu;
//           ilk_defa = 0;
//           }
//           uint8_t n = binary_paket_olustur(TIP_LED_SET, 1, &led_acik_kapali, cikis);
//           usart_send_bytes(cikis, n);
//           MasterState = KOMUT_BEKLE;
//           gonderim_ms = simdi;
//           break;}
          

//         case KOMUT_BEKLE:


//         if(paket_geldi && paket.veri_tipi==TIP_LED_OK){
//               led_acik_kapali ^= 1;  
//               baglanti_sayaci=0;
//               MasterState=KOMUTLAR_ARASI; 
//               gonderim_ms=simdi;
              
//         }

//         else if(simdi - gonderim_ms >=100)//100ms oldu ve hala gelmediyse 
//         { 
//           baglanti_sayaci++;
//           gonderim_ms = simdi; 
//           MasterState=(baglanti_sayaci>=3) ? BAGLANTI_KUR : KOMUT_GONDER;

//         }
//           break;

//         case KOMUTLAR_ARASI:

//             if(simdi - gonderim_ms >=500) MasterState=KOMUT_GONDER;
            
//             break;
//         default:
//           break;

//         }
          
    
//     binary_paket_zaman_asimi(getTickTime());
//     }
// }
        
