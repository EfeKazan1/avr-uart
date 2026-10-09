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

uint8_t nmea_parsing(char *s, char *alan[],uint8_t max){

  //ilk önce ilk karakteri bir alan 0 a atalım.

  uint8_t alan_sayaci=0;

  alan[alan_sayaci++]=s;//ilk karakterden başlasın alan0

  //null ve * olmadıkça devam
  while(*s && *s!='*'){

    //virgül mü
    if(*s==','){
      //virgül yerine null karakteri koy

      *s='\0';

      //alan sayacını artıralım
      //alan sayacı 1 di alan sayacı1=nulldan sonrası ve artık alan sayacı 2 oldu.
      //alan sayacı kontrolü yapalım
      if(alan_sayaci<max) alan[alan_sayaci++]=s+1;

    }
    s++;//bir karakter kaydır.

  }
  //* karakterini de null yapalım
  *s='\0';
  return alan_sayaci;

}

void read_gps(char *alan[],uint8_t n,Gps_t *gps,Nmea_tipi *nmea_tipi){

      //şimdi hangi tipte olduğunu bulalım
      switch (*nmea_tipi) { //switch_case ile $den sonrasına bakıyoruz 

        case NMEA_RMC:
         
          if (n>1 && strlen(alan[1]) >= 6){ // 1. alan: "093015" → saat, dakika, saniye
            gps->saat = iki_basamak(alan[1]);       // "09"
            gps->dakika = iki_basamak(alan[1] + 2);   // "30"
            gps->saniye = iki_basamak(alan[1] + 4);   // "15"
          }
          if (n>9 && strlen(alan[9]) == 6 ){
          // 9. alan: "071026" → gün, ay, yıl
            gps->gun = iki_basamak(alan[9]);          // "07"
            gps->ay  = iki_basamak(alan[9] + 2);      // "10"
            gps->yil = iki_basamak(alan[9] + 4);      // "26"
          }

          gps->kuzey_guney = (n > 4 && alan[4][0]) ? alan[4][0] : '\0';
          gps->dogu_bati=(n>6 && alan[6][0]) ? alan[6][0] : '\0'; // 6. alan: "E" veya "W"
          gps->hareket_yonu=(n>8 && alan[8][0]) ? atoi(alan[8]) : 0xFFFF; // 8. alan: hareket yönü (derece)
          gps->mod=(n>12 && alan[12][0]) ? alan[12][0] : '\0'; // 12. alan: "A" otonom, "D" düzeltilmiş, "N" geçersiz

          gps->boylam=(n>5 && alan[5][0]) ? nmea_derece(atof(alan[5])) : NAN; // 5. alan: boylam (dddmm.mmmm)
          if (gps->dogu_bati   == 'W') gps->boylam = -gps->boylam;


          float ham = atof(alan[3]);          // 3436.22 olmalı
          float der = nmea_derece(ham);       // 34.603667 olmalı
          gps->enlem = der;
          if (gps->kuzey_guney == 'S') gps->enlem = -gps->enlem;   // -34.603667 olmalı

          char t[16];
          usart_send_data("ham=");   dtostrf(ham, 1, 4, t);          usart_send_data(t);
          usart_send_data(" der=");  dtostrf(der, 1, 6, t);          usart_send_data(t);
          usart_send_data(" son=");  dtostrf(gps->enlem, 1, 6, t);   usart_send_data(t);
          usart_send_data("\r\n");


          gps->hiz=(n>7 && alan[7][0]) ? atof(alan[7]) * 1.852 : 0xFFFF; //
          
          break;

        case NMEA_GGA: 
          
          if (n>1 && strlen(alan[1]) >= 6){
            gps->saat   = iki_basamak(alan[1]);       // "09"
            gps->dakika = iki_basamak(alan[1] + 2);   // "30"
            gps->saniye = iki_basamak(alan[1] + 4);   // "15"
          }
          gps->kuzey_guney=(n>3 && alan[3][0]) ? alan[3][0] : '\0'; // 3. alan: "N" veya "S"
          gps->enlem=(n>2 && alan[2][0]) ? nmea_derece(atof(alan[2])) : NAN; // 2. alan: enlem (ddmm.mmmm)
          gps->boylam=(n>4 && alan[4][0]) ? nmea_derece(atof(alan[4])) : NAN; // 4. alan: boylam (dddmm.mmmm)
          gps->dogu_bati=(n>5 && alan[5][0]) ? alan[5][0] : '\0'; // 5. alan: "E" veya "W"

          if (gps->kuzey_guney == 'S') gps->enlem  = -gps->enlem;
          if (gps->dogu_bati   == 'W') gps->boylam = -gps->boylam;

          gps->konum_kalite=(n>6 && alan[6][0]) ? alan[6][0] - '0' : 0; // 6. alan: konum kalitesi (0 yok, 1 GPS, 2 DGPS, 6 tahmini)
          gps->uydu_sayisi=(n>7 && alan[7][0]) ? atoi(alan[7]) : 0; // 7. alan: uydu sayısı
          gps->hdop=(n>8 && alan[8][0]) ? atof(alan[8]) : 0xFFFF; // 8. alan: HDOP (yatay hassasiyet)
          gps->yukseklik=(n>9 && alan[9][0]) ? atoi(alan[9]) : 0xFFFF; // 9. alan: yükseklik (deniz seviyesinden metre)
          break;

        case NMEA_GSA:
          gps->konum_tipi = (n > 2  && alan[2][0])  ? alan[2][0] - '0' : 0;
          gps->pdop       = (n > 15 && alan[15][0]) ? atof(alan[15]) : NAN;
          gps->hdop       = (n > 16 && alan[16][0]) ? atof(alan[16]) : NAN;
          gps->vdop       = (n > 17 && alan[17][0]) ? atof(alan[17]) : NAN;
          break;

        case NMEA_GLL:
          gps->kuzey_guney = (n > 2 && alan[2][0]) ? alan[2][0] : '\0';
          gps->dogu_bati   = (n > 4 && alan[4][0]) ? alan[4][0] : '\0';
          gps->enlem  = (n > 1 && alan[1][0]) ? nmea_derece(atof(alan[1])) : NAN;
          gps->boylam = (n > 3 && alan[3][0]) ? nmea_derece(atof(alan[3])) : NAN;
          if (gps->kuzey_guney == 'S') gps->enlem  = -gps->enlem;
          if (gps->dogu_bati   == 'W') gps->boylam = -gps->boylam;
          if (n > 5 && strlen(alan[5]) >= 6) {
              gps->saat   = iki_basamak(alan[5]);
              gps->dakika = iki_basamak(alan[5] + 2);
              gps->saniye = iki_basamak(alan[5] + 4);
          }
          gps->mod = (n > 7 && alan[7][0]) ? alan[7][0] : '\0';
          break;

        default:
          break;  
      }
  
}

