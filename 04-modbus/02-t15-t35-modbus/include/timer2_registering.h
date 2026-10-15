#ifndef TIMER2_REGISTERING_H
#define TIMER2_REGISTERING_H

#include <stdint.h>
#include "interrupt_registering.h"
/******************************** TIMER 2 ****************************************/

#define _REG_ADDR_TCCR2 0xB0 //TCCR2A 0xB0, TCCR2B 0xB1 adreslerinde bulunur, bu yüzden tek bir pointer ile erişilir
#define REG_TIMER2 (*(volatile uint16_t*)_REG_ADDR_TCCR2) //TCCR2A ve TCCR2B ardışık adreslerde bulunur, bu yüzden tek bir pointer ile erişilir
#define _REG_ADDR_TIFR2 0x37 //TIFR2 registeri 0x37 adresinde bulunur
#define REG_TIFR2 (*(volatile uint8_t*)_REG_ADDR_TIFR2)
#define _REG_ADDR_TIMSK2 0x70 //TIMSK2 registeri 0x70 adresinde bulunur
#define REG_TIMSK2 (*(volatile uint8_t*)_REG_ADDR_TIMSK2)


typedef struct{
    uint8_t s_TCCR2A; //TCCR2A registeri Offset: 0x00
    uint8_t s_TCCR2B; //TCCR2B registeri Offset: 0x01
    uint8_t s_TCNT2; //TCNT2 registeri   Offset: 0x02
    uint8_t s_OCR2A; //OCR2A registeri   Offset: 0x03
    uint8_t s_OCR2B; //OCR2B registeri   Offset: 0x04
}Timer2_Regs_t;

#ifdef TIMER2
#undef TIMER2
#endif

#define TIMER2 ((volatile Timer2_Regs_t*)_REG_ADDR_TCCR2) //TCCR2A ve TCCR2B ardışık adreslerde bulunur, bu yüzden tek bir pointer ile erişilir

/******************** TIMER2 TCCRA BITLERI **************************/
typedef enum{
    TCCR2A_COM2A1_BIT = 7,
    TCCR2A_COM2A0_BIT = 6,
    TCCR2A_COM2B1_BIT = 5,
    TCCR2A_COM2B0_BIT = 4,
    TCCR2A_WGM21_BIT = 1,
    TCCR2A_WGM20_BIT = 0
} TCCR2A_Bits_t;

/******************** TIMER2 TCCRB BITLERI *****************************/
typedef enum{
    TCCR2B_FOC2A_BIT = 7,
    TCCR2B_FOC2B_BIT = 6,
    TCCR2B_WGM22_BIT = 3,
    TCCR2B_CS22_BIT = 2,
    TCCR2B_CS21_BIT = 1,
    TCCR2B_CS20_BIT = 0
} TCCR2B_Bits_t;

/************************************************************************* */
typedef struct{
    uint8_t s_TIFR2;
}Timer2_Flag_Regs_t;

#define TIMER2_FLAG ((volatile Timer2_Flag_Regs_t*)_REG_ADDR_TIFR2)

typedef enum{
    TIFR2_OCF2B_BIT = 2,
    TIFR2_OCF2A_BIT = 1,
    TIFR2_TOV2_BIT = 0
} TIFR2_Bits_t;
/*******************************************************************/

typedef struct{
    uint8_t s_TIMSK2;
}Timer2_Interrupt_Mask_Regs_t;

typedef enum{
    TIMSK2_OCIE2B_BIT = 2,
    TIMSK2_OCIE2A_BIT = 1,
    TIMSK2_TOIE2_BIT = 0
} TIMSK2_Bits_t;

#define TIMER2_INTERRUPT_MASK ((volatile Timer2_Interrupt_Mask_Regs_t*)_REG_ADDR_TIMSK2)
/***************************** TIMER2 CONFIG *****************************/

/************************* TCCR2A BIT SET/CLR *****************************/
#define TIMER2A_COM2A1_SET() (TIMER2->s_TCCR2A|=(1<<TCCR2A_COM2A1_BIT))
#define TIMER2A_COM2A1_CLEAR() (TIMER2->s_TCCR2A&=~(1<<TCCR2A_COM2A1_BIT))
#define TIMER2A_COM2A0_SET() (TIMER2->s_TCCR2A|=(1<<TCCR2A_COM2A0_BIT))
#define TIMER2A_COM2A0_CLEAR() (TIMER2->s_TCCR2A&=~(1<<TCCR2A_COM2A0_BIT))
#define TIMER2A_COM2B1_SET() (TIMER2->s_TCCR2A|=(1<<TCCR2A_COM2B1_BIT))
#define TIMER2A_COM2B1_CLEAR() (TIMER2->s_TCCR2A&=~(1<<TCCR2A_COM2B1_BIT))
#define TIMER2A_COM2B0_SET() (TIMER2->s_TCCR2A|=(1<<TCCR2A_COM2B0_BIT))
#define TIMER2A_COM2B0_CLEAR() (TIMER2->s_TCCR2A&=~(1<<TCCR2A_COM2B0_BIT))

