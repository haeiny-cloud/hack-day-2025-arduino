# typing_speed_to_arduino.py
# pip install pynput pyserial
from pynput import keyboard
from collections import deque
import serial, time

PORT        = "/dev/tty.usbmodem1101"   # Macで [ls /dev/tty.usb*] で確認
BAUD        = 115200
WINDOW      = 1.0
TICK        = 0.1
ALPHA_UP    = 0.5
ALPHA_DOWN  = 0.15

ser = serial.Serial(PORT, BAUD, timeout=0)  # non-blocking
keystamps, smooth = deque(), 0.0

def is_char(key):
    return isinstance(key, keyboard.KeyCode) and key.char and key.char.isalpha()

def on_press(key):
    if is_char(key):
        keystamps.append(time.time())

def trim(now):
    while keystamps and now - keystamps[0] > WINDOW:
        keystamps.popleft()

def loop():
    global smooth
    while True:
        time.sleep(TICK)
        now = time.time()
        trim(now)
        raw = len(keystamps) / WINDOW
        alpha = ALPHA_UP if raw > smooth else ALPHA_DOWN
        smooth += alpha * (raw - smooth)          # 非対称 EMA
        rpm = int( map_range(smooth, 0, 25, 150, 255) )  # 0~25 KPS → 150~255
        ser.write(f"{rpm}\n".encode())            # arduinoにバイトを送信/改行
        print(f"{smooth:4.2f} kps → {rpm:3d}")

def map_range(x, in_min, in_max, out_min, out_max):
    return out_min + (out_max - out_min) * max(0, min(x, in_max)-in_min)/(in_max-in_min)

keyboard.Listener(on_press=on_press).start()
loop()