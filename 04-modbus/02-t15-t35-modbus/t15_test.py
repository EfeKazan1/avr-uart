import time
import serial

PORT = "/dev/cu.wchusbserial130"   # ls /dev/cu.* ile kontrol et
BAUD = 1200

def crc16(veri):
    crc = 0xFFFF
    for b in veri:                  # her bayt için
        crc ^= b
        for _ in range(8):          # 8 bit
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc = crc >> 1
    return bytes([crc & 0xFF, crc >> 8])   # [CRC_L, CRC_H]

istek = bytes([0xEF, 0x06, 0x00, 0x01, 0x00, 0x01])
paket = istek + crc16(istek)        # 6 bayt + 2 bayt CRC = 8 bayt

def bolerek_gonder(s, bosluk):
    s.write(paket[:3]) 
    s.flush()               
    time.sleep(bosluk)              # bekle (saniye)
    s.write(paket[3:])              # kalan 5 bayt
    cevap = s.read(32)              # cevap bekle (en fazla 0,2 s)
    print(f"boşluk {bosluk*1000:.1f} ms → {cevap.hex(' ') or '(yok)'}")

with serial.Serial(PORT, BAUD, timeout=0.5) as s:
    time.sleep(2)                   # reset sonrası bootloader'ı bekle
    s.reset_input_buffer()
    for bosluk in (0.003, 0.010, 0.020, 0.050):
        bolerek_gonder(s, bosluk)
        time.sleep(0.1)