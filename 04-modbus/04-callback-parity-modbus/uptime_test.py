"""
04/04 uptime taşma testi: register 2 (taşma), 3 (ms üst), 4 (ms alt)
Arduino'da ms sayacı 0xFFFFC568 ile başlatılmış olmalı (~15 sn sonra taşar).
Çalıştır: python uptime_test.py
"""
import time
import serial

PARITY = serial.PARITY_EVEN      # main.c'deki UART moduyla AYNI olmalı
STOP   = serial.STOPBITS_ONE
PORT   = "/dev/cu.wchusbserial130"
BAUD   = 115200
ADRES  = 0xEF


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


onceki_tasma = None
with serial.Serial(PORT, BAUD, timeout=0.3, parity=PARITY, stopbits=STOP) as s:
    time.sleep(2)                       # reset + bootloader
    s.reset_input_buffer()
    print(" sn  | taşma |        ms        | toplam (gün)")
    print("-----+-------+------------------+-------------")

    for i in range(25):                 # ~26 saniye
        s.write(paket(ADRES, 3, 0, 2, 0, 3))      # reg 2'den 3 adet
        c = s.read(11)                            # adr fn bayt + 6 veri + 2 crc

        if len(c) != 11 or crc16(c[:-2]) != c[-2:]:
            print(f"{i*2:3d}  | cevap hatalı: {c.hex(' ') or '(yok)'}")
        else:
            tasma = (c[3] << 8) | c[4]
            ms    = (c[5] << 24) | (c[6] << 16) | (c[7] << 8) | c[8]
            toplam_ms = tasma * 2**32 + ms
            isaret = "  ← TAŞTI!" if onceki_tasma is not None and tasma != onceki_tasma else ""
            print(f"{i*2:3d}  | {tasma:5d} | {ms:16d} | {toplam_ms/86_400_000:10.4f}{isaret}")
            onceki_tasma = tasma

        time.sleep(2)