# 04/01 – Modbus RTU Slave (temel)

ATmega328p için **register seviyesinde** yazılmış bir Modbus RTU slave. Fonksiyon 03 (register oku) ve 06 (tek register yaz) destekleniyor. Hata durumlarında standart exception cevapları dönüyor.

Önce bilgisayarda gcc ile, sonra Arduino üzerinde ham baytlarla, en son da **pymodbus** (gerçek bir Modbus master kütüphanesi) ile test edildi.

[03-binary-paket](../../03-binary-paket)'teki UART sürücüsü, CRC-16 ve Timer0 tick'i üzerine kuruludur.

## Kendi protokolüm ile Modbus arasındaki fark

03'te protokolü ben icat ettim, karşı tarafı da ben yazdım. Burada protokol bir standart. Karşı taraf, kodumu hiç görmemiş bir kütüphane.

| | 03 – kendi protokolüm | Modbus RTU |
|---|---|---|
| Başlangıç | `0xAA` | Yok |
| Paketin sonu | Uzunluk alanı | **3,5 karakterlik sessizlik** |
| Tip / komut | Kendi seçtiğim numaralar | Standart fonksiyon kodları (03, 06...) |
| CRC kapsamı | `0xAA` hariç | Adres dahil, son 2 bayt hariç her şey |
| Hata bildirimi | Yok | `fonk \| 0x80` + hata kodu |

## Cihaz

**Slave adresi:** `0xEF` (239), 9600 8N1

| Register | İçerik | Birim | Okuma | Yazma |
|---|---|---|---|---|
| 0 | Cihaz kimliği | sabit `0xEFE1` | ✓ | ✗ → 02 |
| 1 | LED (PB5) | 0 / 1 | ✓ | ✓, diğer değerler → 03 |
| 2 | Uptime, üst 16 bit | ms | ✓ | ✗ → 02 |
| 3 | Uptime, alt 16 bit | ms | ✓ | ✗ → 02 |
| 4 | Sıcaklık | °C × 10, int16 (şimdilik sabit 235) | ✓ | ✗ → 02 |

- Register değerleri **büyük uç önce**, CRC **küçük uç önce** gönderilir.
- 32 bitlik uptime iki register'a bölünür, **üst yarı önce** (word order). İki yarı aynı `getTickTime()` değerinden hesaplanır. Master ikisini tek bir istekte okumalıdır, aksi hâlde yırtık bir değer okuyabilir.

## Paketler

