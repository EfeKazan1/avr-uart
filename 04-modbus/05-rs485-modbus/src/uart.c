#include "uart.h"
#include <util/delay.h>

#define RB_BOYUT 128
volatile static uint8_t rb[RB_BOYUT];
volatile static uint8_t rb_bas = 0;      // kesme buraya yazar
volatile static uint8_t rb_kuyruk = 0;   // main buradan okur

volatile static uint8_t rbtx[RB_BOYUT];
volatile static uint8_t rb_head=0;
volatile static uint8_t rb_tail=0;

volatile static uint16_t rb_zaman[128];//128x2 = 256 byte 



static Zaman_t zaman_al; //fonksiyon adresi tutan değişken

static Pin_t pin_high;

static Pin_t pin_low;

static UsartMode_t mode;

static volatile uint8_t flag=1;

static void usart_init(uint32_t baud,UsartMode_t usartmode,Zaman_t zaman,Pin_t pinhighfonksiyonu,Pin_t pinlowfonksiyonu); // UART haberleşmesini başlatır static olma sebebi gözükmemesi gerek

static void usart_send_char(char c); // Tek bir karakter gönderir

static void usart_send_data(const char *a); // Bir karakter dizisini gönderir

static void usart_send_number(uint16_t number); // Bir sayıyı ASCII formatında gönderir

static void usart_send_bytes(const uint8_t *veri, uint8_t n); //Bilinen bir byte sayısında bir veri gönderir

static void rb_oku(uint8_t *bayt, uint16_t *zaman);// Ring buffer'dan bir karakter okur

static uint8_t rb_veri_var(void); // Ring buffer'da veri olup olmadığını kontrol eder




#define UBRR_DEGER_HESAPLAMA(baud) ((((F_CPU + 8UL * (baud))/(16UL * (baud))) - 1))

#define UBRR_DEGER_U2X_HESAPLAMA(baud) ((((F_CPU + 4UL * (baud))/(8UL * (baud))) - 1))




static void usart_init(uint32_t baud,UsartMode_t usartmode,Zaman_t zaman,Pin_t pinhighfonksiyonu,Pin_t pinlowfonksiyonu){

  pin_high=pinhighfonksiyonu;//pin_high static değişkeni şu an HIGH yapan fonksiyon adresini tutuyor. 

  pin_low=pinlowfonksiyonu;

  zaman_al=zaman;//zaman al şu an getTickinMicro fonksiyonun adresini tutuyor.

   uint16_t ubrr_deger=0;

  if (baud < 1200) baud = 1200;   // uint16 zaman damgası 65 ms'ye kadar güvenli

  switch (baud)
  {
  case 9600:
     UCSR0A = 0; // Normal speed
     ubrr_deger=UBRR_DEGER_HESAPLAMA(baud);
    break;
  case 115200:
    UCSR0A = (1<<U2X0);
    ubrr_deger=UBRR_DEGER_U2X_HESAPLAMA(baud);
    break;
  default:
    UCSR0A = 0; // Normal speed
    ubrr_deger=UBRR_DEGER_HESAPLAMA(baud);
    break;
  }

  uint8_t c = (1<<UCSZ01) | (1<<UCSZ00); // 8 bit veri, 1 stop biti, parite yok

  switch (usartmode)
  {
  case DATA_NO_PARITY_1STOP:    UCSR0C = c;  break;
  case DATA_EVEN_PARITY_1STOP:  UCSR0C=c | (1<<UPM01); break;
  case DATA_ODD_PARITY_1STOP:   UCSR0C= c | (1<<UPM00) | (1<<UPM01); break;
  case DATA_NO_PARITY_2STOP:    UCSR0C= c | (1<<USBS0); break;
  case DATA_EVEN_PARITY_2STOP:  UCSR0C = c | (1<<UPM01) | (1<<USBS0); break;
  case DATA_ODD_PARITY_2STOP:   UCSR0C = c | (1<<UPM00) | (1<<UPM01) | (1<<USBS0);  break;
  default:                      UCSR0C= c; usartmode= DATA_NO_PARITY_1STOP; break;
  }
  mode=usartmode;

  UBRR0H = (uint8_t)(ubrr_deger>>8); // Baud rate yüksek byte

  UBRR0L = (uint8_t)(ubrr_deger & 0xFF); // Baud rate düşük byte

  UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0) | (1<<TXCIE0); // Transmit ve receive enable, receive interrupt enable,TX kesmesi açık

}

