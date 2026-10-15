"""
04-modbus slave testi
Kurulum:  pip install pyserial pymodbus
Çalıştır: python modbus_test.py
"""
import time
import serial

PORT = "/dev/cu.wchusbserial130"   # kendi portunla değiştir (ls /dev/cu.*)
SLAVE = 0xEF


def crc16(data: bytes) -> bytes:
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return bytes([crc & 0xFF, crc >> 8])


def paket(*bayt):
    d = bytes(bayt)
    return d + crc16(d)


# ---------------------------------------------------------------
# 1. AŞAMA: ham baytlar (pyserial). Ne gidiyor, ne geliyor, gör.
# ---------------------------------------------------------------
TESTLER = [
    # (açıklama, istek, beklenen cevap; None = cevap gelmemeli)
    ("06 LED yak",          paket(0xEF, 6, 0, 1, 0, 1), paket(0xEF, 6, 0, 1, 0, 1)),
    ("03 LED oku",          paket(0xEF, 3, 0, 1, 0, 1), paket(0xEF, 3, 2, 0, 1)),
    ("06 LED söndür",       paket(0xEF, 6, 0, 1, 0, 0), paket(0xEF, 6, 0, 1, 0, 0)),
    ("03 olmayan adres",    paket(0xEF, 3, 0, 3, 0, 4), paket(0xEF, 0x83, 2)),
    ("03 adet = 0",         paket(0xEF, 3, 0, 0, 0, 0), paket(0xEF, 0x83, 3)),
    ("06 reg 4'e yaz",      paket(0xEF, 6, 0, 4, 0, 1), paket(0xEF, 0x86, 2)),
    ("06 LED = 5",          paket(0xEF, 6, 0, 1, 0, 5), paket(0xEF, 0x86, 3)),
    ("bilinmeyen fonk 04",  paket(0xEF, 4, 0, 0, 0, 1), paket(0xEF, 0x84, 1)),
    ("başka slave (0x05)",  paket(0x05, 3, 0, 0, 0, 1), None),
    ("bozuk CRC",           paket(0xEF, 3, 0, 0, 0, 1)[:-1] + b"\x00", None),
]

hx = lambda b: " ".join(f"{x:02X}" for x in b) if b else "(yok)"

print("=== 1. aşama: ham baytlar ===")
with serial.Serial(PORT, 9600, timeout=0.2) as s:
    time.sleep(2)              # port açılınca Arduino reset atar, bootloader'ı bekle
    s.reset_input_buffer()
    gecen = 0
    for ad, istek, beklenen in TESTLER:
        s.write(istek)
        cevap = s.read(32)
        ok = (cevap == beklenen) if beklenen else (len(cevap) == 0)
        gecen += ok
        print(f"{'✅' if ok else '❌'} {ad:20s}  →  {hx(cevap)}")
        if not ok:
            print(f"   beklenen:               {hx(beklenen)}")
        time.sleep(0.05)
    print(f"{gecen}/{len(TESTLER)} geçti\n")

    # zaman aşımı testi: yarım paket gönder, bekle, sonra tam paket
    s.write(paket(0xEF, 3, 0, 1, 0, 1)[:4])
    time.sleep(0.02)
    s.write(paket(0xEF, 3, 0, 1, 0, 1))
    cevap = s.read(32)
    print(f"yarım paket + 20 ms + tam paket  →  {hx(cevap)}")
    print("(yarım paket kendi başına atılmalı, tam pakete cevap gelmeli)\n")


# ---------------------------------------------------------------
# 2. AŞAMA: gerçek bir Modbus master (pymodbus)
# Senin kodunu hiç bilmeyen bir kütüphane seninle konuşabiliyor mu?
# ---------------------------------------------------------------
print("=== 2. aşama: pymodbus ===")
from pymodbus.client import ModbusSerialClient

client = ModbusSerialClient(port=PORT, baudrate=9600, bytesize=8,
                            parity="N", stopbits=1, timeout=1)
client.connect()
time.sleep(2)


def cagir(fonk, *args, **kw):
    # pymodbus sürümüne göre slave parametresinin adı farklı
    for anahtar in ("device_id", "slave", "unit"):
        try:
            return fonk(*args, **kw, **{anahtar: SLAVE})
        except TypeError:
            continue
    raise RuntimeError("pymodbus sürümü tanınmadı")


r = cagir(client.read_holding_registers, 0, count=5)
if r.isError():
    print("Okuma hatası:", r)
else:
    reg = r.registers
    uptime = (reg[2] << 16) | reg[3]
    print(f"kimlik   = 0x{reg[0]:04X}")
    print(f"LED      = {reg[1]}")
    print(f"uptime   = {uptime} ms ({uptime / 1000:.1f} sn)")
    print(f"sıcaklık = {reg[4] / 10} °C")

for deger in (1, 0, 1, 0):
    w = cagir(client.write_register, 1, deger)
    print(f"LED ← {deger}:  {'tamam' if not w.isError() else w}")
    time.sleep(0.5)

r = cagir(client.read_holding_registers, 4, count=3)   # 4, 5, 6 → 02 hatası beklenir
print("olmayan adres okuma:", r)

client.close()