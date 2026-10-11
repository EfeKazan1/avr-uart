#include "timer_config.h"
#include "interrupt_registering.h"
#include <avr/interrupt.h>
#include "efe_config.h"

/****************************TIMER OUTPUT COMPARE PIN CONFIGURATION *******************************/

/* ============================================================
 * ORIJINAL TIMER0 CTC TABANLI IMPLEMENTASYON (YORUM SATIRI)
 * Not: Timer0 CTC modu, Arduino millis() fonksiyonunu bozuyordu.
 * E22 LoRa kutuphanesi millis() ile timeout yaptiginden lora.begin()
 * AUX LOW bekleme dongusuede sonsuz kilitleniyordu.
 * Cozum: getTickTime() -> millis() yonlendirildi, Timer0 Arduino OVF modunda kaldi.
 * ============================================================*/

static volatile uint32_t compare_match_counter = 0;

void init_timer0(void){
    TIMER0_CTC_MODE();
    TIMER0->s_OCR0A = 249;
    TIMER0_CLK_ON_PRESCALING_64();
    TIMER0_OUTPUT_COMPARE_A_INTERRUPT_ENABLE();
    
}

ISR(TIMER0_COMPA_vect){
    compare_match_counter++;
}

uint32_t getTickTime(void){
    uint32_t safe;

    uint8_t sreg = SREG;   // I biti dahil o anki durumu sakla
    sreg_interrupt_dis();
    safe = compare_match_counter;
    SREG = sreg; 
    
    return safe;
}

uint32_t getTickinMicro(void){
    uint32_t safe;
    uint8_t tcnt;
    uint8_t sreg = SREG;   // I biti dahil o anki durumu sakla
    sreg_interrupt_dis();
    tcnt = TIMER0->s_TCNT0;
    safe = compare_match_counter;
    if((TIMER0_FLAG->s_TIFR0 & (1<<TIFR0_OCF0A_BIT)) && (tcnt<249)){
        safe++;
        tcnt = TIMER0->s_TCNT0;
    }
    SREG = sreg;
    return ((safe*1000) + (tcnt<<2));
}

//* ============================================================ 

/* Arduino millis() C fonksiyonu - timer_config.c icinden cagirilabilir */
// extern unsigned long millis(void);

// /**
//  * @brief Milisaniye cinsinden sistem zamanini dondurur.
//  *        millis() uzerinden calisir; Timer0 Arduino OVF modunda kalir
//  *        boylece E22 LoRa kutuphanesi timeout mekanizmasi bozulmaz.
//  *        Tum getTickTime() cagrilari degismeden calisir.
//  */
// uint32_t getTickTime(void){
//     return (uint32_t)millis();
// }

// /**
//  * @brief Mikrosaniye cinsinden sistem zamanini dondurur.
//  *        millis() x 1000 ile yaklasik deger uretir.
//  */
// uint32_t getTickinMicro(void){
//     return (uint32_t)millis() * 1000UL;
// }

/****************************** TIMER1 PWM AYARLARI ***************************/


volatile uint16_t ICR1_TOP_VALUE=0; //ICR1'in TOP değeri olarak kullanılacak global değişken

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
void timer1_pwm_init(uint16_t Hz){
    if(Hz<=31){
        ICR1_TOP_VALUE=250000/Hz; //ICR1'in TOP değeri olarak hesaplanan değeri atıyoruz
        TIMER1_FAST_PWM_ICR1_MODE(); //Mode 14 - Fast PWM, TOP = ICR1
        TIMER1_CLK_ON_PRESCALING_64(); //Timer'ı 64 prescaler ile başlat
    }
    else{
        ICR1_TOP_VALUE=16000000/(8UL*Hz); //ICR1'in TOP değeri olarak hesaplanan değeri atıyoruz
        TIMER1_FAST_PWM_ICR1_MODE(); //Mode 14 - Fast PWM, TOP = ICR1
        TIMER1_CLK_ON_PRESCALING_8(); //Timer'ı 8 prescaler ile başlat
    }
    TIMER1->s_ICR1 = ICR1_TOP_VALUE -1 ; //ICR1 registerine TOP değeri olarak atıyoruz
}

/**
 * @brief Timer1'in A kanalını başlatır
 *        PB1 (OC1A) pinini PWM çıkışı yapmaya hazırlar.
 *        DDRB registerında PB1 çıkış ayarlanırlır COM1A1 1 COM1A0 0 yapılır 
 * 
 * @param void 
 *                  
 */
