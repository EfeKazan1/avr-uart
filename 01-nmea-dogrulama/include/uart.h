#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/**
 * @brief UART haberleşmesi için gerekli fonksiyonların prototipleri
 * 
 */

void usart_init(void); // UART haberleşmesini başlatır

void usart_send_char(char c); // Tek bir karakter gönderir

void usart_send_data(const char *a); // Bir karakter dizisini gönderir

void usart_send_number(uint16_t number); // Bir sayıyı ASCII formatında gönderir

//KULLANILMAYAN POLLİNG KARAKTER BAŞI 1.04 MS BEKLER
uint8_t usart_read_data(void); 

uint8_t rb_oku(void);// Ring buffer'dan bir karakter okur

uint8_t rb_veri_var(void); // Ring buffer'da veri olup olmadığını kontrol eder



#endif

