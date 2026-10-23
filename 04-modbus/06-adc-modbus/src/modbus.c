#include "modbus.h"

static uint8_t bayt_sayaci=0,overflow_flag=0;

static uint8_t tampon[8];//Bizim verilerimizin bulunacağı tampon

static uint16_t son_byte_us=0;

static uint16_t adet;

static uint16_t adres;

static uint8_t fonk_no;

static uint16_t registerlar[8]; 

static uint32_t t15_us;

static uint32_t t35_us;

static uint32_t byte_gidisi_us;

static uint8_t slave_adres=0xEF;

static uint16_t adc_gelen_deger=0;

static void   modbus_bayt_al(uint8_t bayt, uint16_t simdi); //anlık byte okuyup bir sayac tutup taştı mı devam mı kontrolü ve tampona veri ekleme 
static uint8_t modbus_isle(uint16_t simdi, uint8_t *cevap,uint32_t simdims,uint16_t tasma_sayaci);

static void modbus_reg_guncelle(uint16_t *registerlar,uint32_t simdims,uint8_t slave_adres,uint16_t tasma_sayaci);

static uint8_t modbus_reg_yazdir(uint16_t *registerlar,uint16_t deger,uint16_t adres,uint8_t *cevap);

static uint8_t modbus_cevap_olustur(uint8_t adet,uint8_t *cevap);

static uint8_t modbus_hata_olustur(uint8_t *cevap, uint8_t hata_kodu,uint8_t fonk_no);

static void modbus_init(uint32_t baud, uint8_t baslangic_adresi,AdresDegisti_t cb);

static void modbus_adc_al (uint16_t *adc);


//static uint8_t modbus_hata_sayma(void);

static inline void byte_gidis_suresi_us(uint32_t baud);

static inline uint8_t modbus_adres(void);

static AdresDegisti_t adres_cb=0;




static inline uint8_t modbus_adres(void){
    return slave_adres;

}

static inline void byte_gidis_suresi_us(uint32_t baud){

    byte_gidisi_us=(11UL * 1000000 / baud);

}

static void modbus_adc_al (uint16_t *adc){

    adc_gelen_deger=*adc;   
     
}

static void modbus_init(uint32_t baud,uint8_t baslangic_adresi,AdresDegisti_t cb){

    adres_cb=cb; //burda adres_cb gelen fonksiyonun adresini tutuyor. 

    if(baud>19200){
        t15_us=750; t35_us=1750;
    }
    else{
        uint32_t karakter = 11UL * 1000000UL / baud;   // 11bitlik karakter kaç us
        t35_us= karakter * 35 / 10;        // 3,5 yerine ×35/10: float kullanmadan
        t15_us= karakter * 15/10;
    }

    byte_gidis_suresi_us(baud);
    
    if (baslangic_adresi >= 1 && baslangic_adresi <= 247)
        slave_adres = baslangic_adresi;
    else
        slave_adres = 0xEF;
    
}

static void modbus_bayt_al(uint8_t bayt, uint16_t simdi){

    if(bayt_sayaci==8)overflow_flag=1; //8 olduysa taştı

    else if((bayt_sayaci!=0) && ((uint16_t)((simdi - son_byte_us) >t15_us + byte_gidisi_us))){
        overflow_flag=1;
        PORTB|=(1<<PORTB4);
        PORTB&=~(1<<PORTB4);
    } 

    else tampon[bayt_sayaci++]=bayt; //byteı yazıyoruz. 

    PORTB|=(1<<PORTB0);
    PORTB&=~(1<<PORTB0);
    //baytın yazıldığı an

    son_byte_us=simdi; //RXten okunan zaman . 
    
}

static void modbus_reg_guncelle(uint16_t *registerlar,uint32_t simdims,uint8_t slave_adres,uint16_t tasma_sayaci){

    registerlar[0]=0xEFE1;

    //led durumu
    if(PINB & (1<<5)) registerlar[1]=1;
    else registerlar[1]=0;

    //anlık zaman 

    registerlar[2] = tasma_sayaci;

    registerlar[3]=(uint16_t)(simdims>>16); //bu sefer micro sn oldu 

    registerlar[4]=(uint16_t)(simdims & 0xFFFF);

    registerlar[5]=slave_adres;

    registerlar[6] = adc_gelen_deger>>4; //ham değeri 16ya bölüyoruz 16 ölçüm yaptık

    uint16_t sicaklik_x10 = (uint32_t)adc_gelen_deger * 5000 / 16384;

    registerlar[7] = sicaklik_x10;

}


