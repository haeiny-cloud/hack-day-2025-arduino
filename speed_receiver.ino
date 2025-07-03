/*  typing_speed_receiver_nano.ino
 */

 #include <Arduino.h>

/* ───── 音階定義（周波数[Hz]）──────────────── */
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784

/* ───── メロディ（約10 秒：20音 × 500 ms）──── */
const int melody[] = {
  NOTE_E5, NOTE_D5, NOTE_C5, NOTE_D5,
  NOTE_E5, NOTE_E5, NOTE_E5, NOTE_D5,
  NOTE_D5, NOTE_D5, NOTE_E5, NOTE_G5,
  NOTE_G5, NOTE_E5, NOTE_D5, NOTE_C5,
  NOTE_D5, NOTE_E5, NOTE_D5, NOTE_C5
};
const int noteDur[] = { 8,8,8,8,8,8,4,8,8,4,8,8,4,8,8,8,8,8,8,8 };  
/* noteDur=8 → 500 ms（4=1 s, 2=2 s）. 合計 ≈10 s */

const byte ENA_PIN = 10;  // L293D 1番ピン → Nano D10（PWM）
const byte IN1_PIN = 9;   // L293D 2番ピン → Nano D9  （方向）
const byte IN2_PIN = 8;   // L293D 7番ピン → Nano D8  （方向）
const byte BUZZER_PIN = 8;    // パイゾブザー

String buf;               // シリアル受信用バッファ
bool     playing      = false;
uint8_t  noteIndex    = 0;
uint32_t noteStarted  = 0;
uint16_t noteLenMs    = 0;

void startSong() {
  noTone(BUZZER_PIN);         // 進行中でも即停止
  playing     = true;
  noteIndex   = 0;
  noteStarted = 0;
}
void stopSong() {
  noTone(BUZZER_PIN);
  playing = false;
}

void setup() {
  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // デフォルトの回転方向（前進）
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);

  analogWrite(ENA_PIN, 0);        // 初期状態は停止
  Serial.begin(115200);           // Python 側と同じボーレート
}

void loop() {
  /* 1) シリアル受信 */
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      int pwm = buf.toInt();
      buf = "";

      if (pwm > 0 && pwm < 100) pwm = 100;
      if (pwm > 255) pwm = 255;

      analogWrite(ENA_PIN, pwm);

      /* 255 なら毎回頭出し再生、230 未満なら停止 */
      if (pwm == 255) {
        startSong();                  // 冒頭から再生
      } else if (pwm < 230 && playing) {
        stopSong();
      }
    } else {
      buf += c;
      if (buf.length() > 5) buf = ""; // 過長保護
    }
  }

  /* 2) メロディ進行（非同期） */
  if (playing) {
    uint32_t now = millis();

    if (noteStarted == 0) {           // 新音開始
      noteLenMs   = 1000 / noteDur[noteIndex];
      tone(BUZZER_PIN, melody[noteIndex], noteLenMs);
      noteStarted = now;
    } else if (now - noteStarted >= noteLenMs * 1.3) {
      noteIndex++;
      if (noteIndex >= sizeof(melody) / sizeof(uint16_t)) {
        stopSong();                   // 曲終了
      } else {
        noteStarted = 0;              // 次音へ
      }
    }
  }
}