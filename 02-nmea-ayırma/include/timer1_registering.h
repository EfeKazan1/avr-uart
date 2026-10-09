#ifndef TIMER1_REGISTERING_H
#define TIMER1_REGISTERING_H

#include <stdint.h>
#include "interrupt_registering.h"
/***************************** TIMER1 ***********************************/
#define _REG_ADDR_TCCR1 0x80 //TCCR1A 0x80, TCCR1B 0x81, TCCR1C 0x82 adreslerinde bulunur, bu yüzden tek bir pointer ile erişilir
#define _REG_ADDR_TIMSK 0x6F //TIMSK1 registeri 0x6F adresinde bulunur
#define REG_TIMSK1 (*(volatile uint8_t*)_REG_ADDR_TIMSK) //TIMSK1 registeri 0x6F adresinde bulunur
#define _REG_ADDR_TIFR1 0x36 //TIFR1 registeri 0x36 adresinde bulunur
#define REG_TIFR1 (*(volatile uint8_t*)_REG_ADDR_TIFR1) //TIFR1 registeri 0x36 adresinde bulunur

typedef struct{
    uint8_t s_TCCR1A; //TCCR1A registeri Offset: 0x00
    uint8_t s_TCCR1B; //TCCR1B registeri Offset: 0x01
    uint8_t s_TCCR1C; //TCCR1C registeri Offset: 0x02
    uint8_t s_reserved; //0x83 boş bir adres
    uint16_t s_TCNT1; //TCNT1 registeri   Offset: 0x04-0x05
    uint16_t s_ICR1; //ICR1 registeri    Offset: 0x06-0x07 (GERCEK donanim sirasi: TCNT1, ICR1, OCR1A, OCR1B)
    uint16_t s_OCR1A; //OCR1A registeri  Offset: 0x08-0x09
    uint16_t s_OCR1B; //OCR1B registeri  Offset: 0x0A-0x0B
}Timer1_Regs_t;

#define TIMER1 ((volatile Timer1_Regs_t*)_REG_ADDR_TCCR1) //TCCR1A ve TCCR1B ardışık adreslerde bulunur, bu yüzden tek bir pointer ile erişilir

typedef enum{
    TCCR1A_COM1A1_BIT = 7,
    TCCR1A_COM1A0_BIT = 6,
    TCCR1A_COM1B1_BIT = 5,
    TCCR1A_COM1B0_BIT = 4,
    TCCR1A_WGM11_BIT = 1,
    TCCR1A_WGM10_BIT = 0
} TCCR1A_Bits_t;

typedef enum{
    TCCR1B_FOC1A_BIT = 7,
    TCCR1B_FOC1B_BIT = 6,
    TCCR1B_WGM13_BIT = 4,
    TCCR1B_WGM12_BIT = 3,
    TCCR1B_CS12_BIT = 2,
    TCCR1B_CS11_BIT = 1,
    TCCR1B_CS10_BIT = 0
} TCCR1B_Bits_t;

typedef enum{
    TCCR1C_FOC1A_BIT = 7,
    TCCR1C_FOC1B_BIT = 6
} TCCR1C_Bits_t;
/******************** TIMER1 TCCRA CLOCK AYARI **************************/
/*
CS12 CS11 CS10 Description
0 0 0 No clock source (Timer/Counter stopped).
0 0 1 clkI/O/1 (no prescaling)
0 1 0 clkI/O/8 (from prescaler)
0 1 1 clkI/O/64 (from prescaler)
1 0 0 clkI/O/256 (from prescaler)
1 0 1 clkI/O/1024 (from prescaler)
1 1 0 External clock source on T1 pin. Clock on falling edge.
1 1 1 External clock source on T1 pin. Clock on rising edge.
*/

