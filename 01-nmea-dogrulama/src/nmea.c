#include "nmea.h"

uint8_t hex_deger(uint8_t c){
  //eğer sayı ise
  if(c>='0' && c<='9') return c - '0';
  //eğer büyük harf ise
  if(c>='A' && c<='F') return c - 'A' + 10;
  //eğer küçük harf ise
  if(c>='a' && c<='f') return c - 'a' + 10;
  return 0xFF; //geçersiz karakter
}

uint8_t nmea_dogrula(const char *s){ 

  //ilk önce $ ile başlıyor mu ona bakmalıyız.Sonra da pointerı ilerletiyoruz
  if(*s++!='$') return 0;
  //şimdi eğer null karakter değilse ve * ye ulaşmadıysak bir while döngüsü ile xor toplamı yapalım
  uint8_t xor_hata=0;
  while(*s && *s!='*') xor_hata ^= *s++;
  //eğer şimdi gelecek karakter * ise işleme devam edelim
  if(*s!='*') return 0;
  //ilk değeri s[1] ve ikincisini s[2] diye alabiliriz şu an s[0]=* ı göstermekte
  uint8_t high=hex_deger(s[1]);uint8_t low=hex_deger(s[2]);
  //şimdi bunları bir byte olcak şekilde alalım
  uint8_t kontrol_edilecek=(high<<4) | low;
  //şimdi kontrol edilecek ile hata_xor a bakalım aynı mı

  if(kontrol_edilecek==xor_hata) return 1;
  return 0;

}