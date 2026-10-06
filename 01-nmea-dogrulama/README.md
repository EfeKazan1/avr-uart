
# 01 – Kesmeli UART + NMEA Doğrulayıcı

ATmega328p (Arduino Uno) için **register seviyesinde**, Arduino kütüphaneleri kullanılmadan yazılmış bir USART sürücüsü ve bunun üzerinde çalışan bir GPS (NMEA 0183) cümle doğrulayıcı.

Seri porttan gelen her satır alınır, NMEA kontrol toplamına göre doğrulanır ve sonuç geri gönderilir.

## Özellikler

- `Serial` ya da başka bir Arduino kütüphanesi yok, sadece AVR register'ları
- **Kesmeli alma (RX):** Her gelen karakter `USART_RX_vect` kesmesiyle 128 baytlık bir halka tampona alınır. Ana döngü meşgulken bile veri kaybolmaz.
- **Kesmeli gönderme (TX):** Gönderilecek karakterler ayrı bir halka tampona bırakılır, `USART_UDRE_vect` kesmesi onları arka planda gönderir. Ana döngü UART donanımını beklemez.
- **Satır tamponu:** Karakterler `\r` ya da `\n` gelene kadar biriktirilir.
- **NMEA doğrulama:** `$` ile `*` arasındaki baytların XOR'u hesaplanıp cümlenin sonundaki iki haneli hex kontrol toplamıyla karşılaştırılır.
- Sayıyı yazıya çeviren kendi `usart_send_number` fonksiyonu

## Dosya yapısı

```
01-nmea-dogrulama/
├── include/
│   ├── uart.h      UART fonksiyonlarının prototipleri
│   └── nmea.h      NMEA fonksiyonlarının prototipleri
├── src/
│   ├── main.c      Satır tamponu ve ana döngü
│   ├── uart.c      UART ayarları, halka tamponlar, iki kesme
│   └── nmea.c      Hex dönüşümü ve kontrol toplamı doğrulama
└── platformio.ini
```

Halka tamponlar `uart.c` içinde gizlidir. `main.c` tampona yalnızca `rb_veri_var()`, `rb_oku()` ve `usart_send_*()` fonksiyonları üzerinden ulaşır.

## Veri akışı

```
Alma:      RX pini → UDR0 → RX kesmesi → RX halka tamponu → main → satır tamponu
Gönderme:  main → TX halka tamponu → UDRE kesmesi → UDR0 → TX pini
```

## UART ayarları

| Ayar | Değer | Register |
|---|---|---|
| Saat | 16 MHz | – |
| Baud | 9600 | `UBRR0 = 103` |
| Çerçeve | 8N1 | `UCSR0C = (1<<UCSZ01) \| (1<<UCSZ00)` |
| Hız modu | Normal (U2X kapalı) | `UCSR0A = 0` |
| Etkin | TX, RX, RX kesmesi | `UCSR0B = (1<<TXEN0) \| (1<<RXEN0) \| (1<<RXCIE0)` |

### UBRR hesabı

UART her biti 16 kez örnekler. Bir örnek arasında beklenecek saat tıkı sayısı:

```
16.000.000 tık/sn ÷ (16 örnek/bit × 9600 bit/sn) = 104,17 tık/örnek
```

Register'a tam sayı yazılabildiği için 104'e yuvarlanır. Sayaç 0'ı da saydığından 1 çıkarılır: **UBRR = 103**. Gerçek hız 9615 baud, hata **%0,16**.

Kodda C'nin tam sayı bölmesi aşağı kestiği için, bölmeden önce bölenin yarısı eklenerek yuvarlama yapılır:

```c
#define UBRR_DEGER (((F_CPU + 8UL * BAUD) / (16UL * BAUD)) - 1)
```

## NMEA kontrol toplamı

```
$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
 └──────────────── XOR'lanan kısım ────────────────────────────┘ └┘
                                                          kontrol toplamı
```

`nmea_dogrula()` bir satırı üç durumda reddeder:

1. `$` ile başlamıyorsa (ortasından alınmış yarım satır)
2. `*` içermiyorsa (kesilmiş satır)
3. Hesaplanan XOR, `*` sonrasındaki hex değerle uyuşmuyorsa (bozulmuş satır)

Yarım satırların reddedilmesi sayesinde, GPS'in ortasından dinlenmeye başlansa bile sistem bir sonraki satırda kendini toparlar.

## Test

GPS gerekmez. Seri monitörü 9600 baud ve satır sonu "Newline" ile açıp satırları yapıştırın:

| Girdi | Beklenen çıktı |
|---|---|
| `$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A` | `GECERLI: ... \| 68 byte uzunlugunda` |
| Aynı satır, `4807` → `4806` | `BOZUK` |
| `$` olmadan, satırın ortasından başlayan bir parça | `BOZUK` |
| Aynı satır, `4807` → `8407` | `GECERLI` (aşağıya bakın) |

## Bilinen sınırlamalar

- **XOR kontrol toplamı karakterlerin sırasını görmez.** İki karakterin yer değiştirmesi (`4807` → `8407`) yakalanmaz, çünkü XOR'da sıra önemli değildir. Bu, NMEA standardının kendi zayıflığıdır. CRC bu sorunu çözer (bkz. `03-crc16`).
- Tek bir satır en fazla 127 karakter olabilir, fazlası atılır.
- Türkçe karakterler UTF-8'de 2 bayt kaplar. Uzunluk değeri karakter değil bayt sayısını gösterir.
- `sprintf`/`snprintf` yaklaşık 1,5–2 KB flash kullanır.

## Not: Wokwi simülatöründe bilinen bir sorun

Kod gerçek bir Arduino Uno'da sorunsuz çalışır. Ancak **Wokwi** simülatöründe TX kesmesi yalnızca ilk mesajı gönderip durur.

Sebebi: Wokwi'nin AVR motoru (avr8js), `USART_UDRE_vect` kesmesi her çalıştığında **UDRE0 bayrağını temizliyor.** Gerçek ATmega328p'de bu bayrak yalnızca UDR0'a yazıldığında temizlenir. Tampon boşken kesme son kez çalışıp kendini kapattığında, Wokwi'de bayrak 0 kalıyor. Sonraki mesajda UDRIE0 açılsa bile kesme bir daha tetiklenmiyor.

Wokwi'de test etmek için iki yol:
- TX tarafında kesme yerine beklemeli (polling) gönderim kullanmak.

## Sonraki adımlar

- [ ] `02-nmea-ayristirma` – NMEA ayrıştırma: `$GPRMC`'den enlem, boylam ve hız çıkarmak
- [ ] `03-crc16` – CRC-16
- [ ] `04-modbus-slave` – Modbus RTU köle cihaz
- [ ] `05-modbus-master` – Bilgisayardan Modbus ile haberleşme
