#ifndef TIMER_CONFIG_H
#define TIMER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Timer 0 yapılandırması ve fonksiyon prototipleri
 */
#include "timer0_registering.h"


#if __has_include("timer1_registering.h")
#include "timer1_registering.h"
#endif

#if __has_include("timer2_registering.h")
#include "timer2_registering.h"
#endif



/********************* TIMER0 Hassas Zaman ********************************/


/**
 * @brief Timer 0 CTC modunda başlatmak için gerekli fonksiyon prototipi ve sayaç değişkeni
 * 
 * @param void
 * @return void
 */
void init_timer0(void);

/**
 * @brief Timer 0 anlık zamanını milisaniye cinsinden döndüren fonksiyon prototipi
 * 
 * @param void
 * @return uint32_t Timer 0 anlık zamanını milisaniye cinsinden döndürür
 */
uint32_t getTickTime(void);

/**
 * @brief Timer 0 anlık zamanını mikrosaniye cinsinden döndüren fonksiyon prototipi
 * 
 * @param void
 * @return uint32_t Timer 0 anlık zamanını mikrosaniye cinsinden döndürür
 */

uint32_t getTickinMicro(void);

/********************** TIMER1 - HASSAS PWM / SERVO KÜTÜPHANESİ **********************/

extern volatile uint16_t ICR1_TOP_VALUE; //ICR1'in TOP değeri olarak kullanılacak global değişken

/**
 * @brief Timer1'i Fast PWM modunda (Mode 14 - ICR1 = TOP) başlatır.
 *        PB1 (OC1A) ve PB2 (OC1B) pinlerini PWM çıkışı yapmaya hazırlar.
 * 
 * 
 * @param Hz İstenen PWM frekansı (Hz cinsinden). Örn: 50Hz için 50 verilmelidir.
 * 8 prescaler ile 16MHz saat frekansında, 50Hz için OCR1A değeri 39999 olarak hesaplanır. (16MHz / (8 * 50Hz)) - 1 = 39999 OCR1A değerini fonksiyon içinde hesaplayarak ICR1'e atarız, böylece Timer1 her 50Hz'de bir sıfırlanır ve PWM sinyali üretilir.
 * 
 * eğer Hz 31 e eşit veya daha küçükse 8 prescale kullanılamaz 64 kullanılır 31Hz için OCR1A değeri 250.000/31=8064 olarak hesaplanır
 * 
 * 
 *                   
 */
void timer1_pwm_init(uint16_t Hz);

/**
 * @brief Timer1'in A kanalını başlatır
 *        PB1 (OC1A) pinini PWM çıkışı yapmaya hazırlar.
 *        DDRB registerında PB1 çıkış ayarlanırlır COM1A1 1 COM1A0 0 yapılır 
 * 
 * @param void 
 *                  
 */
void timer1_pwm_enable_channel_A(void);


/**
 * @brief Timer1'in A kanalını kapatır
 *        COM1A1 0 COM1A0 0 yapılır ki PWM çıkışı kapatılsın
 * 
 * @param void 
 *                  
 */
void timer1_pwm_disable_channel_A(void);


/**
 * @brief Timer1'in B kanalını başlatır
 *        PB2 (OC12) pinini PWM çıkışı yapmaya hazırlar.
 *        DDRB registerında PB2 çıkış ayarlanırlır COM1A1 1 COM1A0 0 yapılır 
 * 
 * @param void 
 *                  
 */
void timer1_pwm_enable_channel_B(void);

/**
 * @brief Timer1'in B kanalını kapatır
 *        COM1A1 0 COM1A0 0 yapılır ki PWM çıkışı kapatılsın
 * 
 * @param void 
 *                  
 */
void timer1_pwm_disable_channel_B(void);

/**
 * @brief Kanal A (PB1 / OC1A) pini için duty cycle (aktiflik) süresini ayarlar.
 * OCR1A registerına değeri yazarız o değer ICR x istenen duty cycle olur benim aslında timer1 pwm init fonksiyonundaki ICR değerine burda da ulaşabilmem lazım ki ona göre compare value'yu hesaplayıp OCR1A'ya atayabileyim, bu yüzden timer1_pwm_init fonksiyonunda ICR1 değerini global bir değişkende saklayalım ve burda o değere göre compare value'yu hesaplayarak OCR1A'ya atayalım. ve aslında compare value yerine duty cycle'ı yüzde cinsinden alalım, fonksiyon içinde o yüzdeye göre compare value'yu hesaplayarak OCR1A'ya atayalım, böylece kullanıcı sadece istediği duty cycle'ı yüzde cinsinden girebilir ve bizim fonksiyonumuz o değere göre compare value'yu hesaplayarak OCR1A'ya atar ve PWM sinyalinin duty cycle'ını ayarlar.
 * 
 * @param duty_cycle_x10 Sayaç eşleşme değeri (OCR1A) x 10 çünkü ondalıklı sayılar istemiyoruz .
 */

void timer1_pwm_set_duty_A(uint16_t duty_percentage_x10);
/**
 * @brief Kanal B (PB2 / OC1B) pini için duty cycle (aktiflik) süresini ayarlar.
 * 
 * @param duty_cycle Sayaç eşleşme değeri (OCR1B).
 */
void timer1_pwm_set_duty_B(uint16_t duty_percentage_x10);
#ifdef __cplusplus
}
#endif

#endif