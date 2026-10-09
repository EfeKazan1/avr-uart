#ifndef EFE_CONFIG_H
#define EFE_CONFIG_H


/**
 * Bu dosya, AVR mikrodenetleyiciler için temel yapılandırma ve tanımlamaları içeren bir başlık dosyasıdır.
 * @author Efe KAZAN
 * @date 2026-06-01

    * @brief:Avr mikrodenetleyicilerde kullanılan timerlar, GPIO pinleri ve kesme yapılandırmaları gibi temel bileşenlerin tanımları ve fonksiyon prototiplerini içerir.
 */
#pragma once 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <stdint.h>

/**
 * @brief Timer yapılandırması ve fonksiyon prototipleri
 * Timer için özel yapılandırmalar ve fonksiyon prototipleri bulunur.
 */
#include "timer_config.h"

/**
 * @brief GPIO yapılandırması ve fonksiyon prototipleri
 */
#include "gpio_registering.h"

/**
 * @brief Kesme yapılandırması ve fonksiyon prototipleri
 */
#include "interrupt_registering.h"



#endif