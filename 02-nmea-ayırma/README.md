# 02 – NMEA Ayrıştırma + GPS Durum Takibi

ATmega328p (Arduino Uno) için **register seviyesinde**, Arduino kütüphaneleri kullanılmadan yazılmış bir GPS (NMEA 0183) ayrıştırıcı.

[01-nmea-dogrulama](../01-nmea-dogrulama)'daki kesmeli UART sürücüsünün üzerine kuruludur. Doğrulanan cümleler alanlara bölünür, konum, saat, tarih, hız ve hassasiyet bilgileri bir struct'a yazılır. Timer0 ile tutulan sistem saati sayesinde GPS verisinin ne kadar taze olduğu takip edilir.

## Özellikler

- **4 cümle tipi:** RMC, GGA, GSA, GLL
- **Konum dönüşümü:** NMEA'nın `ddmm.mmmm` biçimi ondalık dereceye çevrilir, güney ve batı negatif yazılır
- **Fix kontrolü:** Geçersiz (`V`) ya da fix'siz cümleler struct'a yazılmaz
- **Boş alan güvenliği:** Her alana erişmeden önce alan sayısı kontrol edilir. Gelmeyen değerler `NAN` olarak işaretlenir
- **Kendi `millis()`'im:** Timer0 CTC modunda 1 ms'lik tick, kesme güvenli okuma
- **GPS durum makinesi:** Veri yok / uydu aranıyor / konum var durumları ayrı ayrı algılanır
- Konum ve hız saniyede bir yazdırılır

## Dosya yapısı

```
02-nmea-ayristirma/
├── include/
│   ├── uart.h                   UART sürücüsü
│   ├── nmea.h                   Gps_t struct'ı, NMEA fonksiyonları
│   ├── timer_config.h           Timer0 tick fonksiyonları
│   ├── timer0_registering.h     Timer0 register tanımları
│   └── interrupt_registering.h  Kesme açma/kapama makroları
├── src/
│   ├── main.c                   Satır tamponu, durum makinesi, yazdırma
│   ├── uart.c                   Halka tamponlar, RX/UDRE kesmeleri
│   ├── nmea.c                   Doğrulama, alanlara bölme, ayrıştırma
│   └── timer_config.c           Timer0 CTC, 1 ms tick
└── platformio.ini
```

Katmanlar birbirinden bağımsızdır: `uart.c` NMEA'yı, `nmea.c` de UART'ı ya da timer'ı bilmez. Hepsini `main.c` birbirine bağlar. Bu sayede UART sürücüsü başka protokoller için de kullanılabilir.

## Veri akışı

```
RX pini → UART kesmesi → halka tampon → satır tamponu
        → nmea_dogrula()      checksum doğru mu?
        → nmea_parsing()      virgüllerden alanlara böl
        → gps_detect()        fix geçerli mi?
        → read_gps()          alanları Gps_t'ye yaz
        → zaman damgası       son_cumle_ms, son_fix_ms
        → durum makinesi      VERI_YOK / FIX_YOK / FIX_VAR
```

## Desteklenen cümleler

| Cümle | Okunan alanlar | Fix koşulu |
|---|---|---|
| **RMC** | Saat, tarih, enlem, boylam, hız, hareket yönü, mod | 2. alan `A` |
| **GGA** | Saat, enlem, boylam, fix kalitesi, uydu sayısı, HDOP, yükseklik | 6. alan `0` değil |
| **GSA** | Fix tipi (2D/3D), PDOP, HDOP, VDOP | 2. alan `2` ya da `3` |
| **GLL** | Enlem, boylam, saat, mod | 6. alan `A` |

GSA konum taşımadığı için "konum geldi" zaman damgasını güncellemez.

### Alanlara bölme

`nmea_parsing()` yeni bellek ayırmaz. Cümlenin içindeki virgülleri `\0` ile değiştirir ve her alanın başlangıç adresini bir pointer dizisine yazar:

```
$GPRMC,093015,A,3750.8800,N,...
       ↑      ↑ ↑
   alan[1] alan[2] alan[3] ...
```

Boş alanlar (`,,`) boş string olarak kalır ve alan numaraları kaymaz.

### Koordinat dönüşümü

NMEA konumu derece ve dakika birleşik yazar: `3750.8800` = 37° 50.88'.

```c
float nmea_derece(float ddmm) {
    int16_t derece = (int16_t)(ddmm / 100);   // 37
    float dakika  = ddmm - derece * 100;      // 50.88
    return derece + dakika / 60.0f;           // 37.848
}
```

