#ifndef INTERRUPT_REGISTERING_H
#define INTERRUPT_REGISTERING_H

#include <stdint.h>
#include <avr/interrupt.h>

/******************REGISTERS ADRESSES**********************/
#define _REG_ADDR_EICRA 0x69
#define _REG_ADDR_EIMSK 0x3D
#define _REG_ADDR_EIFR  0x3C
#define _REG_ADDR_SREG  0x5F
#define _REG_ADDR_PCICR 0x35
#define _REG_ADDR_PCIFR 0x3B
#define _REG_ADDR_PCMSK 0x6B
#define _REG_ADDR_PCMSK0 0x6B
#define _REG_ADDR_PCMSK1 0x6C
#define _REG_ADDR_PCMSK2 0x6D


/*****************REGISTERS********************************/
#define _REG_SREG  *((volatile uint8_t*)_REG_ADDR_SREG)
#define _REG_EICRA *((volatile uint8_t*)_REG_ADDR_EICRA)
#define _REG_EIMSK *((volatile uint8_t*)_REG_ADDR_EIMSK)
#define _REG_EIFR *((volatile uint8_t*)_REG_ADDR_EIFR)
#define _REG_PCICR *((volatile uint8_t*)_REG_ADDR_PCICR)
#define _REG_PCIFR *((volatile uint8_t*)_REG_ADDR_PCIFR)
#define _REG_PCMSK0 *((volatile uint8_t*)_REG_ADDR_PCMSK0)
#define _REG_PCMSK1 *((volatile uint8_t*)_REG_ADDR_PCMSK1)
#define _REG_PCMSK2 *((volatile uint8_t*)_REG_ADDR_PCMSK2)


/*********************************************************/

/****************************PIN & MASKING*****************************/

/*EIFR PIN & MASKING*/
typedef struct{
  uint8_t s_INT0:1;
  uint8_t s_INT1:1;
  uint8_t reserved:6;
}EIFR_s;

#define _EIFR ((volatile EIFR_s*)_REG_ADDR_EIFR)
/******************************************************************************************/

/*EIMSK PIN & MASKING*/
typedef struct{
  uint8_t s_INT0:1;
  uint8_t s_INT1:1;
  uint8_t reserved:6;
}EIMSK_s;

#define _EIMSK ((volatile EIMSK_s*)_REG_ADDR_EIMSK) //0x3D adresine git EIMSK_STR pointerı ile ->INT0 ile adresinin ilk bitine eriş 
/******************************************************************************************/

/*EICRA  PIN & MASKING*/
typedef struct{
  uint8_t s_ISC_INT0:2;
  uint8_t s_ISC_INT1:2;
  uint8_t reserved:4;
}EICRA_s;

#define _EICRA ((volatile EICRA_s*)_REG_ADDR_EICRA)
/******************************************************************************************/


/*PCICR PIN & MASKING*/
typedef struct{
  uint8_t s_PCIE0:1;
  uint8_t s_PCIE1:1;
  uint8_t s_PCIE2:1;
  uint8_t reserved:5;
}PCICR_s;

#define _PCICR ((volatile PCICR_s*)_REG_ADDR_PCICR)
/******************************************************************************************/

/*PCIFR PIN & MASKING*/
typedef struct
{
  uint8_t s_PCIF0:1;
  uint8_t s_PCIF1:1;
  uint8_t s_PCIF2:1;
  uint8_t reserved:5;
}PCIFR_s;

#define _PCIFR ((volatile PCIFR_s*)_REG_ADDR_PCIFR)
/******************************************************************************************/

/*PCMSK PIN & MASKING*/
typedef struct
{
  uint8_t s_PCMSK0; //0x6B
  uint8_t s_PCMSK1; //0x6C
  uint8_t s_PCMSK2; //0x6D
}PCMSK_s;

#define _PCMSK ((volatile PCMSK_s*)_REG_ADDR_PCMSK0) //PCMSK0, PCMSK1 ve PCMSK2 aynı struct içinde tanımlanır çünkü ardışık adreslerde bulunurlar


/******************************************************************************************/

/*CONSTANTS*/

#define EICRA_LOW_LEVEL_ISC             0
#define EICRA_ANY_LOGICAL_LEVEL_ISC     1
#define EICRA_FALLING_EDGE_ISC          2
#define EICRA_RISING_EDGE_ISC           3

#define ENABLE    1

/******/


/******************************************/
#define sreg_interrupt_en() sei() //Pointer olduğu için 8 biti de kontrol ediyor
#define sreg_interrupt_dis() cli()

/*INTERRUPT ENABLE/DISABLE*/
#define INT0_EN() (_EIMSK->s_INT0=ENABLE)
#define INT1_EN() (_EIMSK->s_INT1=ENABLE)

/*INTERRUPT CONFIGURATION*/
#define EICRA_LOW_LEVEL_INT0() (_EICRA->s_ISC_INT0=0)
#define EICRA_ANY_LOGICAL_LEVEL_INT0() (_EICRA->s_ISC_INT0=1)
#define EICRA_FALLING_EDGE_INT0() (_EICRA->s_ISC_INT0=2)
#define EICRA_RISING_EDGE_INT0() (_EICRA->s_ISC_INT0=3)

#define EICRA_LOW_LEVEL_INT1() (_EICRA->s_ISC_INT1=0)
#define EICRA_ANY_LOGICAL_LEVEL_INT1() (_EICRA->s_ISC_INT1=1)
#define EICRA_FALLING_EDGE_INT1() (_EICRA->s_ISC_INT1=2)
#define EICRA_RISING_EDGE_INT1() (_EICRA->s_ISC_INT1=3)

/*INTERRUPT FLAG CLEARING*/
#define EIFR_IGNORE_INT0() (_EIFR->s_INT0=ENABLE)
#define EIFR_IGNORE_INT1() (_EIFR->s_INT1=ENABLE)

/*PCINT INTERRUPT ENABLE/DISABLE*/
#define PCIE0_EN() (_PCICR->s_PCIE0=ENABLE)
#define PCIE1_EN() (_PCICR->s_PCIE1=ENABLE)
#define PCIE2_EN() (_PCICR->s_PCIE2=ENABLE)

/*PINCHANGE INTERRUPT FLAG CLEARING*/
#define PCIFR_IGNORE_PCINT0() (_PCIFR->s_PCIF0=ENABLE)
#define PCIFR_IGNORE_PCINT1() (_PCIFR->s_PCIF1=ENABLE)
#define PCIFR_IGNORE_PCINT2() (_PCIFR->s_PCIF2=ENABLE)

#endif