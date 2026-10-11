#ifndef BINARY_PACKET_H
#define BINARY_PACKET_H

#include <stdint.h>
#include <timer_config.h>
#include <string.h>

/**
 * @param CRC-16 ile kontrol edilen binary paket formatı
 * 
 * Paket yapısı:
 * | BAŞLANGIÇ (0xAA) | VERİ UZUNLUĞU | PAKET TİPİ | VERİ | CRC-16 L | CRC-16 H |
 */


typedef enum{ //bunlar int yani 16 bitliktir
    TIP_LED=0x01,
    TIP_PING=0x02
};

typedef struct {

    uint8_t veri_uzunlugu;
    uint8_t veri_tipi;
    uint8_t veriler[32];
}Paket_t;


typedef enum{
    //| BASLANGIC_BEKLE | 0xAA | Sayacı sıfırla | UZUNLUK_BEKLE |
// | BASLANGIC_BEKLE | başka bayt | Yok say | BASLANGIC_BEKLE |
// | UZUNLUK_BEKLE | > 32 | Reddet | BASLANGIC_BEKLE |
// | UZUNLUK_BEKLE | ≤ 32 | Uzunluğu sakla | TIP_VE_VERI_TOPLA |
// | TIP_VE_VERI_TOPLA | ilk bayt | Tip olarak sakla, **hemen** sayaç == uzunluk mu kontrol et | Evetse CRC_L_BEKLE |
// | TIP_VE_VERI_TOPLA | sonraki baytlar | Veriye ekle, sayacı artır | Sayaç == uzunluk ise CRC_L_BEKLE |
// | CRC_L_BEKLE | bayt | Sakla | CRC_H_BEKLE |
// | CRC_H_BEKLE | CRC tuttu | Paketi teslim et (return 1) | BASLANGIC_BEKLE |
// | CRC_H_BEKLE | CRC tutmadı | `hata_sayaci++` (sessizce) | BASLANGIC_BEKLE |
// | (herhangi biri) | 50 ms bayt gelmedi | Yarım paketi at | BASLANGIC_BEKLE |

    BASLANGIC_BEKLE,
    UZUNLUK_BEKLE,
    TIP_VE_VERI_TOPLA,
    CRC_L_BEKLE,
    CRC_H_BEKLE

}ReadingState_t;

//---------------TX yani gönderilecek verileri ayarlama----------------
//Paketi oluşturalım


uint8_t binary_paket_olustur(uint8_t tip, uint8_t uzunluk, const uint8_t *alinan_veri, uint8_t *paket_cikis);

/*

---

## 1. Paket yapısı

```
| 0xAA | UZUNLUK | TİP | VERİ (0–32 bayt) | CRC_L | CRC_H |
```

- **0xAA**: Başlangıç baytı
- **UZUNLUK**: Sadece VERİ'nin bayt sayısı (0–32)
- **TİP**: Paketin ne olduğu (komut numarası)
- **CRC**: UZUNLUK + TİP + VERİ üzerinden hesaplanır (0xAA dahil değil). Önce düşük bayt, sonra yüksek bayt.
- Çok baytlı sayılar **little-endian** gönderilir (düşük bayt önce), kaydırma ile: `(t >> 0) & 0xFF`, `(t >> 8) & 0xFF`...

**Zarf ve mektup:** `0xAA`, UZUNLUK ve CRC zarftır (paketi taşır). TİP ve VERİ mektuptur (asıl bilgi). Main sadece mektupla ilgilenir.

### Komutlar

| TİP | Ad | PC → Arduino | Arduino → PC |
|---|---|---|---|
| 0x01 | LED | 1 bayt: 0 kapat / 1 aç | Aynı paket (onay) |
| 0x02 | PING | Veri yok | 4 bayt: `getTickTime()` |

### Test paketleri (CRC-16/MODBUS)

| Ne | Baytlar |
|---|---|
| LED aç | `AA 01 01 01 E0 50` |
| PING | `AA 00 02 80 71` |
| PING cevabı (10000 ms) | `AA 04 02 10 27 00 00 CC 94` |

---

## 2. Modüller ve kim neyi bilir

```
uart.c    → donanımı bilir, halka tamponları tutar. Paketi bilmez.
paket.c   → paket yapısını bilir. UART'ı da timer'ı da bilmez.
crc.c     → sadece CRC hesaplar.
main.c    → hepsini birbirine bağlar.
```

`paket.c` zamanı parametre olarak alır (`simdi_ms`), timer'ı kendisi çağırmaz. Böylece bilgisayarda Arduino olmadan test edilebilir.

---

## 3. TX: Paket oluşturma

```c
uint8_t paket_olustur(uint8_t *cikis, uint8_t tip, const uint8_t *veri, uint8_t uzunluk);
// dönüş: cikis'e yazılan toplam bayt sayısı, hata varsa 0
```

Adımlar:
1. `uzunluk > 32` ise `return 0`
2. `cikis[0] = 0xAA`
3. `cikis[1] = uzunluk`
4. `cikis[2] = (uint8_t)tip`
5. Veriyi `cikis[3]`'ten başlayarak kopyala
6. CRC'yi `cikis[1]`'den başlayarak `2 + uzunluk` bayt üzerinden hesapla
7. Sona önce CRC_L, sonra CRC_H ekle
8. Toplam uzunluğu döndür: `5 + uzunluk`

Kullanımı (main'de):
```c
uint8_t veri[1] = { 1 };
uint8_t cikis[37];
uint8_t n = paket_olustur(cikis, PAKET_LED, veri, 1);
for (uint8_t i = 0; i < n; i++) usart_send_char(cikis[i]);
```

TX halka tamponundan okuyan UDRE ISR'ıdır. `uart.c`'de değişiklik gerekmez.

---

## 4. RX: Paket alma

### Durum makinesi

| Durum | Olay | Ne yapılır | Sonraki durum |
|---|---|---|---|
| BASLANGIC_BEKLE | 0xAA | Sayacı sıfırla | UZUNLUK_BEKLE |
| BASLANGIC_BEKLE | başka bayt | Yok say | BASLANGIC_BEKLE |
| UZUNLUK_BEKLE | > 32 | Reddet | BASLANGIC_BEKLE |
| UZUNLUK_BEKLE | ≤ 32 | Uzunluğu sakla | TIP_VE_VERI_TOPLA |
| TIP_VE_VERI_TOPLA | ilk bayt | Tip olarak sakla, **hemen** sayaç == uzunluk mu kontrol et | Evetse CRC_L_BEKLE |
| TIP_VE_VERI_TOPLA | sonraki baytlar | Veriye ekle, sayacı artır | Sayaç == uzunluk ise CRC_L_BEKLE |
| CRC_L_BEKLE | bayt | Sakla | CRC_H_BEKLE |
| CRC_H_BEKLE | CRC tuttu | Paketi teslim et (return 1) | BASLANGIC_BEKLE |
| CRC_H_BEKLE | CRC tutmadı | `hata_sayaci++` (sessizce) | BASLANGIC_BEKLE |
| (herhangi biri) | 50 ms bayt gelmedi | Yarım paketi at | BASLANGIC_BEKLE |

**Notlar:**
- Veri toplarken gelen 0xAA sadece veridir, başlangıç sayılmaz.
- Satır tamponu yok. Sonuna `'\0'` eklenmez, çünkü veri 0x00 içerebilir.
- Senkron kaybolursa (yanlış AA) ya uzunluk > 32 olur ya da CRC tutmaz → paket atılır → sistem kendini toparlar.
- CRC hatasında metin mesajı gönderilmez, hat binary olduğu için karşı tarafı bozar.

### Nerede çalışır: main'de

- CRC hesabı ISR için çok uzun.
- Main takılırsa halka tampon baytları korur (9600 baud'da ~130 ms tolerans).
- Durum makinesi ve zaman aşımı aynı yerde → `durum` değişkeninin tek yazarı var → `cli` gerekmez.
- Paket teslimi bayrakla değil, fonksiyonun dönüş değeriyle → buton bayrağı yarışı oluşmaz.


*/

