#ifndef KEYS_H
#define KEYS_H

#include "hardware/gpio.h"  // GPIO操作
#include "class/hid/hid.h"  // HIDキー定数
#include "../rp2_common/hardware_adc/include/hardware/adc.h"

// タイマー配列（全体時間、ドン時間、カッ時間）
long int timer_f  = 0;
long int timer_d  = 0;
long int timer_j  = 0;
long int timer_k  = 0;
long int timer_edge = 0;
long int timer_face = 0;

// GPIOピンとキーコードの対応を表す構造体
struct PinKey {
    const uint8_t pin;  // Picoのピン番号
    const uint8_t key;  // HIDキーコード
};

// 初回接続フラグ
bool first_connect = false;

// 感度や入力受付時間の設定
int kando[9] = {
    80, 160, 160, 80, 30, 12, 35, 0, 0
};
/*
    0: 左縁の感度
    1: 左面の感度
    2: 右面の感度
    3: 右縁の感度
    4: 面が入力された際の次の入力受付時間 B
    5: 縁の入力受付時間 C
    6: 面入力時に縁を無視する時間 D
    7: シュミレーター用の入力制限 H
    8: 単体の入力受付時間 A
*/

// キーボードクラス
class KeyBoard {
private:
    static constexpr size_t num_pins = 4; // 使用ピン数
    const PinKey pin_keys[num_pins] = {
        {0, HID_KEY_C},//X,C,N,M
        {1, HID_KEY_X},
        {2, HID_KEY_N},
        {3, HID_KEY_M},
    };

public:
    uint8_t key_codes[6] = {0};  // HIDレポート用キーコード（最大6つ）
    long int result_ac[4] = {0}; // ADCの基準値

    // コンストラクタ（GPIO初期化）
    KeyBoard() {
        for (size_t i = 0; i < num_pins; i++) {
            gpio_init(pin_keys[i].pin);
            gpio_pull_up(pin_keys[i].pin);
		  // gpio_set_pulls(pin_keys[i].pin, false, false);  // ここを追加！
            gpio_set_dir(pin_keys[i].pin, GPIO_IN);
        }
    }

    // デストラクタ
    virtual ~KeyBoard() {}

    // 現在の時間（ミリ秒単位）
    uint32_t millis() {
        return to_ms_since_boot(get_absolute_time());
    }

    // キー状態を更新し、変更があればtrueを返す
    bool update() {
        long int main_timer = millis();
        bool changed = false;
        uint8_t index = 0;

        // キーコードを初期化
        for (size_t i = 0; i < num_pins; i++) {
            key_codes[i] = 0;
        }

        // 各ピンのADC値を取得し、キー入力を判定
        for (size_t i = 0; i < num_pins; i++) {
            adc_select_input(i);
            long int result = adc_read();
            /*
    0: 左縁の感度
    1: 左面の感度
    2: 右面の感度
    3: 右縁の感度
    4: 面が入力された際の次の入力受付時間 B
    5: 縁の入力受付時間 C
    6: 面入力時に縁を無視する時間 D
    7: シュミレーター用の入力制限 H
    8: 単体の入力受付時間 A
*/


            // 各キーの判定条件
            if (result - result_ac[2] > kando[2] && i == 2 &&//感度 J
                main_timer - timer_face >= kando[4] &&//面が入力されてからのdelay
                main_timer - timer_edge > kando[6] &&//縁が入力されてからのdelay
                main_timer - timer_j > kando[8]) {//個のdelay
                key_codes[index++] = pin_keys[i].key;
                changed = true;
                timer_face = timer_j = millis();
            }
            else if (result - result_ac[1] > kando[1] && i == 1 &&//感度 D
                     main_timer - timer_edge >= kando[5] &&//縁が入力されてからのdelay
                     main_timer - timer_face > kando[6] &&//面が入力されてからのdelay
                     main_timer - timer_d > kando[8]) {//個のdelay
                key_codes[index++] = pin_keys[i].key;
                changed = true;
                timer_edge = timer_d = millis();
            }
            
            else if (result - result_ac[0] > kando[0] && i == 0 &&//感度 F
                     main_timer - timer_face >= kando[4] &&//面が入力されてからのdelay
                     main_timer - timer_edge > kando[6] &&//縁が入力されてからのdelay
                     main_timer - timer_f > kando[8]) {//個のdelay
                key_codes[index++] = pin_keys[i].key;
                changed = true;
                timer_face = timer_f = millis();
            }
            else if (result - result_ac[3] > kando[3] && i == 3 &&//感度 K
                     main_timer - timer_edge >= kando[5] &&//縁が入力されてからのdelay
                     main_timer - timer_face > kando[6] &&//面が入力されてからのdelay
                     main_timer - timer_k > kando[8]) {//個のdelay
                key_codes[index++] = pin_keys[i].key;
                changed = true;
                timer_edge = timer_k = millis();
            }

            // ADC基準値を更新
            result_ac[i] = result;

            // 最大6つのキーまで処理
            if (index >= 6) {
                break;
            }
        }

        return changed;
    }
};

#endif // KEYS_H