void timer1_pwm_enable_channel_A(void){
    GPIOB->DDR.pin.pin1= PIN_OUT;

    TIMER1->s_TCCR1A|=(1<<TCCR1A_COM1A1_BIT); //COM1A1 1 yapılır
    TIMER1->s_TCCR1A&=~(1<<TCCR1A_COM1A0_BIT); //COM1A0 0 yapılır
}


/**
 * @brief Timer1'in A kanalını kapatır
 *        COM1A1 0 COM1A0 0 yapılır ki PWM çıkışı kapatılsın
 * 
 * @param void 
 *                  
 */
void timer1_pwm_disable_channel_A(void){
    TIMER1->s_TCCR1A&=~((1<<TCCR1A_COM1A1_BIT)|(1<<TCCR1A_COM1A0_BIT)); //COM1A1 COM1A0 0 yapılır
}

/**
 * @brief Timer1'in B kanalını başlatır
 *        PB2 (OC12) pinini PWM çıkışı yapmaya hazırlar.
 *        DDRB registerında PB2 çıkış ayarlanırlır COM1A1 1 COM1A0 0 yapılır 
 * 
 * @param void 
 *                  
 */
void timer1_pwm_enable_channel_B(void){
     GPIOB->DDR.pin.pin2= PIN_OUT;

    TIMER1->s_TCCR1A|=(1<<TCCR1A_COM1B1_BIT); //COM1A1 1 yapılır
    TIMER1->s_TCCR1A&=~(1<<TCCR1A_COM1B0_BIT); //COM1A0 0 yapılır
}

/**
 * @brief Timer1'in B kanalını kapatır
 *        COM1A1 0 COM1A0 0 yapılır ki PWM çıkışı kapatılsın
 * 
 * @param void 
 *                  
 */
void timer1_pwm_disable_channel_B(void){
    TIMER1->s_TCCR1A&=~((1<<TCCR1A_COM1B1_BIT)|(1<<TCCR1A_COM1B0_BIT)); //COM1A1 COM1A0 0 yapılır

}

/**
 * @brief Kanal A (PB1 / OC1A) pini için duty cycle (aktiflik) süresini ayarlar.
 * OCR1A registerına değeri yazarız o değer ICR x istenen duty cycle olur benim aslında timer1 pwm init fonksiyonundaki ICR değerine burda da ulaşabilmem lazım ki ona göre compare value'yu hesaplayıp OCR1A'ya atayabileyim, bu yüzden timer1_pwm_init fonksiyonunda ICR1 değerini global bir değişkende saklayalım ve burda o değere göre compare value'yu hesaplayarak OCR1A'ya atayalım. ve aslında compare value yerine duty cycle'ı yüzde cinsinden alalım, fonksiyon içinde o yüzdeye göre compare value'yu hesaplayarak OCR1A'ya atayalım, böylece kullanıcı sadece istediği duty cycle'ı yüzde cinsinden girebilir ve bizim fonksiyonumuz o değere göre compare value'yu hesaplayarak OCR1A'ya atar ve PWM sinyalinin duty cycle'ını ayarlar.
 * 
 * @param duty_percentage_x10 Sayaç eşleşme değeri (OCR1A).
 */

void timer1_pwm_set_duty_A(uint16_t duty_percentage_x10){

    if(duty_percentage_x10>1000) duty_percentage_x10=1000; //Duty cycle'ı yüzde cinsinden aldığımız için 100'den büyük değerleri 100 yapıyoruz
    
    TIMER1->s_OCR1A=((uint32_t)ICR1_TOP_VALUE * duty_percentage_x10) / 1000; //OCCR1A için value'yu hesaplıyoruz
    
}
/**
 * @brief Kanal B (PB2 / OC1B) pini için duty cycle (aktiflik) süresini ayarlar.
 * 
 * @param duty_percentage_x10 Sayaç eşleşme değeri (OCR1B).
 */

void timer1_pwm_set_duty_B(uint16_t duty_percentage_x10){
    if(duty_percentage_x10>1000) duty_percentage_x10=1000; //Duty cycle'ı yüzde cinsinden aldığımız için 100'den büyük değerleri 100 yapıyoruz
    
    TIMER1->s_OCR1B=((uint32_t)ICR1_TOP_VALUE * duty_percentage_x10) / 1000; //OCCR1B için value'yu hesaplıyoruz
}