**03 – oku:** `EF 03 00 00 00 04 53 47` (register 0'dan 4 tane)
```
EF 03 08 EF E1 00 01 00 01 E2 40 CRC CRC
      │  └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘
      │  reg0  reg1  reg2  reg3
      bayt sayısı = adet × 2
```

**06 – yaz:** `EF 06 00 01 00 01 0E 84` (LED'i yak). Cevap isteğin birebir aynısı.

**Hata:** `EF 83 02 A0 C4` → 03 isteğinde geçersiz adres

| Kod | Anlamı | Ne zaman |
|---|---|---|
| 01 | Geçersiz fonksiyon | 03 ve 06 dışında bir fonksiyon |
| 02 | Geçersiz adres | Olmayan register'ı okumak, yazılamayan register'a yazmak |
| 03 | Geçersiz değer | Adet 0, LED'e 0/1 dışında bir değer |

**Sessiz kalınan durumlar:** Adres benim değil, CRC tutmuyor, paket çok kısa, taşma. Paket bozuksa adres baytına da güvenilemez. Cevap verirsem gerçek hedefin cevabıyla hatta çarpışabilirim.

## Mimari

```
UART RX ISR → halka tampon → main: modbus_bayt_al() → çerçeve tamponu (8 bayt)
                              main: modbus_isle()    → sessizlik? → kontrol → cevap
```

```c
while (1) {
    uint32_t simdi = getTickTime();          // turda bir kez
    while (rb_veri_var())
        modbus_bayt_al(rb_oku(), simdi);
    uint8_t n = modbus_isle(simdi, cevap);
    if (n) usart_send_bytes(cevap, n);
}
```

**`modbus_bayt_al`** (sadece bayt geldiğinde çalışır): Yer varsa tampona yazar, yoksa taşma bayrağını koyar. Her durumda zamanı kaydeder.

**`modbus_isle`** (her turda çalışır):
1. Tampon boşsa ya da son bayttan beri 5 ms geçmediyse → çık
2. `uzunluk = n; n = 0;` Tampon burada, **bir kez** temizlenir. Böylece hangi `return`'den çıkılırsa çıkılsın temiz kalır
3. Taşma → at
4. Adres, uzunluk ≥ 4, CRC → biri tutmazsa sessiz kal
5. Fonksiyona göre ayır, alanları **o fonksiyona göre** yorumla

**Sessizlik eşiği neden 5 ms?** 9600 baud'da 3,5 karakter ≈ 3,6 ms. Timer 1 ms adımlarla sayıyor. Son bayt bir tick'ten hemen önce geldiyse `>= 4` koşulu gerçekte ~3,1 ms sonra doğru olur. `>= 5` ile gerçekte en az 4 ms geçer.

**Taşma:** 8 bayttan uzun bir paket (mesela fonksiyon 16) geldiğinde tampon sıfırlanmaz, sadece işaretlenir. Taşan baytlar okunmaya devam eder ve zamanı güncellemeye devam eder. Paket ancak tamamen bittikten sonra atılır. Aksi hâlde paketin kuyruğu yeni bir paket sanılırdı.

## Test

### 1. Bilgisayarda (gcc, Arduino yok)

`modbus.c` donanımı bilmediği için (zamanı parametre olarak alıyor) bilgisayarda derlenip test edildi: 11 test, 10'u ilk seferde geçti. Kalan biri (bilinmeyen fonksiyon → 01) düzeltildi.

### 2. Arduino, ham baytlar (pyserial)

```
✅ 06 LED yak            →  EF 06 00 01 00 01 0E 84
✅ 03 LED oku            →  EF 03 02 00 01 91 93
✅ 06 LED söndür         →  EF 06 00 01 00 00 CF 44
✅ 03 olmayan adres      →  EF 83 02 A0 C4
✅ 03 adet = 0           →  EF 83 03 61 04
✅ 06 reg 4'e yaz        →  EF 86 02 A3 94
✅ 06 LED = 5            →  EF 86 03 62 54
✅ bilinmeyen fonk 04    →  EF 84 01 E2 F5
✅ başka slave (0x05)    →  (yok)
✅ bozuk CRC             →  (yok)
10/10 geçti

yarım paket + 20 ms + tam paket  →  EF 03 02 00 00 50 53
```

### 3. Arduino, pymodbus

```
kimlik   = 0xEFE1
LED      = 0
uptime   = 231833 ms (231.8 sn)
sıcaklık = 23.5 °C
LED ← 1:  tamam
LED ← 0:  tamam
olmayan adres okuma: ExceptionResponse(dev_id=239, function_code=131, exception_code=2)
```

`dev_id=239` = `0xEF`, `function_code=131` = `0x83`. Hata cevabı da standart bir Modbus exception olarak çözüldü.

Test betiği: [`modbus_test.py`](modbus_test.py)

## Öğrenilenler


**`&adres` ile `&registerlar[adres]` aynı şey değil.**
İlki `adres` değişkeninin RAM'deki yeri. Fonksiyon oradan başlayıp yanındaki değişkenleri okuyordu. 02'deki `gps_detect` hatasıyla aynı tür.

**`memcpy` endianness'i bilmez.**
AVR `uint16_t`'yi bellekte küçük uç önce tutuyor (`0x0328` → `28 03`). Modbus büyük uç önce istiyor. Register'lar `>> 8` ve `& 0xFF` ile elle ayrıldı.

**Önce fonksiyona bak, sonra baytları yorumla.**
`tampon[4..5]` 03'te adet, 06'da değer. Kontroller `switch`'ten önce yapılınca `LED = 0` yazma isteği "adet = 0" sanılıp reddediliyordu.

**AVR'de `int` 16 bit.**
`adres + adet` toplamı `int`'e yükseltilir ve taşabilir. Kontrol `(uint32_t)adres + adet` ile yapılıyor.

## Bilinen sınırlamalar

| Bu sürüm | Gerçek bir ürün |
|---|---|
| TTL UART, tek slave | RS-485, aynı hatta 32+ cihaz |
| Fonksiyon 03 ve 06 | Genelde 03, 04, 06, 16 |
| Adres ve baud kodda sabit | Register'dan değiştirilebilir, EEPROM'da saklanır |
| Broadcast (adres 0) yok sayılıyor | 06 uygulanır, cevap verilmez |
| 1 ms'lik tick, 9600 baud | µs hassasiyetinde sessizlik, yüksek baud |
| 8 baytlık tampon: fonksiyon 16 sessizce atılıyor | Tam tampon, 01 hatası |

## Sonraki adımlar

- [x] `01-basic-modbus`: 03/06, exception'lar, pymodbus ile test
- [ ] `02-eeprom-microus-modbus`: slave adresi register'dan değiştirilebilir ve EEPROM'da saklanır, 115200 baud, µs hassasiyetinde sessizlik
- [ ] `03-rs485-modbus`: MAX485, DE/RE yön kontrolü, iki slave aynı hatta
