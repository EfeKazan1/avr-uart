import serial, time

PORT = "/dev/cu.wchusbserial140"   # kendi portunu yaz: ls /dev/cu.usb*

s = serial.Serial(PORT, 9600, timeout=1)
time.sleep(2)                    # Arduino port açılınca resetlenir, açılmasını bekle

def gonder(baytlar, ad):
    s.reset_input_buffer()
    s.write(bytes(baytlar))
    cevap = s.read(64)
    print(f"{ad:12} gönderildi: {bytes(baytlar).hex(' ')}")
    print(f"{'':12} cevap:      {cevap.hex(' ') if cevap else '(yok)'}\n")

gonder([0xAA, 0x01, 0x01, 0x01, 0xE0, 0x50], "LED yak")
time.sleep(1)
gonder([0xAA, 0x01, 0x01, 0x00, 0x21, 0x90], "LED söndür")
gonder([0xAA, 0x00, 0x02, 0x80, 0x71],       "PING")

# Hata testi: CRC'nin son baytı bozuk → cevap gelmemeli
gonder([0xAA, 0x01, 0x01, 0x01, 0xE0, 0x51], "Bozuk CRC")

# Çöp + doğru paket → alıcı toparlanıp LED'i yakmalı
gonder([0x13, 0x37, 0xAA, 0x01, 0x01, 0x01, 0xE0, 0x50], "Çöp + LED")