uint8_t gps_detect(char *alan[],uint8_t n,Nmea_tipi *nmea_tipi){

  switch (*nmea_tipi) {
    case NMEA_RMC: return n > 2 && alan[2][0] == 'A'; //Burada n kaç tane alan çıktığıdır.Alan[2] 2.alanın ilk karakterinin adresidir. alan[2][0] ise 2. alanın ilk karakteridir. yani A mı V mi onu kontrol ediyoruz.
    case NMEA_GGA: return n > 6 && alan[6][0] && alan[6][0] != '0';
    case NMEA_GSA: return n > 2 && (alan[2][0] == '2' || alan[2][0] == '3');
    case NMEA_GLL: return n > 6 && alan[6][0] == 'A';
    default:       return 0;
  }

}uint8_t iki_basamak(const char *s) {
    return (s[0] - '0') * 10 + (s[1] - '0');
}

float nmea_derece(float ddmm) {
    int16_t derece = (int16_t)(ddmm / 100);   // 3750.88 → 37
    float dakika = ddmm - derece * 100;       // 3750.88 − 3700 = 50.88
    return derece + dakika / 60.0f;           // 37 + 0.848 = 37.848
}

uint8_t which_gps_mode(char *alan[]){//Alan dizisinin 0.elemanı burada \0 yani aslında virgüle kadar olan kısmın ilk elemanının adresidir. yani alan[0]=$ karakterinin adresidir. *(alan[0])=$ deriz. alan[0]+3 ise $ karakterinin adresinden 3 karakter sonrasının adresi olur. 
  //Burada NMEA_RMC, NMEA_GGA, NMEA_GSA, NMEA_GLL tiplerinden hangisiyle başlanldığını bulacağız 

  if(strcmp(alan[0] + 3,"RMC")==0) return NMEA_RMC;
  else if(strcmp(alan[0] + 3,"GGA")==0) return NMEA_GGA;
  else if(strcmp(alan[0] + 3,"GSA")==0) return NMEA_GSA;
  else if(strcmp(alan[0] + 3,"GLL")==0) return NMEA_GLL;
  else return NMEA_BILINMEYEN;

}