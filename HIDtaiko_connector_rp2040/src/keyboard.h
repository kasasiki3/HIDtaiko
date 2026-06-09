#ifndef KEYS_H
#define KEYS_H

#include "hardware/gpio.h"
#include "class/hid/hid.h"
#include "../rp2_common/hardware_adc/include/hardware/adc.h"

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

int kando[10] = {
    80, 160, 160, 80, 30, 12, 35, 0, 0, 0
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

public:
    uint8_t key_codes[6] = {0};
    long int result_ac[4] = {0};

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

    bool update() {
        static uint64_t last_sample_us = 0;
        uint64_t now_us = to_us_since_boot(get_absolute_time());
        if (now_us - last_sample_us < 10) return false;
        last_sample_us = now_us;

        long int main_timer = millis();
        bool changed = false;
        uint8_t index = 0;

        for (size_t i = 0; i < num_pins; i++) {
            key_codes[i] = 0;
        }

        for (size_t i = 0; i < num_pins; i++) {
            adc_select_input(3 - i);
            long int result = adc_read();

            if (result - result_ac[2] > kando[2] && i == 2 &&
                main_timer - timer_face >= kando[4] &&
                main_timer - timer_edge > kando[6] &&
                main_timer - timer_j > kando[8]) {
                key_codes[index++] = key_map[i];
                changed = true;
                timer_face = timer_j = millis();
            } else if (result - result_ac[1] > kando[1] && i == 1 &&
                       main_timer - timer_face >= kando[4] &&
                       main_timer - timer_edge > kando[6] &&
                       main_timer - timer_d > kando[8]) {
                key_codes[index++] = key_map[i];
                changed = true;
                timer_face = timer_d = millis();
            } else if (result - result_ac[0] > kando[0] && i == 0 &&
                       main_timer - timer_edge >= kando[5] &&
                       main_timer - timer_face > kando[6] &&
                       main_timer - timer_f > kando[9]) {
                key_codes[index++] = key_map[i];
                changed = true;
                timer_edge = timer_f = millis();
            } else if (result - result_ac[3] > kando[3] && i == 3 &&
                       main_timer - timer_edge >= kando[5] &&
                       main_timer - timer_face > kando[6] &&
                       main_timer - timer_k > kando[9]) {
                key_codes[index++] = key_map[i];
                changed = true;
                timer_edge = timer_k = millis();
            }

            result_ac[i] = result;

            if (index >= 6) {
                break;
            }
        }

        return changed;
    }
};

#endif // KEYS_H
