#ifndef KEYS_H
#define KEYS_H

#include "hardware/gpio.h"
#include "class/hid/hid.h"
#include "hardware/adc.h"
#include "calibration_profile.h"

long int timer_f    = 0;
long int timer_d    = 0;
long int timer_j    = 0;
long int timer_k    = 0;
long int timer_edge = 0;
long int timer_face = 0;

struct PinKey {
    const uint8_t pin;
    const uint8_t key;
};

// kando[11] の既定は 1（1パッド2ボタン）。2本目は ZL/LS/RS/ZR で、ver1.2 が2年
// 動作している組み合わせなので安全。3本目以降(L Y R X)は未確認なので既定では使わない
int kando[15] = {
    80, 160, 160, 80, 30, 12, 35, 0, 0, 0, 25, 1, 0, 30, 0
};

uint8_t key_map[4] = {HID_KEY_D, HID_KEY_F, HID_KEY_J, HID_KEY_K};
/*
    0: 左縁の感度
    1: 左面の感度
    2: 右面の感度
    3: 右縁の感度
    4: 面が入力された際の次の入力受付時間 B
    5: 縁の入力受付時間 C
    6: 面入力時に縁を無視する時間 D
    7: シュミレーター用の入力制限 H
    8: 面の単体入力受付時間 A
    9: 縁の単体入力受付時間 A_edge
    10: Switch押下保持時間 T_HOLD(ms)
    11: Switch入力分散モード 0=なし 1=2系統 2=4系統
    12: Switch PID選択 0=0x0092 1=0x00C1 2=0x00F0
    13: LED明るさ 5-100(%)。ver1.3のケースはLEDが見えるので眩しさを実物合わせで詰める
    14: Switchキー配置 0=Proコン配置(本体メニューも叩いて操作) 1=タタコン配置(演奏専用)
*/

class KeyBoard {
private:
    static constexpr size_t num_pins = 4;
    const PinKey pin_keys[num_pins] = {
        {0, HID_KEY_D},
        {1, HID_KEY_F},
        {2, HID_KEY_J},
        {3, HID_KEY_K},
    };
    PairDelayGate pair_gate;
    bool pair_was_active = false;

public:
    uint8_t key_codes[6] = {0};
    long int result_ac[4] = {0};
    uint8_t hit_mask = 0;   // 発火したセンサーのビット。update()がtrueを返したときだけ読むこと
    bool sample_ready = false;   // ADC 4chを最後まで読んだupdateだけtrueになる
    uint32_t last_dt_us = 0;     // 完了frameのupdate先頭間隔。wire化時にuint16へ検証して詰める
    uint64_t last_sample_timestamp_us = 0; // 判定に使うupdate先頭の絶対時刻
    uint16_t last_adc[4] = {0};  // 論理index 0..3順。実ADC入力は既存どおり3-i

    // サンプリング実周期の統計。「固定周期10µs」が実機で本当に成立しているかを測るため
    uint32_t stat_min = 0xFFFFFFFF;
    uint32_t stat_max = 0;
    uint64_t stat_sum = 0;
    uint32_t stat_cnt = 0;

    // センサーごとの最大デルタ。叩かずに measure すればノイズフロア＝感度の下限が分かる
    uint16_t delta_max[4] = {0};

    // センサーごとの検出打数。ゲーム画面の判定数と比べれば、取りこぼしがファーム側かゲーム側か分かる
    uint32_t hit_cnt[4] = {0};

    // リセット直後の1サンプルは、STATの応答をCDCへ流している間の中断を含むので統計に入れない。
    // これを入れないと毎回のSTATが自分の出力による数msのギャップを stat_max として報告する
    bool stat_skip = true;

    void stat_reset() {
        stat_min = 0xFFFFFFFF;
        stat_max = 0;
        stat_sum = 0;
        stat_cnt = 0;
        stat_skip = true;
        for (int i = 0; i < 4; i++) { delta_max[i] = 0; hit_cnt[i] = 0; }
    }

    KeyBoard() {
        for (size_t i = 0; i < num_pins; i++) {
            gpio_init(pin_keys[i].pin);
            gpio_pull_up(pin_keys[i].pin);
            gpio_set_dir(pin_keys[i].pin, GPIO_IN);
        }
    }

