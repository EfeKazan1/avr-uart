#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <math.h>


typedef uint32_t (*Zaman_t)(void);

typedef enum{
   
    DATA_NO_PARITY_1STOP=0,
    DATA_EVEN_PARITY_1STOP=1,
    DATA_ODD_PARITY_1STOP=2,
    DATA_NO_PARITY_2STOP=3,
    DATA_EVEN_PARITY_2STOP=4,
    DATA_ODD_PARITY_2STOP=5,

}UsartMode_t;


typedef struct {
    void(*init)(uint32_t,UsartMode_t,Zaman_t);
    void (*usart_send_char)(char);
    void (*usart_send_number)(uint16_t);
    void (*usart_send_string)(const char *);
    void (*usart_send_data)(const uint8_t *,uint8_t);
    void (*ring_buffer_read_byte)(uint8_t *,uint16_t *);
    uint8_t (*ring_buffer_not_empty)(void);
    uint8_t (*usart_hatali_paket_sayisi)(void);

}Uart_t;

extern const Uart_t Usart;


#endif

