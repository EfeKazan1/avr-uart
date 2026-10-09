#ifndef TIMER0_REGISTERING_H
#define TIMER0_REGISTERING_H

#include <stdint.h>
#include "interrupt_registering.h"

#define _REG_ADDR_TCCR0 0x44 //TCCR0A 0x44, TCCR0B 0x45 adreslerinde bulunur, bu yüzden tek bir pointer ile erişilir
#define REG_TIMER0 (*(volatile uint16_t*)_REG_ADDR_TCCR0) //TCCR0A ve TCCR0B ardışık adreslerde bulunur, bu yüzden tek bir pointer ile erişilir
#define _REG_ADDR_TIFR0 0x35 //TIFR0 registeri 0x35 adresinde bulunur
#define REG_TIFR0 (*(volatile uint8_t*)_REG_ADDR_TIFR0) //TIFR0 registeri 0x35 adresinde bulunur
#define _REG_ADDR_TIMSK0 0x6E //TIMSK0 registeri 0x6E adresinde bulunur
#define REG_TIMSK0 (*(volatile uint8_t*)_REG_ADDR_TIMSK0) //TIMSK0 registeri 0x6E adresinde bulunur



/**
 * @brief Timer0 registerlerini temsil eden struct ve ilgili makrolar:
 * Timer0, AVR mikrodenetleyicilerde bulunan 8-bit bir zamanlayıcıdır. Bu zamanlayıcı, çeşitli modlarda çalışabilir ve farklı uygulamalarda kullanılabilir. Timer0'ın registerlerini temsil eden struct ve ilgili makrolar, Timer0'ı yapılandırmak ve kullanmak için gereklidir.
 */
typedef struct{
    uint8_t s_TCCR0A; //TCCR0A registeri Offset: 0x00
    uint8_t s_TCCR0B; //TCCR0B registeri Offset: 0x01
    uint8_t s_TCNT0; //TCNT0 registeri   Offset: 0x02
    uint8_t s_OCR0A; //OCR0A registeri   Offset: 0x03
    uint8_t s_OCR0B; //OCR0B registeri   Offset: 0x04

}Timer0_Regs_t;


#define TIMER0 ((volatile Timer0_Regs_t*)_REG_ADDR_TCCR0) //TCCR0A ve TCCR0B ardışık adreslerde bulunur, bu yüzden tek bir pointer ile erişilir

/***************************** TIMER0 FLAG *********************************/
typedef struct{
    uint8_t s_TIFR0;
}Timer0_Flag_Regs_t;

// – – – – – OCF0B OCF0A TOV0

#define TIMER0_FLAG ((volatile Timer0_Flag_Regs_t*)_REG_ADDR_TIFR0) 


typedef enum{
    TIFR0_OCF0B_BIT = 2,
    TIFR0_OCF0A_BIT = 1,
    TIFR0_TOV0_BIT = 0
} TIFR0_Bits_t;

/******************** TIMER INTERRUPT MASK REGISTER *********************/
typedef struct{
    uint8_t s_TIMSK0;
}Timer0_Interrupt_Mask_Regs_t;

// – – – – – OCIE0B OCIE0A TOIE0 

#define TIMER0_INTERRUPT_MASK ((volatile Timer0_Interrupt_Mask_Regs_t*)_REG_ADDR_TIMSK0)

typedef enum{
    TIMSK0_OCIE0B_BIT = 2,
    TIMSK0_OCIE0A_BIT = 1,
    TIMSK0_TOIE0_BIT = 0
} TIMSK0_Bits_t;

/******************************TCCR0A Register Bitleri*********************************/
//                      COM0A1 COM0A0 COM0B1 COM0B0 –– WGM01 WGM00 

typedef enum{
    TCCR0A_COM0A1_BIT = 7,
    TCCR0A_COM0A0_BIT = 6,
    TCCR0A_COM0B1_BIT = 5,
    TCCR0A_COM0B0_BIT = 4,
    TCCR0A_WGM01_BIT = 1,
    TCCR0A_WGM00_BIT = 0
} TCCR0A_Bits_t;

/******************************TCCR0B Register Bitleri*********************************/
//                      FOC0A FOC0B –– WGM02 –– CS02 CS01 CS00

typedef enum{
    TCCR0B_FOC0A_BIT = 7,
    TCCR0B_FOC0B_BIT = 6,
    TCCR0B_WGM02_BIT = 3,
    TCCR0B_CS02_BIT = 2,
    TCCR0B_CS01_BIT = 1,
    TCCR0B_CS00_BIT = 0
} TCCR0B_Bits_t;

/************************* TCCR0A BIT SET/CLR *****************************/
#define TIMER0A_COM0A1_SET() (TIMER0->s_TCCR0A|=(1<<TCCR0A_COM0A1_BIT))
#define TIMER0A_COM0A1_CLEAR() (TIMER0->s_TCCR0A&=~(1<<TCCR0A_COM0A1_BIT))
#define TIMER0A_COM0A0_SET() (TIMER0->s_TCCR0A|=(1<<TCCR0A_COM0A0_BIT))
#define TIMER0A_COM0A0_CLEAR() (TIMER0->s_TCCR0A&=~(1<<TCCR0A_COM0A0_BIT))
#define TIMER0A_COM0B1_SET() (TIMER0->s_TCCR0A|=(1<<TCCR0A_COM0B1_BIT))
#define TIMER0A_COM0B1_CLEAR() (TIMER0->s_TCCR0A&=~(1<<TCCR0A_COM0B1_BIT))
#define TIMER0A_COM0B0_SET() (TIMER0->s_TCCR0A|=(1<<TCCR0A_COM0B0_BIT))
#define TIMER0A_COM0B0_CLEAR() (TIMER0->s_TCCR0A&=~(1<<TCCR0A_COM0B0_BIT))

