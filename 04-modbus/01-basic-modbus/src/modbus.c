#include "modbus.h"

static uint8_t bayt_sayaci=0,overflow_flag=0;

static uint8_t tampon[8];//Bizim verilerimizin bulunacağı tampon

static uint32_t son_byte_ms=0;

static uint16_t adet;

static uint16_t adres;

static uint8_t fonk_no;

static uint16_t registerlar[5]; 





void modbus_bayt_al(uint8_t bayt, uint32_t simdi){

    if(bayt_sayaci==8)overflow_flag=1; //8 olduysa taştı

    else tampon[bayt_sayaci++]=bayt; //byteı yazıyoruz. 

    //baytın yazıldığı an

    son_byte_ms=simdi;
    
}

void modbus_reg_guncelle(uint16_t *registerlar,uint32_t simdi){

    registerlar[0]=0xEFE1;

    //led durumu
    if(PINB & (1<<5)) registerlar[1]=1;
    else registerlar[1]=0;

    //anlık zaman 

    registerlar[2]=(uint16_t)(simdi>>16);

    registerlar[3]=(uint16_t)(simdi & 0xFFFF);

    registerlar[4] = 235;

}


uint8_t modbus_cevap_olustur(uint8_t adet,uint8_t *cevap){

    uint8_t i=0;

    cevap[0]=ADRES;cevap[1]=fonk_no;cevap[2]=adet*2;

    while(i<adet){
        //1 register bizden kaç register istendiyse o kadar gerekiyor 
        cevap[3+2*i] = (uint8_t)(registerlar[adres + i]>>8);

        cevap[4+2*i] = (uint8_t) (registerlar[adres +i] & 0xFF);

        i++;

    }

    uint16_t crc_hesaplanan= crc16(cevap,adet*2+3);


    cevap[3+adet*2]=(uint8_t)(crc_hesaplanan & 0xFF);
    cevap[4+adet*2]=(uint8_t)(crc_hesaplanan>>8);

    return 5+2*adet;


}

uint8_t modbus_hata_olustur(uint8_t *cevap, uint8_t hata_kodu,uint8_t fonk_no){


    cevap[0]=ADRES;
    cevap[1]=(fonk_no | 0x80);
    cevap[2]=hata_kodu;

    uint16_t crc_hesaplanan= crc16(cevap,3);

    cevap[3]=(uint8_t)(crc_hesaplanan & 0xFF);
    cevap[4]=(uint8_t)(crc_hesaplanan>>8);

    return 5;
}
uint8_t modbus_reg_yazdir(uint16_t *registerlar,uint16_t deger,uint16_t adres,uint8_t *cevap){

    registerlar[adres]=deger;

    memcpy(cevap,tampon,8);

    return 8;
}

void led_on_off(uint16_t deger){

    if(deger) PORTB|=(1<<5);

    else PORTB&=~(1<<5);

}


uint8_t modbus_isle(uint32_t simdi, uint8_t *cevap){

    //burada şimdi taştı mı taşmadı mı kontrolü yapıcaz

    //sadece paket tamam ise uzunluk yollayacağız 

    if(bayt_sayaci==0) return 0; 

    if(simdi - son_byte_ms <5) return 0;

    
    uint8_t uzunluk=bayt_sayaci;

    bayt_sayaci=0;

    //taştıysa 
    if(overflow_flag){ overflow_flag=0; return 0;} 

    //taşma bitti burdan çıkalım 
    
    //şimdi taşmamış ve paket bittiğine göre paketin uzunluğu 4 ten küçükse çöpe atalım

    if(uzunluk<4) return 0; 

    //şimdi en az 4 diyoruz ve Baş kısmının kontrolünü yapalım

    if(tampon[0] != ADRES) return 0; 

    //şimdi aynı zamanda adresi de kontrol etmiş olduk o zaman crc hesaplayalım

    uint16_t hesaplanan_crc=crc16(tampon,uzunluk-2); //burda tamponun ilk karakterinin bulunduğu adresten uzunluk -2 ye kadar crc hesaplayacak

    uint16_t gelen_crc = (uint16_t)(tampon[uzunluk-2] & 0xFF) | (uint16_t)(tampon[uzunluk-1] <<8) ;

    if(hesaplanan_crc != gelen_crc) return 0; //hesap yanlıştır

    //burada fonksiyon ve değer hata kontrolü yapılacak

    adet= (uint16_t)(tampon[4]<<8) | (uint16_t)(tampon[5]);

    adres=(uint16_t)(tampon[2]<<8) | (uint16_t)(tampon[3]);

    fonk_no= tampon[1];

    switch (fonk_no)
    {
    case 0x03:
        //Okuma fonksiyonu

        //okumada nerede hata çıkar 

        // eğer 0 adet reg okursan hata çıkar eğer olmayan regleri okumaya çalışırsan da hata çıkar

        if(adet==0) return modbus_hata_olustur(cevap,GECERSIZ_DEGER,tampon[1]);

        if((uint32_t)adres + adet > REGISTER_VALUE) return modbus_hata_olustur(cevap,GECERSIZ_ADRES,tampon[1]); 

        //hata yok şu an 

        //diyelim ki adres 2 den 2 tane bayt istiyor 

        //anlık durumumuzu güncelleyelim 
        modbus_reg_guncelle(registerlar,simdi);

        //şimdi bir cevap verelim 

        return modbus_cevap_olustur(adet,cevap);

        break;
        
    case 0x06:
                        // adr  no  bas_reg  deger   crcl  crch
        //yazma fonksiyonu EF | 06 | 00 01 | 00 00 | CRC_L| CRC_H | 


        if(adres != 1) return modbus_hata_olustur(cevap,GECERSIZ_ADRES,tampon[1]);
        if(!(adet==1 || adet==0)) return modbus_hata_olustur(cevap,GECERSIZ_DEGER,tampon[1]);

        led_on_off(adet);

        return modbus_reg_yazdir(registerlar,adet,adres,cevap);

        break;
    
    default:
        return modbus_hata_olustur(cevap, GECERSIZ_FONK, tampon[1]);
        break;
    }
    

    











    return 0;

}