#define TIMER1_NO_CLK() do{TIMER1->s_TCCR1B&=~((1<<TCCR1B_CS12_BIT)|(1<<TCCR1B_CS11_BIT)|(1<<TCCR1B_CS10_BIT));}while(0)
#define TIMER1_CLK_ON_NO_PRESCALING() do{TIMER1->s_TCCR1B&=~((1<<TCCR1B_CS12_BIT)|(1<<TCCR1B_CS11_BIT)); TIMER1->s_TCCR1B|=(1<<TCCR1B_CS10_BIT);}while(0)
#define TIMER1_CLK_ON_PRESCALING_8() do{TIMER1->s_TCCR1B&=~((1<<TCCR1B_CS12_BIT)|(1<<TCCR1B_CS10_BIT)); TIMER1->s_TCCR1B|=(1<<TCCR1B_CS11_BIT);}while(0)
#define TIMER1_CLK_ON_PRESCALING_64() do{TIMER1->s_TCCR1B&=~(1<<TCCR1B_CS12_BIT); TIMER1->s_TCCR1B|=(1<<TCCR1B_CS11_BIT)|(1<<TCCR1B_CS10_BIT);}while(0)
#define TIMER1_CLK_ON_PRESCALING_256() do{TIMER1->s_TCCR1B&=~((1<<TCCR1B_CS11_BIT)|(1<<TCCR1B_CS10_BIT)); TIMER1->s_TCCR1B|=(1<<TCCR1B_CS12_BIT);}while(0)
#define TIMER1_CLK_ON_PRESCALING_1024() do{TIMER1->s_TCCR1B&=~(1<<TCCR1B_CS11_BIT); TIMER1->s_TCCR1B|=(1<<TCCR1B_CS12_BIT)|(1<<TCCR1B_CS10_BIT);}while(0)
#define TIMER1_CLK_ON_EXTERNAL_FALLING() do{TIMER1->s_TCCR1B&=~(1<<TCCR1B_CS10_BIT); TIMER1->s_TCCR1B|=(1<<TCCR1B_CS12_BIT)|(1<<TCCR1B_CS11_BIT);}while(0)
#define TIMER1_CLK_ON_EXTERNAL_RISING() do{TIMER1->s_TCCR1B|=(1<<TCCR1B_CS12_BIT)|(1<<TCCR1B_CS11_BIT)|(1<<TCCR1B_CS10_BIT);}while(0)

/********************** TIMER1 MOD AYARI **************************/

/*
 WGM13 WGM12 WGM11 WGM10 Mode of Operation TOP OCRA1A OCRA1B ICR1
0 0 0 0 0 Normal 0xFFFF Immediate MAX
1 0 0 0 1 PWM, phase correct, 8-bit 0x00FF TOP BOTTOM
2 0 0 1 0 PWM, phase correct, 9-bit 0x01FF TOP BOTTOM
3 0 0 1 1 PWM, phase correct, 10-bit 0x03FF TOP BOTTOM
4 0 1 0 0 CTC OCR1A Immediate MAX
5 0 1 0 1 Fast PWM, 8-bit 0x00FF BOTTOM TOP
6 0 1 1 0 Fast PWM, 9-bit 0x01FF BOTTOM TOP
7 0 1 1 1 Fast PWM, 10-bit 0x03FF BOTTOM TOP
8 1 0 0 0 PWM, phase and frequency
correct ICR1 BOTTOM BOTTOM
9 1 0 0 1 PWM, phase and frequency
correct OCR1A BOTTOM BOTTOM
10 1 0 1 0 PWM, phase correct ICR1 TOP BOTTOM
11 1 0 1 1 PWM, phase correct OCR1A TOP BOTTOM
12 1 1 0 0 CTC ICR1 Immediate MAX
13 1 1 0 1 (Reserved) – – –
14 1 1 1 0 Fast PWM ICR1 BOTTOM TOP
15 1 1 1 1 Fast PWM OCR1A BOTTOM TOP
*/

#define TIMER1_NORMAL_MODE() do{TIMER1->s_TCCR1A&=~((1<<TCCR1A_WGM11_BIT)|(1<<TCCR1A_WGM10_BIT)); TIMER1->s_TCCR1B&=~((1<<TCCR1B_WGM13_BIT)|(1<<TCCR1B_WGM12_BIT));}while(0)
#define TIMER1_CTC_OCR1A_MODE() do{TIMER1->s_TCCR1A&=~((1<<TCCR1A_WGM11_BIT)|(1<<TCCR1A_WGM10_BIT)); TIMER1->s_TCCR1B&=~(1<<TCCR1B_WGM13_BIT); TIMER1->s_TCCR1B|=(1<<TCCR1B_WGM12_BIT);}while(0)
#define TIMER1_CTC_ICR1_MODE() do{TIMER1->s_TCCR1A&=~((1<<TCCR1A_WGM11_BIT)|(1<<TCCR1A_WGM10_BIT)); TIMER1->s_TCCR1B|=(1<<TCCR1B_WGM12_BIT)|(1<<TCCR1B_WGM13_BIT);}while(0)
#define TIMER1_FAST_PWM_OCR1A_MODE() do{TIMER1->s_TCCR1A&=~((1<<TCCR1A_WGM11_BIT)|(1<<TCCR1A_WGM10_BIT)); TIMER1->s_TCCR1B&=~(1<<TCCR1B_WGM13_BIT); TIMER1->s_TCCR1B|=(1<<TCCR1B_WGM12_BIT);}while(0)
#define TIMER1_FAST_PWM_ICR1_MODE() do{TIMER1->s_TCCR1A&=~((1<<TCCR1A_WGM11_BIT)|(1<<TCCR1A_WGM10_BIT)); TIMER1->s_TCCR1B|=(1<<TCCR1B_WGM12_BIT)|(1<<TCCR1B_WGM13_BIT);}while(0)

/******************** TIMER1 TIMSK BITLERI **************************/
typedef struct{
    uint8_t s_TIMSK1;
}Timer1_Interrupt_Mask_Regs_t;