/*****************************TCCR0 MODE SELECTION *******************************/
#define TIMER0_NORMAL_MODE() do{TIMER0->s_TCCR0A&=~((1<<TCCR0A_WGM01_BIT)|(1<<TCCR0A_WGM00_BIT)); TIMER0->s_TCCR0B&=~(1<<TCCR0B_WGM02_BIT);}while(0)
#define TIMER0_CTC_MODE() do{TIMER0->s_TCCR0A&=~(1<<TCCR0A_WGM00_BIT); TIMER0->s_TCCR0A|=(1<<TCCR0A_WGM01_BIT); TIMER0->s_TCCR0B&=~(1<<TCCR0B_WGM02_BIT);}while(0)

/***************************** TIMER FLAG AND RESETTING COUNTER *******************************/
#define TIMER0_OVERFLOW_FLAG_CLEARED() (TIMER0_FLAG->s_TIFR0|=(1<<TIFR0_TOV0_BIT))
#define TIMER0_COUNTER_RESET() (TIMER0->s_TCNT0=6)

/****************************** TIMER ENABLE INTERRUPT ***************************************/
#define TIMER0_OVERFLOW_INTERRUPT_ENABLE() (TIMER0_INTERRUPT_MASK->s_TIMSK0|=(1<<TIMSK0_TOIE0_BIT))
#define TIMER0_OVERFLOW_INTERRUPT_DISABLE() (TIMER0_INTERRUPT_MASK->s_TIMSK0&=~(1<<TIMSK0_TOIE0_BIT))
#define TIMER0_OUTPUT_COMPARE_A_INTERRUPT_ENABLE() (TIMER0_INTERRUPT_MASK->s_TIMSK0|=(1<<TIMSK0_OCIE0A_BIT))
#define TIMER0_OUTPUT_COMPARE_A_INTERRUPT_DISABLE() (TIMER0_INTERRUPT_MASK->s_TIMSK0&=~(1<<TIMSK0_OCIE0A_BIT))
#define TIMER0_OUTPUT_COMPARE_B_INTERRUPT_ENABLE() (TIMER0_INTERRUPT_MASK->s_TIMSK0|=(1<<TIMSK0_OCIE0B_BIT))
#define TIMER0_OUTPUT_COMPARE_B_INTERRUPT_DISABLE() (TIMER0_INTERRUPT_MASK->s_TIMSK0&=~(1<<TIMSK0_OCIE0B_BIT))
/**************************** TCCR0 CLOCK SELECTION *******************************/
/*
0 0 0 No clock source (Timer/Counter stopped)
0 0 1 clkI/O/(no prescaling)
0 1 0 clkI/O/8 (from prescaler)
0 1 1 clkI/O/64 (from prescaler)
1 0 0 clkI/O/256 (from prescaler)
1 0 1 clkI/O/1024 (from prescaler)
1 1 0 External clock source on T0 pin. Clock on falling edge.
1 1 1 External clock source on T0 pin. Clock on rising edge.
*/

#define TIMER0_NO_CLK() do{TIMER0->s_TCCR0B&=~((1<<TCCR0B_CS02_BIT)|(1<<TCCR0B_CS01_BIT)|(1<<TCCR0B_CS00_BIT));}while(0)
#define TIMER0_CLK_ON_NO_PRESCALING() do{TIMER0->s_TCCR0B&=~((1<<TCCR0B_CS02_BIT)|(1<<TCCR0B_CS01_BIT)); TIMER0->s_TCCR0B|=(1<<TCCR0B_CS00_BIT);}while(0)
#define TIMER0_CLK_ON_PRESCALING_8() do{TIMER0->s_TCCR0B&=~((1<<TCCR0B_CS02_BIT)|(1<<TCCR0B_CS00_BIT)); TIMER0->s_TCCR0B|=(1<<TCCR0B_CS01_BIT);}while(0)
#define TIMER0_CLK_ON_PRESCALING_64() do{TIMER0->s_TCCR0B&=~(1<<TCCR0B_CS02_BIT); TIMER0->s_TCCR0B|=(1<<TCCR0B_CS01_BIT)|(1<<TCCR0B_CS00_BIT);}while(0)
#define TIMER0_CLK_ON_PRESCALING_256() do{TIMER0->s_TCCR0B&=~((1<<TCCR0B_CS01_BIT)|(1<<TCCR0B_CS00_BIT)); TIMER0->s_TCCR0B|=(1<<TCCR0B_CS02_BIT);}while(0)
#define TIMER0_CLK_ON_PRESCALING_1024() do{TIMER0->s_TCCR0B&=~(1<<TCCR0B_CS01_BIT); TIMER0->s_TCCR0B|=(1<<TCCR0B_CS02_BIT)|(1<<TCCR0B_CS00_BIT);}while(0)
#define TIMER0_CLK_ON_EXTERNAL_FALLING() do{TIMER0->s_TCCR0B&=~(1<<TCCR0B_CS00_BIT); TIMER0->s_TCCR0B|=(1<<TCCR0B_CS02_BIT)|(1<<TCCR0B_CS01_BIT);}while(0)
#define TIMER0_CLK_ON_EXTERNAL_RISING() do{TIMER0->s_TCCR0B|=(1<<TCCR0B_CS02_BIT)|(1<<TCCR0B_CS01_BIT)|(1<<TCCR0B_CS00_BIT);}while(0)



#endif