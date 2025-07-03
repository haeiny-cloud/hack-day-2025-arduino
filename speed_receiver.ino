/*  typing_speed_receiver_nano.ino
 *  Arduino Nano + L293D（DCモーター1台）用スケッチ
 *
 *  ┌──────── L293D ───────┐          ┌──── Arduino Nano ────┐
 *  │ 1  EN1 (ENA) ────────┼── PWM → │ D10  (OC1B)           │
 *  │ 2  IN1 ──────────────┼─────────│ D2   (回転方向)         │
 *  │ 7  IN2 ──────────────┼─────────│ D3   (回転方向)         │
 *  │ 3  OUT1 ────┐ モーター.        ┌─┴── OUT2 6              │
 *  │ 4  GND      └────────────┐                             │
 *  │ 8  Vcc2 (モーター電源)      │                             │
 *  │16  Vcc1 (5 V)            │                            │
 *  └─────────────────────────┴────────────────────────────┘
 *
 *  • Python から “0〜255\n” の整数が送られてくる。
 *  • 受信した値を D10 に PWM 出力し、モーター速度を制御する。
 *  • IN1, IN2 は基本的に固定（HIGH/LOW）で今回の回転方向は前進のみ。
 */

const byte ENA_PIN = 10;  // L293D 1番ピン → Nano D10（PWM）
const byte IN1_PIN = 9;   // L293D 2番ピン → Nano D9  （方向）
const byte IN2_PIN = 8;   // L293D 7番ピン → Nano D8  （方向）

String buf;               // シリアル受信用バッファ

void setup() {
  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);

  // デフォルトの回転方向（前進）
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);

  analogWrite(ENA_PIN, 0);        // 初期状態は停止
  Serial.begin(115200);           // Python 側と同じボーレート
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {              // 行終端に到達
      int pwm = buf.toInt();      // 数値文字列 → 整数
      buf = "";                   // バッファクリア

      pwm = constrain(pwm, 0, 255);
      analogWrite(ENA_PIN, pwm);  // モーターに PWM 反映
    } else {
      buf += c;                   // 文字を蓄積
      if (buf.length() > 4) buf = "";  // 上限超過時はリセットして保護
    }
  }
}