Yön harfi `S` ya da `W` ise sonuç negatif yapılır. Boylamda derece 3 basamaklıdır (`13945.68` = 139° 45.68'), formül iki durumda da aynı çalışır.

## Timer0: 1 ms tick

```
16 MHz / 64 (prescaler) = 250 kHz  →  bir tık = 4 µs
250 tık (OCR0A = 249)            =  1 ms  →  TIMER0_COMPA_vect
```

- `getTickTime()`: Açılıştan beri geçen milisaniye
- `getTickinMicro()`: Mikrosaniye (4 µs çözünürlük)

**Kesme güvenli okuma:** Sayaç 4 baytlıktır, AVR ise tek seferde 1 bayt okur. Okuma sırasında kesme araya girerse yarısı eski, yarısı yeni bir değer okunabilir. Bu yüzden okuma sırasında kesmeler kapatılır ve SREG kaydedilip geri yüklenir. `sei()` kullanılmaz, böylece fonksiyon kesmelerin kapalı olduğu bir yerden çağrılsa bile onları izinsiz açmaz.

`getTickinMicro()` ayrıca kesmeler kapalıyken gerçekleşen ama henüz ISR tarafından sayılmamış eşleşmeyi `OCF0A` bayrağından kontrol eder. Böylece zaman hiçbir zaman geriye gitmez.

**Taşma:** Zaman farkları her zaman çıkarma ile hesaplanır (`simdi - onceki >= sure`). Unsigned aritmetik sayesinde sayaç taşsa da (ms için ~49.7 gün) sonuç doğru çıkar.

## GPS durum makinesi

| Durum | Anlamı | Olası sebep |
|---|---|---|
| `GPS_VERI_YOK` | Checksum'ı geçen hiçbir cümle gelmiyor | Kablo koptu, modül kapalı, baud yanlış |
| `GPS_FIX_YOK` | Cümle geliyor ama konum geçersiz | Soğuk başlangıç, kapalı alan |
| `GPS_FIX_VAR` | Konum geçerli ve taze | Normal çalışma |

Durum, iki zaman damgasının tazeliğinden hesaplanır:

```c
if      (!cumle_taze) durum = GPS_VERI_YOK;
else if (!fix_taze)   durum = GPS_FIX_YOK;
else                  durum = GPS_FIX_VAR;
```

- **Geçiş mesajı** (kenar): Sadece durum değiştiğinde bir kere basılır
- **Konum satırı** (seviye): `GPS_FIX_VAR` durumunda olduğu sürece saniyede bir basılır

Bu ayrım yer istasyonu için önemli. "GPS yok" uyarısında operatör kabloya mı bakmalı, yoksa açık alana mı çıkmalı? Bu üç durum bu soruyu cevaplıyor.

## Test

GPS gerekmez. Seri monitörü 9600 baud ve satır sonu "Newline" ile açıp cümleleri yapıştırın. Zaman aşımı 2 saniye olduğu için yapıştırmalar arasında ~1 saniye bekleyin.

| Girdi | Beklenen |
|---|---|
| `$GPRMC,061520,A,3541.1200,N,13945.6800,E,000.0,000.0,091026,,,A*78` | `KONUM VAR`, Tokyo: `35.685333, 139.761333` |
| `$GPRMC,153045,A,3436.2200,S,05822.8100,W,003.2,270.0,081026,,,A*76` | Buenos Aires: `-34.603668, -58.380165` |
| `$GPRMC,101501,A,3750.8850,N,02750.7200,E,054.0,045.0,091026,,,A*79` | Hız: `100.0 km/s` |
| `$GPRMC,101502,A,3750.8800,N,02750.7000,E,,,091026,,,A*7D` | Hız alanı boş: `hiz=-` |
| `$GPRMC,093017,V,,,,,,,081026,,,N*52` (1 sn arayla) | `UYDU ARANIYOR` |
| `$GPGSA,A,3,04,05,09,12,17,20,,,,,,,1.8,0.9,1.5*39` | Konum damgası güncellenmez |
| Hiçbir şey göndermemek | 2 sn sonra `VERI YOK` |
| Checksum'ı bozuk bir cümle | `BOZUK`, veri gelmemiş sayılır |


## Bilinen sınırlamalar

- Float'ın ~7 anlamlı hanesi var. Enlemde bu ~0.3 m'lik bir yuvarlama demek, GPS'in kendi hatasının (2–5 m) yanında önemsiz.
- AVR'de `printf` `%f` desteklemediği için sayılar `dtostrf` ile yazdırılır.
- GSV (görünen uydular) ve VTG cümleleri ayrıştırılmaz.
- Struct'ta değerler cümleler arasında korunur. RMC'nin tarihi, sonra gelen GLL'de değişmeden kalır. Tazelik için zaman damgalarına bakılmalıdır.
- NMEA'nın XOR checksum'ı karakter yer değiştirmelerini yakalayamaz (bkz. `01-nmea-dogrulama`).

## Sonraki adımlar

- [x] `01-nmea-dogrulama` – Kesmeli UART + checksum
- [x] `02-nmea-ayristirma` – Ayrıştırma, Timer0 tick, GPS durum makinesi
- [ ] Gerçek NEO-6M ile açık alanda test
- [ ] `03-crc16` – CRC-16 ve binary paket protokolü (başlangıç baytı + uzunluk + veri + CRC)
- [ ] `04-modbus-slave` – Modbus RTU köle cihaz
