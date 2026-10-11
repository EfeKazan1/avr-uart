#include "binary_packet.h"

/*
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
*/

uint8_t binary_paket_olustur(uint8_t tip, uint8_t uzunluk, const uint8_t *alinan_veri, uint8_t *paket_cikis){

    if(uzunluk>32) return 0; //uzunluk kontrolü

    //baslangıç

    paket_cikis[0]=0xAA; paket_cikis[1]=uzunluk; paket_cikis[2]=(uint8_t)tip;

    //verileri bundan sonra kopyalama

    memcpy((paket_cikis+3),alinan_veri,uzunluk);

    //CRC hesaplama fonksiyonu

    uint16_t crc = crc16(paket_cikis + 1, 2 + uzunluk);   // uzunluk + tip + veri

    paket_cikis[3 + uzunluk] = crc & 0xFF;                // önce düşük bayt

    paket_cikis[4 + uzunluk] = crc >> 8;                  // sonra yüksek bayt
    

    return uzunluk+5;

}

//DURUM DEĞİŞKENİ
static ReadingState_t binary_state=BASLANGIC_BEKLE;

static uint8_t veri_sayaci=0;

static uint8_t tip_okundu_mu=0;

static uint8_t crc_l=0, crc_h;

static uint32_t son_bayt_ms = 0;

static uint8_t tampon[34]; //[0] = uzunluk ,[1]=tip, [2]...=veri

static uint8_t hata_sayaci = 0;

uint8_t binary_paket_oku(uint8_t okunan_byte,Paket_t *paket, uint32_t bulunulan_zaman_ms){


    //maindeki reading buraya okunan byte olarak gelecek. 

    //Burada bir FSM olacak

    son_bayt_ms = bulunulan_zaman_ms;

    switch (binary_state)
    {
    case BASLANGIC_BEKLE:

        if(okunan_byte == 0xAA)
        {
         veri_sayaci=0; tip_okundu_mu=0; binary_state=UZUNLUK_BEKLE;
        }
        
        break;
    
    case UZUNLUK_BEKLE:

        if(okunan_byte>32) binary_state=BASLANGIC_BEKLE;
        else {
            //paket->veri_uzunlugu=okunan_byte;
            tampon[0]=okunan_byte;
            binary_state=TIP_VE_VERI_TOPLA;
        }
        
        break;
    
    case TIP_VE_VERI_TOPLA:
        //ilk okunacak şimdi tip

        if(!tip_okundu_mu){

            //paket->veri_tipi=okunan_byte;
            tampon[1]=okunan_byte;
            tip_okundu_mu=1;
        } 

        else{

            //şimdi paketteki veri kısmına okunan baytları yazacağız ve sayacı bir artıracağız
        
            //eğer 2 ise 2 sayac okunacak sayac 0 ve 1 

            //paket->veriler[veri_sayaci++]=okunan_byte;

            tampon[2 + veri_sayaci++]=okunan_byte;
        }

        if(veri_sayaci==tampon[0])binary_state=CRC_L_BEKLE;

        break;  
    
    case CRC_L_BEKLE:

        crc_l=okunan_byte; binary_state=CRC_H_BEKLE;

        break;
    
    case CRC_H_BEKLE:
        
        crc_h=okunan_byte;

        binary_state=BASLANGIC_BEKLE;

        uint16_t gelen    = ((uint16_t)crc_h << 8) | crc_l;

        uint16_t hesaplanan = crc16(tampon, 2 + tampon[0]);

        if (gelen == hesaplanan) {
            // paketi teslim et, return 1
            paket->veri_uzunlugu = tampon[0];
            paket->veri_tipi     = tampon[1];
            memcpy(paket->veriler, tampon + 2, tampon[0]);
            return 1;


        } else {
            hata_sayaci++;
        }
        binary_state = BASLANGIC_BEKLE;

        break;
    default:
        break;
        
    }
    return 0;
}


void binary_paket_zaman_asimi(uint32_t simdi_ms) {
    if (binary_state != BASLANGIC_BEKLE && (simdi_ms - son_bayt_ms) > 50) {
        binary_state = BASLANGIC_BEKLE;      // yarım paket 50 ms'dir bekliyor, at
    }
}

uint16_t binary_paket_hata_sayisi(void) { return hata_sayaci; }


//     ### Durum makinesi

// | Durum | Olay | Ne yapılır | Sonraki durum |
// |---|---|---|---|
// | BASLANGIC_BEKLE | 0xAA | Sayacı sıfırla | UZUNLUK_BEKLE |
// | BASLANGIC_BEKLE | başka bayt | Yok say | BASLANGIC_BEKLE |
// | UZUNLUK_BEKLE | > 32 | Reddet | BASLANGIC_BEKLE |
// | UZUNLUK_BEKLE | ≤ 32 | Uzunluğu sakla | TIP_VE_VERI_TOPLA |
// | TIP_VE_VERI_TOPLA | ilk bayt | Tip olarak sakla, **hemen** sayaç == uzunluk mu kontrol et | Evetse CRC_L_BEKLE |
// | TIP_VE_VERI_TOPLA | sonraki baytlar | Veriye ekle, sayacı artır | Sayaç == uzunluk ise CRC_L_BEKLE |
// | CRC_L_BEKLE | bayt | Sakla | CRC_H_BEKLE |
// | CRC_H_BEKLE | CRC tuttu | Paketi teslim et (return 1) | BASLANGIC_BEKLE |
// | CRC_H_BEKLE | CRC tutmadı | `hata_sayaci++` (sessizce) | BASLANGIC_BEKLE |
// | (herhangi biri) | 50 ms bayt gelmedi | Yarım paketi at | BASLANGIC_BEKLE |

// **Notlar:**
// - Veri toplarken gelen 0xAA sadece veridir, başlangıç sayılmaz.
// - Satır tamponu yok. Sonuna `'\0'` eklenmez, çünkü veri 0x00 içerebilir.
// - Senkron kaybolursa (yanlış AA) ya uzunluk > 32 olur ya da CRC tutmaz → paket atılır → sistem kendini toparlar.
// - CRC hatasında metin mesajı gönderilmez, hat binary olduğu için karşı tarafı bozar.

// ### Nerede çalışır: main'de

// - CRC hesabı ISR için çok uzun.
// - Main takılırsa halka tampon baytları korur (9600 baud'da ~130 ms tolerans).
// - Durum makinesi ve zaman aşımı aynı yerde → `durum` değişkeninin tek yazarı var → `cli` gerekmez.
// - Paket teslimi bayrakla değil, fonksiyonun dönüş değeriyle → buton bayrağı yarışı oluşmaz.





