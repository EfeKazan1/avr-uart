#ifndef _GPIO_REGISTERING_H
#define _GPIO_REGISTERING_H

#include <stdint.h>

/****************PORT ADRESSES OF B**********************/
#define _ADDR_GPIOB         0x23
#define _ADDR_PINB          0x23
#define _ADDR_DDRB          0x24
#define _ADDR_PORTB         0x25

/****************PORT ADRESSES OF C**********************/
#define _ADDR_GPIOC         0x26
#define _ADDR_PINC          0x26
#define _ADDR_DDRC          0x27
#define _ADDR_PORTC         0x28

/****************PORT ADRESSES OF D**********************/
#define _ADDR_GPIOD         0x29
#define _ADDR_PIND          0x29
#define _ADDR_DDRD          0x2A
#define _ADDR_PORTD         0x2B

/******************************************************/

/******************REGISTERS*************************/

#define PINB_REG       *((volatile uint8_t*)_ADDR_PINB)
#define DDRB_REG       *((volatile uint8_t*)_ADDR_DDRB)
#define PORTB_REG      *((volatile uint8_t*)_ADDR_PORTB)

#define PINC_REG       *((volatile uint8_t*)_ADDR_PINC)
#define DDRC_REG       *((volatile uint8_t*)_ADDR_DDRC)
#define PORTC_REG      *((volatile uint8_t*)_ADDR_PORTC)

#define PIND_REG       *((volatile uint8_t*)_ADDR_PIND)
#define DDRD_REG       *((volatile uint8_t*)_ADDR_DDRD)
#define PORTD_REG       *((volatile uint8_t*)_ADDR_PORTD)

/******************************************************/
/*****************STRUCT & UNION**************/
typedef struct{
    volatile uint8_t pin0:1;
    volatile uint8_t pin1:1;
    volatile uint8_t pin2:1;
    volatile uint8_t pin3:1;
    volatile uint8_t pin4:1;
    volatile uint8_t pin5:1;
    volatile uint8_t pin6:1;
    volatile uint8_t pin7:1;
}pin_t;

typedef union{
    pin_t pin;
    uint8_t value;
}_gpio_reg_t;

typedef struct{
    _gpio_reg_t PIN;
    _gpio_reg_t DDR;
    _gpio_reg_t PORT;
}_gpio_t;


/******************REGISTERS STRUCTS*************************/

/****************PORT ADRESSES OF B**********************/
#define GPIOB       ((volatile _gpio_t*)_ADDR_GPIOB)

/****************PORT ADRESSES OF C**********************/
#define GPIOC       ((volatile _gpio_t*)_ADDR_GPIOC)

/****************PORT ADRESSES OF D**********************/
#define GPIOD       ((volatile _gpio_t*)_ADDR_GPIOD)

/**********************VALUES*************************/

typedef enum{
    ALL_OUTPUT  = 0xFF,
    ALL_INPUT   = 0x00,
} ddr_val_e;

typedef enum{
    ALL_HIGH    = 0xFF,
    ALL_LOW     = 0x00,
    ALL_PULL_UP = 0xFF,
} port_val_e;

typedef enum{
    PIN_OUT  = 1,
    PIN_IN   = 0,
} pin_ddr_e;

typedef enum{
    PIN_HI      = 1,
    PIN_LO      = 0,
    PIN_PULL    = 1,
} pin_port_e;


#endif