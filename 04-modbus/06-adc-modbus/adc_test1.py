"""
04/06 ADC testi: register 6 (ham ADC ortalaması) ve register 7 (sıcaklık × 10)
tek istekte okunur, yarım saniyede bir.
LM35 + 5 V referans: 1 adım = 5000/1024 ≈ 4.88 mV, LM35 = 10 mV/°C
Çalıştır: python adc_test.py   (durdurmak için Ctrl+C)
"""
import time
import serial

PORT   = "/dev/cu.usbserial-BG04PLY2"   # RS-485 dönüştürücü
BAUD   = 115200
PARITY = serial.PARITY_EVEN             # main.c'deki UART moduyla AYNI olmalı
ADRES  = 0xEF
REG    = 6                              # reg 6'dan başla
ADET   = 2                              # reg 6 ve reg 7


def crc16(veri):
    crc = 0xFFFF
    for b in veri:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return bytes([crc & 0xFF, crc >> 8])


def paket(*bayt):
    d = bytes(bayt)
    return d + crc16(d)


def int16(deger):
    """16 bitlik register'ı işaretli sayı olarak yorumla (eksi sıcaklıklar için)."""
    return deger - 0x10000 if deger & 0x8000 else deger


with serial.Serial(PORT, BAUD, parity=PARITY, timeout=0.3) as s:
    time.sleep(0.5)
    s.reset_input_buffer()
    print(" reg6 ham | ham'dan °C | reg7 (×10) | reg7 °C")
    print("----------+------------+------------+--------")
    try:
        while True:
            s.write(paket(ADRES, 3, 0, REG, 0, ADET))
            c = s.read(5 + 2 * ADET)                 # adr fn bayt + 4 veri + 2 crc = 9

            if len(c) != 5 + 2 * ADET or crc16(c[:-2]) != c[-2:]:
                print(f"cevap hatalı: {c.hex(' ') or '(yok)'}")
            elif c[1] & 0x80:
                print(f"exception: kod {c[2]}  (2 ise REGISTER_VALUE 8 mi?)")
            else:
                ham  = (c[3] << 8) | c[4]
                reg7 = int16((c[5] << 8) | c[6])
                ham_c = ham * 5000 / 1024 / 10        # karşılaştırma için: ortalamadan hesap
                print(f"{ham:9d} | {ham_c:10.1f} | {reg7:10d} | {reg7 / 10:6.1f}")

            time.sleep(0.5)
    except KeyboardInterrupt:
        pass