#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <math.h>


/**
 * @brief UART haberleşmesi için gerekli fonksiyonların prototipleri
 * 
 */

typedef struct {
    void(*init)(uint32_t);
    void (*usart_send_char)(char);
    void (*usart_send_number)(uint16_t);
    void (*usart_send_string)(const char *);
    void (*usart_send_data)(const uint8_t *,uint8_t);
    uint8_t (*ring_buffer_read_byte)(void);
    uint8_t (*ring_buffer_not_empty)(void);

}Uart_t;

extern const Uart_t Usart;


#endif

