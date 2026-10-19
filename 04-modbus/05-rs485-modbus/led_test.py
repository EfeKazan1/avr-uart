import time, serial

PORT   = "/dev/cu.usbserial-BG04PLY2"

# PORT = "/dev/cu.wchusbserial130" 
PARITY = serial.PARITY_EVEN          # main.c ile aynı
BAUD   = 115200

def crc16(d):
    crc = 0xFFFF
    for b in d:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return bytes([crc & 0xFF, crc >> 8])

def paket(*b):
    d = bytes(b)
    return d + crc16(d)

with serial.Serial(PORT, BAUD, parity=PARITY, timeout=0.3) as s:
    for i in range(6):
        deger = i % 2                                  # 0, 1, 0, 1 …
        s.write(paket(0xEF  , 6, 0, 1, 0, deger))        # register 1 = LED
        print("LED ->", deger, "| cevap:", s.read(8).hex(' ') or "(yok)")
        time.sleep(1)