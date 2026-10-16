#include "uart.h"


#define RB_BOYUT 128
volatile static uint8_t rb[RB_BOYUT];
volatile static uint8_t rb_bas = 0;      // kesme buraya yazar
volatile static uint8_t rb_kuyruk = 0;   // main buradan okur

volatile static uint8_t rbtx[RB_BOYUT];
volatile static uint8_t rb_head=0;
volatile static uint8_t rb_tail=0;

static void usart_init(uint32_t baud); // UART haberleşmesini başlatır static olma sebebi gözükmemesi gerek

static void usart_send_char(char c); // Tek bir karakter gönderir

static void usart_send_data(const char *a); // Bir karakter dizisini gönderir

static void usart_send_number(uint16_t number); // Bir sayıyı ASCII formatında gönderir

static void usart_send_bytes(const uint8_t *veri, uint8_t n); //Bilinen bir byte sayısında bir veri gönderir

static uint8_t rb_oku(void);// Ring buffer'dan bir karakter okur

static uint8_t rb_veri_var(void); // Ring buffer'da veri olup olmadığını kontrol eder


#define UBRR_DEGER_HESAPLAMA(baud) ((((F_CPU + 8UL * (baud))/(16UL * (baud))) - 1))

#define UBRR_DEGER_U2X_HESAPLAMA(baud) ((((F_CPU + 4UL * (baud))/(8UL * (baud))) - 1))


static void usart_init(uint32_t baud){

   uint16_t ubrr_deger=0;

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

  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // 8 bit veri, 1 stop biti, parite yok

  UBRR0H = (uint8_t)(ubrr_deger>>8); // Baud rate yüksek byte

  UBRR0L = (uint8_t)(ubrr_deger & 0xFF); // Baud rate düşük byte

  UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0); // Transmit ve receive enable, receive interrupt enable

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
  
  UCSR0B |= (1 << UDRIE0);           // "UDR0 boşalınca haber ver" kesmesini aç

}

ISR(USART_UDRE_vect){

  if(rb_head!=rb_tail){  // eğer tampon boş değilse
    UDR0=rbtx[rb_tail]; // karakteri UDR0'a yaz, yani gönder
    rb_tail = (rb_tail + 1) % RB_BOYUT; // kuyruk bir sonraki konuma geç
  }
  else{
    UCSR0B&=~(1<<UDRIE0); // eğer tampon boşsa kesmeyi kapat
  }
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
  uint8_t okunan=UDR0;
  uint8_t imlec = (rb_bas + 1 ) % RB_BOYUT; //imleç yani hep sonrakini gösterir.
  //diyelim ki rb_bas 127.byte bu nedenle imlec 0a gelir ama alttaki kontrol ile okunmayan verinin üstüne yazmaz.
  if(imlec != rb_kuyruk){
    rb[rb_bas]=okunan; //halka tamponun head kısmında durdu
    rb_bas=imlec; 
  }
  
}
static uint8_t rb_oku(void){
  //burda kuyruktan çekecez
  uint8_t okunan=rb[rb_kuyruk];

  //kuyruk 0 olsun 1 oldu 2 oldu 3 oldu
  rb_kuyruk=(1+rb_kuyruk) % RB_BOYUT;

  return okunan;
}

static uint8_t rb_veri_var(void){

  return rb_kuyruk!=rb_bas;
}