const Uart_t Usart = {
    .init = usart_init, // .begin çağrıldığında otomatik usart_init çalışacak
    .usart_send_char=usart_send_char,
    .usart_send_number=usart_send_number,
    .usart_send_string=usart_send_data,
    .usart_send_data=usart_send_bytes,
    .ring_buffer_not_empty=rb_veri_var,
    .ring_buffer_read_byte=rb_oku
};

static void usart_send_char(char c){

  uint8_t sonraki = (rb_head + 1) % RB_BOYUT; // bir sonraki konum
  while (sonraki == rb_tail);      // tampon doluysa yer açılana kadar bekle
  rbtx[rb_head] = c;                    // karakteri heade koy
  rb_head = sonraki;    
  
  uint8_t sreg=SREG;
  cli();
  if(flag){flag=0; if(pin_high)pin_high();} //pin_high oldu  burda volatile oku değiştir yaz başka bişey bulamadım. 
  UCSR0A|=(1<<TXC0);
  PORTD |=  (1 << PORTD6);
  PORTD &= ~(1 << PORTD6);              // "isle'yi çağırmak üzereyim"
  SREG=sreg;
  UCSR0B |= (1 << UDRIE0);           // "UDR0 boşalınca haber ver" kesmesini aç
}

ISR(USART_UDRE_vect){

  PORTB|=(1<<PORTB1);
  PORTB&=~(1<<PORTB1);

  if(rb_head!=rb_tail){  // eğer tampon boş değilse
    UDR0=rbtx[rb_tail]; // karakteri UDR0'a yaz, yani gönder
    rb_tail = (rb_tail + 1) % RB_BOYUT; // kuyruk bir sonraki konuma geç
    
  }
  else{
    UCSR0B&=~(1<<UDRIE0); // eğer tampon boşsa kesmeyi kapat
  }
}

ISR(USART_TX_vect){
    if(pin_low) pin_low();
    PORTD|=(1<<PORTD7);PORTD&=~(1<<PORTD7);
    if(!flag) flag=1;
}

static void usart_send_data(const char *a){

  while(*a) usart_send_char(*a++); // karakter dizisini gönder

}

static void usart_send_number(uint16_t number){
  //char veri tipinde ascii ile işlem yapıcaksak hep char olmalı UTF-8 Formatında unsignedlar sıkıntılı
  char t[5];
  uint8_t i=0;//sayac

  do{
    t[i++]='0' + (number%10); //t dizisinin ilk elemanı 48+mesela 8 olsun 56 yani asciide char 8 karakteri olur i artar
    number/=10;//sayıyı 10 ile bölüyoruz
  }while(number>0);

  while(i>0)usart_send_char(t[--i]);//i kontrol edilir sonra bir azalıp işleme girer.
}


static void usart_send_bytes(const uint8_t *veri, uint8_t n) {
    for (uint8_t i = 0; i < n; i++) usart_send_char(veri[i]);
}


ISR(USART_RX_vect){
  //ilk önce okusun burda amacımız halka tampona entegre etmek byte ı 

  uint8_t durum = UCSR0A;
  uint8_t okunan=UDR0;

  if(!(durum & ((1<<UPE0) | (1<<DOR0) | (1<<FE0)))){ //tutuyorsa yazsın 
      
      uint32_t mikrous = zaman_al ? zaman_al() : 0;   // 1) önce saat

      uint8_t imlec = (rb_bas + 1 ) % RB_BOYUT; //imleç yani hep sonrakini gösterir
      
      //diyelim ki rb_bas 127.byte bu nedenle imlec 0a gelir ama alttaki kontrol ile okunmayan verinin üstüne yazmaz.
      if(imlec != rb_kuyruk){

        rb[rb_bas]=okunan; //halka tamponun head kısmında durdu

        rb_zaman[rb_bas]=(uint16_t)mikrous;

        rb_bas=imlec;

      }
  }
}
static void rb_oku(uint8_t *bayt, uint16_t *zaman){
  //burda kuyruktan çekecez

  *bayt=rb[rb_kuyruk];

  *zaman=rb_zaman[rb_kuyruk]; //rb_zaman rb sindeki okunan baytın bulunduğu zamanı döndürür.

  //kuyruk 0 olsun 1 oldu 2 oldu 3 oldu
  rb_kuyruk=(1+rb_kuyruk) % RB_BOYUT;

}

static uint8_t rb_veri_var(void){

  return rb_kuyruk!=rb_bas;
}










