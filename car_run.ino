/*
  Car control using BLE_mini
 */

#include <ble_mini.h>

/**
 * ピンの定義
 *
 * signal1とsignal2で使用するピンに名前を付ける。
 */
const int SIGNAL1_1 = 9; // FET8
const int SIGNAL1_2 = 3; // FET2
const int SIGNAL2_1 = 7; // FET6
const int SIGNAL2_2 = 5; // FET4
const int LED = 13;

/**
 * 速度の設定 
 *
 * 0-255の値を指定する
 */
const int SPEED_MIN = 30;
const int SPEED_GEAR_WEIGHT = 25;
const int SPEED_GEAR_MAX = 9;
const int SPEED_GEAR_STOP = 0;

const int CAR_STATE_STOP = 0;
const int CAR_STATE_FORWARD = 1;
const int CAR_STATE_BACK = 2;
int current_state = CAR_STATE_STOP;
int current_speed = 255;
int current_gear = SPEED_GEAR_MAX;

/**
 * setup関数
 *
 * シリアル通信を使うため、初期化する。
 * モーターの制御に使用するピンをOUTPUTに指定する。
 */
void setup() {
  BLEMini_begin(57600);
  
  pinMode(LED, OUTPUT);
  
  pinMode(SIGNAL2_1, OUTPUT);
  pinMode(SIGNAL2_1, OUTPUT);
  pinMode(SIGNAL1_1, OUTPUT);
  pinMode(SIGNAL1_2, OUTPUT);
}

/**
 * signal1関数
 *
 * モーターの制御をわかりやすくするため、シグナル関数を用意する。
 * signal1関数とsignal2関数を使って、モーターの正転、逆転、停止を制御する。
 * FETモジュールの8番と2番は必ず反対の信号になる
 * signal1関数とsignal2関数に同時に1を渡すと、FETモジュールが壊れるので注意する。
 */
// 0:FET8=ON, 1:FET2=ON
void signal1(int value) {
  switch (value) {
    case 0:
      digitalWrite(SIGNAL1_1, HIGH); // FET8=5V
      digitalWrite(SIGNAL1_2, LOW); // FET2=0V
      break;
    case 1:
      digitalWrite(SIGNAL1_1, LOW); // FET8=0V
      analogWrite(SIGNAL1_2, current_speed); // FET2=5V(PWM)
      break;
  }
}

/**
 * signal2関数
 *
 * モーターの制御をわかりやすくするため、シグナル関数を用意する。
 * signal1関数とsignal2関数を使って、モーターの正転、逆転、停止を制御する。
 * FETモジュールの4番と6番は必ず反対の信号になる
 *
 * signal1関数とsignal2関数に同時に1を渡すと、FETモジュールが壊れるので注意する。
 */
// 0: FET6=ON, 1: FET4=ON
void signal2(int value) {
  switch (value) {
    case 0:
      digitalWrite(SIGNAL2_1, HIGH); // FET6=5V
      digitalWrite(SIGNAL2_2, LOW); // FET4=0V
      break;
    case 1:
      digitalWrite(SIGNAL2_1, LOW); // FET6=0V
      analogWrite(SIGNAL2_2, current_speed); // FET4=5V(PWM)
      break;
  }
}

/**
 * stop関数
 *
 * 停止コマンドが来た場合のステータスの変更を行う。
 */
void stop() {
  // 2, 4, 6, 8を切断
  signal1(0);
  signal2(0);
  current_state = CAR_STATE_STOP;
}

/**
 * forward関数
 *
 * 前進コマンドが来た場合のステータスの変更を行う。
 */
void forward() {
  signal1(1); // ON:FET2
  signal2(0); // ON:FET6
  current_state = CAR_STATE_FORWARD;
}

/**
 * back関数
 *
 * 後退コマンドが来た場合のステータスの変更を行う。
 */
void back() {
  signal1(0); // ON:FET8
  signal2(1); // ON:FET4
  current_state = CAR_STATE_BACK;
}

void change_speed_gear(int speed_gear){
  if(speed_gear > SPEED_GEAR_MAX || speed_gear < SPEED_GEAR_STOP) {
    return;
  }
  
  current_gear = speed_gear;
  if(speed_gear == SPEED_GEAR_STOP) {
    current_speed = 0;
    return;
  }
  
  current_speed = speed_gear * SPEED_GEAR_WEIGHT + SPEED_MIN;
}

/**
 * loop関数
 *
 * コマンドを受け取り、forward / back / stop のいずれかの適切な関数を呼ぶ。
 */
void loop() {
  if(!BLEMini_available()) {
    switch(current_state) {
      case CAR_STATE_FORWARD:
        forward();
        break;
      case CAR_STATE_BACK:
        back();
        break;
      case CAR_STATE_STOP:
        stop();
        break;
    }
  }
 
  while ( BLEMini_available()) {
    byte cmd;
    cmd = BLEMini_read();
    
    byte buf[] = {cmd, current_state+'0', current_gear+'0'};
    BLEMini_write_bytes(buf, 3);
  
   // Parse data here
    switch (cmd)
    {
      case 'f':
        forward();
        break;
      case 'b': // query board total pin count
        back();
        break;
      case 's': // query pin mode
        stop();
      case 'c':
        change_speed_gear(BLEMini_read() - '0');
        break;
    }
  }
  
  if(!BLEMini_available) {
    byte buf[] = {0, current_state+'0', current_gear+'0'};
    BLEMini_write_bytes(buf, 3);
  }
  
}