#define TIMER1_INTERRUPT_MASK ((volatile Timer1_Interrupt_Mask_Regs_t*)_REG_ADDR_TIMSK)

typedef enum{
    TIMSK1_ICIE1_BIT = 5,
    TIMSK1_OCIE1B_BIT = 2,
    TIMSK1_OCIE1A_BIT = 1,
    TIMSK1_TOIE1_BIT = 0
} TIMSK1_Bits_t;

#define TIMER1_OVERFLOW_INTERRUPT_ENABLE() (TIMER1_INTERRUPT_MASK->s_TIMSK1|=(1<<TIMSK1_TOIE1_BIT))
#define TIMER1_OVERFLOW_INTERRUPT_DISABLE() (TIMER1_INTERRUPT_MASK->s_TIMSK1&=~(1<<TIMSK1_TOIE1_BIT))
#define TIMER1_OUTPUT_COMPARE_A_INTERRUPT_ENABLE() (TIMER1_INTERRUPT_MASK->s_TIMSK1|=(1<<TIMSK1_OCIE1A_BIT))
#define TIMER1_OUTPUT_COMPARE_A_INTERRUPT_DISABLE() (TIMER1_INTERRUPT_MASK->s_TIMSK1&=~(1<<TIMSK1_OCIE1A_BIT))
#define TIMER1_OUTPUT_COMPARE_B_INTERRUPT_ENABLE() (TIMER1_INTERRUPT_MASK->s_TIMSK1|=(1<<TIMSK1_OCIE1B_BIT))
#define TIMER1_OUTPUT_COMPARE_B_INTERRUPT_DISABLE() (TIMER1_INTERRUPT_MASK->s_TIMSK1&=~(1<<TIMSK1_OCIE1B_BIT))
#define TIMER1_INPUT_CAPTURE_INTERRUPT_ENABLE() (TIMER1_INTERRUPT_MASK->s_TIMSK1|=(1<<TIMSK1_ICIE1_BIT))
#define TIMER1_INPUT_CAPTURE_INTERRUPT_DISABLE() (TIMER1_INTERRUPT_MASK->s_TIMSK1&=~(1<<TIMSK1_ICIE1_BIT))

/********************* TIMER1 TIFR BITLERI ***************************/

typedef struct{
    uint8_t s_TIFR1;
}Timer1_Flag_Regs_t;

#define TIMER1_FLAG ((volatile Timer1_Flag_Regs_t*)_REG_ADDR_TIFR1)

typedef enum{
    TIFR1_ICF1_BIT = 5,
    TIFR1_OCF1B_BIT = 2,
    TIFR1_OCF1A_BIT = 1,
    TIFR1_TOV1_BIT = 0
} TIFR1_Bits_t;

#define TIMER1_OVERFLOW_FLAG_CLEARED() (TIMER1_FLAG->s_TIFR1|=(1<<TIFR1_TOV1_BIT))
/****************************TIMER HESAPLAMALARI***********************************/



/*
F_CPU = 16000000Hz yani saniyede 16 milyon tık var
eğer prescaler kullanmazsak, timer her 1/16.000.000 saniyede bir artar yani 1 milisaniyede 16.000 tık artar
eğer prescaler olarak 8 kullanırsak, timer her 8/16.000.000 saniyede bir artar yani 1 milisaniyede 2000 tık artar
eğer prescaler olarak 64 kullanırsak, timer her 64/16.000.000 saniyede bir artar yani 1 milisaniyede 250 tık artar
eğer prescaler olarak 256 kullanırsak, timer her 256/16.000.000 saniyede bir artar yani 1 milisaniyede 62,5 tık artar
eğer prescaler olarak 1024 kullanırsak, timer her 1024/16.000.000 saniyede bir artar yani 1 milisaniyede 15,625 tık artar

Her tıkın sayısı ise -prescale 64 olsun yani ms de 250 tık atıyor- 1/250 milisaniye yani 4 mikrosaniyede  bir artar.
Bu değer -4 mikrosaniye- Çözünürlük olarak adlandırılır,timerın hassasiyetini gösterir.

Overflow için adım sayısını ise şöyle hesaplayalım istenen delay 1 sn olsun, prescaler 64 olsun yani ms de 250 tık
Adım sayısı= istenen delay/ her tıkın süresi(çözünürlük)= 1 sn/4 mikrosaniye= 1.000.000 mikrosaniye/4 mikrosaniye= 250.000 tık
yani timer 250.000 tık attığında 1 sn geçmiş olur,
biz burda overflow sayısını hesaplarız ve adım sayısını 256 değil 250 olarak alırız elle overflow ederiz yani
250.000 tık / 250 adım ise = 1000 overflow olursa 1 sn geçmiş olur

eğer software overflow compare yaparsak if bloklarıyla 249u görünce overflow sayacını 1 arttırırız ve 1000 olunca istediğimiz delay süresi geçmiş olur.

*/

#endif

