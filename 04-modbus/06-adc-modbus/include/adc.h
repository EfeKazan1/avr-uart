#ifndef ADC_H
#define ADC_H


#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/*

ble 23-5. ADC Prescaler Selections
ADPS2 ADPS1 ADPS0 Division Factor
0 0 0 2
0 0 1 2
0 1 0 4
0 1 1 8
1 0 0 16
1 0 1 32
1 1 0 64
1 1 1 128
*/
typedef enum{
    PRESCALER_2=2,
    PRESCALER_4=4,
    PRESCALER_8=8,
    PRESCALER_16=16,
    PRESCALER_32=32,
    PRESCALER_64=64,
    PRESCALER_128=128
}AdcPrescaler_t;

typedef enum{
    LEFT_ADJUST,
    RIGHT_ADJUST
}LeftRightAdjust_t;

typedef enum{
    VREF_AREF,
    VREF_AVCC,
    VREF_1V1
}VoltageRef_t;

typedef enum{
    ADC_0,
    ADC_1,
    ADC_2,
    ADC_3,
    ADC_4
}AdChannel_t;

typedef enum{
    ADC_INT_EN,
    ADC_INT_DIS
}AdcInterrupt_t;



typedef enum { ADC_OK, ADC_KAPALI, ADC_HATA_ZAMAN_ASIMI,ADC_PRESCALER_HATA,ADC_INIT_OK,ADC_ZAMAN_FONKSIYONU_VERILMEDI,ADC_MESGUL} AdcDurum_t;

typedef uint32_t (*Zaman_al) (void);

 void adc_olcum_baslat_(void);



typedef struct{

    AdcDurum_t (*init)(AdcPrescaler_t,LeftRightAdjust_t,VoltageRef_t,AdChannel_t,AdcInterrupt_t,Zaman_al);
    AdcDurum_t (*get_value)(uint16_t *);
    AdcDurum_t (*read_adc)(uint16_t *);
    AdcDurum_t (*adc_isr_get_value)(uint16_t *);
    
}ADC_t;

extern const ADC_t ADC0;






#endif