    virtual ~KeyBoard() {}

    uint32_t millis() {
        return to_ms_since_boot(get_absolute_time());
    }

    bool update(bool pair_active = false, const calibration_profile_t *profile = nullptr) {
        sample_ready = false;
        static uint64_t last_sample_us = 0;
        uint64_t now_us = to_us_since_boot(get_absolute_time());
        uint32_t dt_us = (uint32_t)(now_us - last_sample_us);
        if (dt_us < 10) return false;

        // 実周期を記録する。初回(last_sample_us==0)は起動からの経過なので除外する
        if (last_sample_us != 0 && !stat_skip) {
            if (dt_us < stat_min) stat_min = dt_us;
            if (dt_us > stat_max) stat_max = dt_us;
            stat_sum += dt_us;
            stat_cnt++;
        }
        stat_skip = false;
        last_sample_us = now_us;
        last_dt_us = dt_us;
        last_sample_timestamp_us = now_us;

        // 判定msをsample時刻から一意に決め、rawのdt_usと位相から完全再生できるようにする
        long int main_timer = (long int)(now_us / 1000u);
        bool changed = false;
        uint8_t index = 0;

        // mode切替時はpairの受理履歴だけを消し、legacyのglobal timerには触らない
        if (pair_active != pair_was_active) {
            pair_gate.reset();
            pair_was_active = pair_active;
        }

        for (size_t i = 0; i < num_pins; i++) {
            key_codes[i] = 0;
        }
        hit_mask = 0;   // このupdate()での発火フラグをリセット

        for (size_t i = 0; i < num_pins; i++) {
            adc_select_input(3 - i);
            long int result = adc_read();
            last_adc[i] = (uint16_t)result;

            if (pair_active) {
                if (profile != nullptr &&
                    pair_gate.accept((uint8_t)i, (int32_t)(result - result_ac[i]), (uint32_t)main_timer, profile)) {
                    key_codes[index++] = key_map[i];
                    changed = true;
                    hit_mask |= (1 << i);
                    hit_cnt[i]++;
                }
            } else {
                if (result - result_ac[2] > kando[2] && i == 2 &&
                    main_timer - timer_face >= kando[4] &&
                    main_timer - timer_edge > kando[6] &&
                    main_timer - timer_j > kando[8]) {
                    key_codes[index++] = key_map[i];
                    changed = true;
                    hit_mask |= (1 << i);
                    hit_cnt[i]++;
                    timer_face = timer_j = main_timer;
                } else if (result - result_ac[1] > kando[1] && i == 1 &&
                           main_timer - timer_face >= kando[4] &&
                           main_timer - timer_edge > kando[6] &&
                           main_timer - timer_d > kando[8]) {
                    key_codes[index++] = key_map[i];
                    changed = true;
                    hit_mask |= (1 << i);
                    hit_cnt[i]++;
                    timer_face = timer_d = main_timer;
                } else if (result - result_ac[0] > kando[0] && i == 0 &&
                           main_timer - timer_edge >= kando[5] &&
                           main_timer - timer_face > kando[6] &&
                           main_timer - timer_f > kando[9]) {
                    key_codes[index++] = key_map[i];
                    changed = true;
                    hit_mask |= (1 << i);
                    hit_cnt[i]++;
                    timer_edge = timer_f = main_timer;
                } else if (result - result_ac[3] > kando[3] && i == 3 &&
                           main_timer - timer_edge >= kando[5] &&
                           main_timer - timer_face > kando[6] &&
                           main_timer - timer_k > kando[9]) {
                    key_codes[index++] = key_map[i];
                    changed = true;
                    hit_mask |= (1 << i);
                    hit_cnt[i]++;
                    timer_edge = timer_k = main_timer;
                }
            }

            // 判定式には触らず、デルタの最大値だけ記録する
            long int d = result - result_ac[i];
            if (d > (long int)delta_max[i]) delta_max[i] = (uint16_t)d;

            result_ac[i] = result;

            if (index >= 6) {
                break;
            }
        }

        sample_ready = true;
        return changed;
    }
};

#endif // KEYS_H