static uint8_t modbus_cevap_olustur(uint8_t adet,uint8_t *cevap){

    uint8_t i=0;

    cevap[0]=slave_adres;cevap[1]=fonk_no;cevap[2]=adet*2;

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

static uint8_t modbus_hata_olustur(uint8_t *cevap, uint8_t hata_kodu,uint8_t fonk_no){


    cevap[0]=slave_adres;
    cevap[1]=(fonk_no | 0x80);
    cevap[2]=hata_kodu;

    uint16_t crc_hesaplanan= crc16(cevap,3);

    cevap[3]=(uint8_t)(crc_hesaplanan & 0xFF);
    cevap[4]=(uint8_t)(crc_hesaplanan>>8);

    return 5;
}
static uint8_t modbus_reg_yazdir(uint16_t *registerlar,uint16_t deger,uint16_t adres,uint8_t *cevap){

    registerlar[adres]=deger;

    memcpy(cevap,tampon,8);

    return 8;
}

static void led_on_off(uint16_t deger){

    if(deger) PORTB|=(1<<5);

    else PORTB&=~(1<<5);

}


static uint8_t modbus_isle(uint16_t simdi, uint8_t *cevap,uint32_t simdims,uint16_t tasma_sayaci){

    //burada şimdi taştı mı taşmadı mı kontrolü yapıcaz

    //sadece paket tamam ise uzunluk yollayacağız 

    if(bayt_sayaci==0) return 0; 

    if((uint16_t)((simdi - son_byte_us) <t35_us  + byte_gidisi_us )) return 0;

    uint8_t uzunluk=bayt_sayaci;

    bayt_sayaci=0;

    //taştıysa 
    if(overflow_flag){ overflow_flag=0; return 0;} 

    //taşma bitti burdan çıkalım 
    
    //şimdi taşmamış ve paket bittiğine göre paketin uzunluğu 4 ten küçükse çöpe atalım

    if(uzunluk<4) return 0; 

    //şimdi en az 4 diyoruz ve Baş kısmının kontrolünü yapalım

    if(tampon[0] != slave_adres) return 0; 

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
        modbus_reg_guncelle(registerlar,simdims,slave_adres,tasma_sayaci);

        //şimdi bir cevap verelim 

        return modbus_cevap_olustur(adet,cevap);

        break;
        
    case 0x06:
                        // adr  no  bas_reg  deger   crcl  crch
        //yazma fonksiyonu EF | 06 | 00 01 | 00 00 | CRC_L| CRC_H | 


        if(adres==1){
            if(!((adet==0) || (adet==1))) return  modbus_hata_olustur(cevap,GECERSIZ_DEGER,tampon[1]);
            led_on_off(adet);
        } 
        else if(adres==5){
            if((adet==0 ) || (adet>247)) return modbus_hata_olustur(cevap,GECERSIZ_DEGER,tampon[1]);

            slave_adres=(uint8_t)adet;
            if(adres_cb) adres_cb(slave_adres);//adres_cb boş yer göstermiyorsa gösterdiği fonksiyonu slave_adresiyle çağırsın.

        }
        
        else return modbus_hata_olustur(cevap,GECERSIZ_ADRES,tampon[1]);

        return modbus_reg_yazdir(registerlar,adet,adres,cevap);

        break;
    
    default:
        return modbus_hata_olustur(cevap, GECERSIZ_FONK, tampon[1]);
        break;
    }

    return 0;

}

const Modbus_t Modbus ={
    .modbus_bayt_al=modbus_bayt_al,
    .modbus_init=modbus_init,
    .modbus_isle=modbus_isle,
    .modbus_adres=modbus_adres,
    .modbus_adc_al=modbus_adc_al
};