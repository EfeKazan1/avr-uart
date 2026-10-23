#include "adc.h"

static AdcDurum_t adc_init(AdcPrescaler_t prescaler,LeftRightAdjust_t adjust,VoltageRef_t voltageref,AdChannel_t channel, AdcInterrupt_t interrupt,Zaman_al zaman);

static AdcDurum_t deger_dondur(uint16_t *dondur);

static AdcDurum_t adc_oku(uint16_t *donus);

static AdcDurum_t adc_sonuc_al(uint16_t *deger);

static Zaman_al _zaman_=0;

static  uint8_t adc_okuma_sayaci=0;

static  uint16_t adc_okunanlar=0;

static volatile uint8_t hazir_flag=0;

static volatile uint16_t adc_sonuc=0;

static AdcDurum_t adc_init(AdcPrescaler_t prescaler,LeftRightAdjust_t adjust,VoltageRef_t voltageref,AdChannel_t channel, AdcInterrupt_t interrupt,Zaman_al zaman){

    //İlk önce F_CPU ya göre bir bölme yapalım 

    _zaman_=zaman;  //mikro fonksiyonunu tutan adres fonksiyonu
    
    if(!_zaman_) return ADC_ZAMAN_FONKSIYONU_VERILMEDI;

    if(F_CPU/prescaler>200000UL || F_CPU/prescaler<50000) return ADC_PRESCALER_HATA;

    //sadece 128 yapıcam

    //10-bit adc

    ADCSRA= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADEN);

    if(interrupt==ADC_INT_EN) ADCSRA|=(1<<ADIE);

    //adc kanalı için 

    uint8_t chnl=(channel & 0x0F);

    if(voltageref==VREF_AVCC) ADMUX= chnl | (1<<REFS0);

    else if(voltageref==VREF_1V1) ADMUX= chnl | (1<<REFS0) | (1<<REFS1);

    else ADMUX=chnl;

    if(adjust==LEFT_ADJUST) ADMUX|=(1<<ADLAR);
    
    else ADMUX &= ~(1<<ADLAR); //garanti olsun

    return ADC_INIT_OK;

}



void adc_olcum_baslat_(void){

    ADCSRA = (ADCSRA & ~(1<<ADIF)) | (1<<ADSC);

}


static AdcDurum_t adc_oku(uint16_t *donus){

    //ADC okumasını aç 

    if(!(ADCSRA & (1<<ADEN))) return ADC_KAPALI;

    ADCSRA |= (1<<ADSC);

    //okurken bekle 

    uint32_t mikrous= (_zaman_) ? _zaman_() : 0; //anlık zaman

    while(1){

        if(_zaman_() - mikrous <500){ //500 mikro saniye içinde adsc low ise yazdır
            if(!(ADCSRA & (1<<ADSC))){

                *donus=ADC;

                return ADC_OK;
            }
        }
        else return ADC_HATA_ZAMAN_ASIMI;

    }
}

static AdcDurum_t adc_oku_flagli(uint16_t *donus){

    if(!(ADCSRA & (1<<ADEN))) return ADC_KAPALI;

    ADCSRA |= (1<<ADIF); //bayrağı temizle. 

    ADCSRA = (ADCSRA & ~(1<<ADIF)) | (1<<ADSC);

    uint32_t mikrous= (_zaman_) ? _zaman_() : 0; //anlık zaman

    while(1){

        if(_zaman_() - mikrous <500){ //500 mikro saniye içinde adsc low ise yazdır
            if((ADCSRA & (1<<ADIF))){

                *donus=ADC;

                ADCSRA |= (1<<ADIF);

                return ADC_OK;
            }
        }
        else return ADC_HATA_ZAMAN_ASIMI;

    }
}

ISR(ADC_vect){

    //burada ölçülmüş bir adc değeri var ve adsc kapandı adıf sıfırlandı
    uint16_t gelen_adc=ADC;

    if(adc_okuma_sayaci!=16){

        adc_okunanlar+=gelen_adc;

        adc_okuma_sayaci++;

        ADCSRA = (ADCSRA & ~(1<<ADIF)) | (1<<ADSC); //ölçümü yeniden başlat

        PORTB|=(1<<PORTB1);
        PORTB&=~(1<<PORTB1);

    }
    else { //artık 16 okuma oldu.

        adc_sonuc= adc_okunanlar;

        hazir_flag=1;

        adc_okunanlar=0;

        adc_okuma_sayaci=0; //sayac sıfırlandı
    }

}

static AdcDurum_t adc_sonuc_al(uint16_t *deger){

    AdcDurum_t durum = ADC_MESGUL;

    uint8_t sreg=SREG;
    cli();

    if(hazir_flag){

        *deger=adc_sonuc;
        hazir_flag=0;
        durum=ADC_OK;

    }
    SREG=sreg;

    return durum;

}

static AdcDurum_t deger_dondur(uint16_t *dondur){

    uint16_t gelen,toplam=0;
    uint8_t hata_sayaci=0;

    for(uint8_t i=0;i<16;i++){ //16 okuma 

        uint8_t d=ADC0.read_adc(&gelen);

        if(d==ADC_OK) toplam+=gelen;

        else hata_sayaci++;

        if(hata_sayaci==3) return d; //veya okuma hatası da denilebilir

    }

    *dondur=toplam>>4;

    return ADC_OK;

}


const ADC_t ADC0 = {
    .init=adc_init,
    .get_value=deger_dondur,
    // .read_adc=adc_oku
    .read_adc=adc_oku_flagli,
    .adc_isr_get_value=adc_sonuc_al
};