uint8_t binary_paket_oku(uint8_t okunan_byte,Paket_t *paket, uint32_t bulunulan_zaman_ms);
/*

### Arayüz

```c
typedef enum { PAKET_LED = 0x01, PAKET_PING = 0x02 } PaketTip_t;

typedef struct {          // sadece "mektup": zarf (AA, CRC) yok
    uint8_t tip;
    uint8_t uzunluk;
    uint8_t veri[32];
} Paket_t;

uint8_t  paket_bayt_isle(uint8_t bayt, uint32_t simdi_ms, Paket_t *p);  // paket hazırsa 1
void     paket_zaman_asimi(uint32_t simdi_ms);                          // main her döngüde çağırır
uint16_t paket_hata_sayisi(void);
```

Kullanımı (main'de):
```c
Paket_t p;
while (1) {
    while (rb_veri_var()) {
        if (paket_bayt_isle(rb_oku(), getTickTime(), &p)) {
            switch (p.tip) {
            case PAKET_LED:   LED'i ayarla, onay gönder  break;
            case PAKET_PING:  zamanı gönder              break;
            }
        }
    }
    paket_zaman_asimi(getTickTime());
}
```

RX halka tamponundan okuyan main'dir. `paket.c` halka tamponu bilmez, sadece tek bir bayt alır.

---


## 5. Uygulama sırası (her adımı test etmeden sonrakine geçme)

- [ ] **Adım 1 – CRC:** CRC-16'nın nasıl çalıştığını öğren, `crc16()` fonksiyonunu yaz. Test: `crc16({01,01,01}, 3)` → `0x50E0`, `crc16({00,02}, 2)` → `0x7180`
- [ ] **Adım 2 – TX:** `paket_olustur()` yaz. Test: LED paketi `AA 01 01 01 E0 50` çıkmalı.
- [ ] **Adım 3 – RX, sadece mutlu yol:** Durum makinesini yaz, zaman aşımı olmadan. Test: LED ve PING paketlerini tanımalı.
- [ ] **Adım 4 – RX, hatalar:** Uzunluk > 32, bozuk CRC, paketten önce çöp baytlar. Test: hata sayacı artmalı, sonraki doğru paket yine tanınmalı.
- [ ] **Adım 5 – Zaman aşımı:** `paket_zaman_asimi()`. Test: yarım paket gönder, 50 ms bekle, sonra tam paket gönder → tam paket tanınmalı.
- [ ] **Adım 6 – Main + Python:** Komutları işle, cevap gönder. Bilgisayardan Python ile paket gönderip cevabı oku.

Adım 1–5 bilgisayarda gcc ile, Arduino olmadan test edilebilir.

void binary_paket_oku();

*/

void binary_paket_zaman_asimi(uint32_t simdi_ms);




uint16_t binary_paket_hata_sayisi(void);



#endif // BINARY_PACKET_H

