"""
04/02 EEPROM testi: slave adresi değişiyor mu, reset sonrası kalıyor mu?
Çalıştır: python eeprom_test.py
"""
import time
import serial

PARITY = serial.PARITY_EVEN     # PARITY_NONE / PARITY_ODD
STOP   = serial.STOPBITS_ONE     # STOPBITS_TWO

PORT = "/dev/cu.wchusbserial1130"   # ls /dev/cu.* ile kontrol et
BAUD = 115200


def crc16(veri):
    crc = 0xFFFF
    for b in veri:
        crc ^= b
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc = crc >> 1
    return bytes([crc & 0xFF, crc >> 8])


def paket(*bayt):                   # paket(0xEF, 6, 0, 5, 0, 0xEE) → CRC'yi kendisi ekler
    d = bytes(bayt)
    return d + crc16(d)


gecen = 0
toplam = 0

def test(s, ad, istek, beklenen):
    global gecen, toplam
    s.write(istek)
    cevap = s.read(32)
    if beklenen is None:            # cevap gelmemeli
        ok = (len(cevap) == 0)
    else:
        ok = (cevap == beklenen)
    toplam += 1
    gecen += ok
    print(f"{'✅' if ok else '❌'} {ad:32s} → {cevap.hex(' ') or '(yok)'}")
    if not ok:
        print(f"   beklenen:                          {beklenen.hex(' ') if beklenen else '(yok)'}")
    time.sleep(0.05)


# ------------------------------------------------------------
# 1. AŞAMA: adresi değiştir
# ------------------------------------------------------------
print("=== 1. aşama: adres 0xEF → 0xEE ===")
with serial.Serial(PORT, BAUD, timeout=0.3, parity=PARITY, stopbits=STOP) as s:
    time.sleep(2)                   # port açılınca Arduino resetlenir, bootloader'ı bekle
    s.reset_input_buffer()

    test(s, "EF: adresi 0xEE yap",
         paket(0xEF, 6, 0, 5, 0, 0xEE),
         paket(0xEF, 6, 0, 5, 0, 0xEE))          # eko, ESKİ adresten

    test(s, "EF: eski adres artık susmalı",
         paket(0xEF, 3, 0, 5, 0, 1),
         None)

    test(s, "EE: yeni adresten oku",
         paket(0xEE, 3, 0, 5, 0, 1),
         paket(0xEE, 3, 2, 0, 0xEE))

    test(s, "EE: geçersiz adres 0",
         paket(0xEE, 6, 0, 5, 0, 0),
         paket(0xEE, 0x86, 3))                    # hata da YENİ adresten

    test(s, "EE: geçersiz adres 248",
         paket(0xEE, 6, 0, 5, 0, 248),
         paket(0xEE, 0x86, 3))

# Port burada kapandı. Tekrar açılınca DTR hattı Arduino'yu RESETLER.
# RAM silinir, slave adresi açılışta EEPROM'dan okunur.
print("\n--- port kapatıldı, yeniden açılınca Arduino resetlenecek ---\n")

# ------------------------------------------------------------
# 2. AŞAMA: reset sonrası adres kaldı mı?
# ------------------------------------------------------------
print("=== 2. aşama: reset sonrası ===")
with serial.Serial(PORT, BAUD, timeout=0.3, parity=PARITY, stopbits=STOP) as s:
    time.sleep(2)
    s.reset_input_buffer()

    test(s, "EE: reset sonrası hâlâ 0xEE mi?",
         paket(0xEE, 3, 0, 5, 0, 1),
         paket(0xEE, 3, 2, 0, 0xEE))              # ← asıl test: KALICILIK

    test(s, "EE: adresi 0xEF'e geri al",
         paket(0xEE, 6, 0, 5, 0, 0xEF),
         paket(0xEE, 6, 0, 5, 0, 0xEF))

    test(s, "EF: geri alındı mı?",
         paket(0xEF, 3, 0, 5, 0, 1),
         paket(0xEF, 3, 2, 0, 0xEF))

print(f"\n{gecen}/{toplam} geçti")