/*****************************TCCR2 MODE SELECTION *******************************/
#define TIMER2_NORMAL_MODE() do{TIMER2->s_TCCR2A&=~((1<<TCCR2A_WGM21_BIT)|(1<<TCCR2A_WGM20_BIT)); TIMER2->s_TCCR2B&=~(1<<TCCR2B_WGM22_BIT);}while(0)
#define TIMER2_CTC_MODE() do{TIMER2->s_TCCR2A&=~(1<<TCCR2A_WGM20_BIT); TIMER2->s_TCCR2A|=(1<<TCCR2A_WGM21_BIT); TIMER2->s_TCCR2B&=~(1<<TCCR2B_WGM22_BIT);}while(0)

/***************************** TIMER FLAG AND RESETTING COUNTER *******************************/
#define TIMER2_OVERFLOW_FLAG_CLEARED() (TIMER2_FLAG->s_TIFR2|=(1<<TIFR2_TOV2_BIT))
#define TIMER2_COUNTER_RESET() (TIMER2->s_TCNT2=6)

/****************************** TIMER ENABLE INTERRUPT ***************************************/
#define TIMER2_OVERFLOW_INTERRUPT_ENABLE() (TIMER2_INTERRUPT_MASK->s_TIMSK2|=(1<<TIMSK2_TOIE2_BIT))
#define TIMER2_OVERFLOW_INTERRUPT_DISABLE() (TIMER2_INTERRUPT_MASK->s_TIMSK2&=~(1<<TIMSK2_TOIE2_BIT))
#define TIMER2_OUTPUT_COMPARE_A_INTERRUPT_ENABLE() (TIMER2_INTERRUPT_MASK->s_TIMSK2|=(1<<TIMSK2_OCIE2A_BIT))
#define TIMER2_OUTPUT_COMPARE_A_INTERRUPT_DISABLE() (TIMER2_INTERRUPT_MASK->s_TIMSK2&=~(1<<TIMSK2_OCIE2A_BIT))
#define TIMER2_OUTPUT_COMPARE_B_INTERRUPT_ENABLE() (TIMER2_INTERRUPT_MASK->s_TIMSK2|=(1<<TIMSK2_OCIE2B_BIT))
#define TIMER2_OUTPUT_COMPARE_B_INTERRUPT_DISABLE() (TIMER2_INTERRUPT_MASK->s_TIMSK2&=~(1<<TIMSK2_OCIE2B_BIT))
/**************************** TCCR2 CLOCK SELECTION *******************************/
/*
0 0 0 No clock source (Timer/Counter stopped).
0 0 1 clkT2S/(no prescaling)
0 1 0 clkT2S/8 (from prescaler)
0 1 1 clkT2S/32 (from prescaler)
1 0 0 clkT2S/64 (from prescaler)
1 0 1 clkT2S/128 (from prescaler)
1 1 0 clkT2S/256 (from prescaler)
1 1 1 clkT2S/1024 (from prescaler)
*/

#define TIMER2_NO_CLK() do{TIMER2->s_TCCR2B&=~((1<<TCCR2B_CS22_BIT)|(1<<TCCR2B_CS21_BIT)|(1<<TCCR2B_CS20_BIT));}while(0)
#define TIMER2_CLK_ON_NO_PRESCALING() do{TIMER2->s_TCCR2B&=~((1<<TCCR2B_CS22_BIT)|(1<<TCCR2B_CS21_BIT)); TIMER2->s_TCCR2B|=(1<<TCCR2B_CS20_BIT);}while(0)
#define TIMER2_CLK_ON_PRESCALING_8() do{TIMER2->s_TCCR2B&=~((1<<TCCR2B_CS22_BIT)|(1<<TCCR2B_CS20_BIT)); TIMER2->s_TCCR2B|=(1<<TCCR2B_CS21_BIT);}while(0)
#define TIMER2_CLK_ON_PRESCALING_32() do{TIMER2->s_TCCR2B&=~(1<<TCCR2B_CS22_BIT); TIMER2->s_TCCR2B|=(1<<TCCR2B_CS21_BIT)|(1<<TCCR2B_CS20_BIT);}while(0)
#define TIMER2_CLK_ON_PRESCALING_64() do{TIMER2->s_TCCR2B&=~((1<<TCCR2B_CS21_BIT)|(1<<TCCR2B_CS20_BIT)); TIMER2->s_TCCR2B|=(1<<TCCR2B_CS22_BIT);}while(0)
#define TIMER2_CLK_ON_PRESCALING_128() do{TIMER2->s_TCCR2B|=((1<<TCCR2B_CS22_BIT)|(1<<TCCR2B_CS20_BIT)); TIMER2->s_TCCR2B&=~(1<<TCRR2B_CS21_BIT);}while(0)
#define TIMER2_CLK_ON_PRESCALING_256() do{TIMER2->s_TCCR2B&=~(1<<TCCR2B_CS20_BIT); TIMER2->s_TCCR2B|=(1<<TCCR2B_CS22_BIT)|(1<<TCCR2B_CS21_BIT);}while(0)
#define TIMER2_CLK_ON_PRESCALING_1024() do{TIMER2->s_TCCR2B|=((1<<TCCR2B_CS22_BIT)|(1<<TCCR2B_CS21_BIT)|(1<<TCCR2B_CS20_BIT));}while(0)


#endif