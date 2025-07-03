from pynput import keyboard
from collections import deque
import time

WINDOW       = 1.0      # 즉시 측정 구간(초)
TICK         = 1.0      # 출력 주기(초)
ALPHA_UP     = 0.5     # 가속 계수
ALPHA_DOWN   = 0.3     # 감속 계수

keystamps    = deque()  # 최근 글자 입력 타임스탬프
smooth_kps   = 0.0      # 평활 Keys-Per-Second

def is_letter(key) -> bool:
    return isinstance(key, keyboard.KeyCode) and key.char and key.char.isalpha()

def on_press(key):
    if is_letter(key):               # 문자일 때만 기록
        now = time.time()
        keystamps.append(now)

def trim_window(now):
    while keystamps and now - keystamps[0] > WINDOW:
        keystamps.popleft()

def reporter():
    global smooth_kps
    while True:
        time.sleep(TICK)
        now = time.time()
        trim_window(now)

        raw_kps = len(keystamps) / WINDOW
        alpha   = ALPHA_UP if raw_kps > smooth_kps else ALPHA_DOWN
        smooth_kps += alpha * (raw_kps - smooth_kps)

        wpm = smooth_kps * 60 / 5
        print(f"{smooth_kps:4.2f} keys/s ≈ {wpm:5.1f} WPM")

if __name__ == "__main__":
    keyboard.Listener(on_press=on_press).start()
    reporter()