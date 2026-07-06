#ifndef KEYS_H
#define KEYS_H

#include "hardware/gpio.h"
#include "class/hid/hid.h"
#include "../rp2_common/hardware_adc/include/hardware/adc.h"
#include "usb_descriptors.h"

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
    // Matrix config: rows GP8,GP7,GP6 ; cols GP4,GP3,GP2
    const uint8_t matrix_rows[3] = {8, 7, 6};
    const uint8_t matrix_cols[3] = {4, 3, 2};
    const uint8_t matrix_map[9] = {
        HID_KEY_X,         // row0,col0 -> NORTH (maps to Switch X)
        HID_KEY_A,         // row0,col1 -> EAST  (maps to Switch A)
        HID_KEY_L,         // row0,col2 -> SOUTH (maps to Switch L)
        HID_KEY_Y,         // row1,col0 -> WEST  (maps to Switch Y)
        HID_KEY_B,         // row1,col1 -> B     (mapped)
        HID_KEY_R,         // row1,col2 -> R     (shoulder)
        HID_KEY_ESCAPE,    // row2,col0 -> START (+)
        HID_KEY_MINUS,     // row2,col1 -> SELECT (-)
        HID_KEY_HOME       // row2,col2 -> HOME (also used to toggle USB mode)
    };

    // Corner button state tracking (debounce)
    bool corner_button_pressed = false;
    long int corner_button_last_change = 0;
    static constexpr long int CORNER_DEBOUNCE_MS = 50;

    // Matrix button state tracking to emit a single input per press
    bool matrix_button_state[9] = {false};
    long int matrix_button_last_change[9] = {0};
    static constexpr long int MATRIX_DEBOUNCE_MS = 20;

    void updateLedState() {
        // LED ON when in Switch gamepad mode, OFF when in keyboard mode
        gpio_put(25, (g_usb_mode == USB_MODE_SWITCH_GAMEPAD) ? 1 : 0);
    }

public:
    uint8_t key_codes[6] = {0};
    long int result_ac[4] = {0};

    KeyBoard() {
        for (size_t i = 0; i < num_pins; i++) {
            gpio_init(pin_keys[i].pin);
            gpio_pull_up(pin_keys[i].pin);
            gpio_set_dir(pin_keys[i].pin, GPIO_IN);
        }

        // Init matrix pins
        for (int r = 0; r < 3; ++r) {
            gpio_init(matrix_rows[r]);
            gpio_set_dir(matrix_rows[r], GPIO_IN);
            gpio_pull_down(matrix_rows[r]);
        }
        for (int c = 0; c < 3; ++c) {
            gpio_init(matrix_cols[c]);
            gpio_set_dir(matrix_cols[c], GPIO_OUT);
            gpio_put(matrix_cols[c], 0);
        }

        // Init LED GP25 (RP2040 built-in LED) + set initial state
        gpio_init(25);
        gpio_set_dir(25, GPIO_OUT);
        updateLedState();
    }

    virtual ~KeyBoard() {}

    uint32_t millis() {
        return to_ms_since_boot(get_absolute_time());
    }

    // Map HID key codes to Switch gamepad buttons
    hid_switch_report_t getGamepadReport() {
        hid_switch_report_t report = {0};
        
        // Map matrix keys to Switch buttons
        for (int i = 0; i < 6 && key_codes[i] != 0; ++i) {
            switch (key_codes[i]) {
                // Match ITAIKO bit layout exactly:
                // bit0: Y, bit1: B, bit2: A, bit3: X
                case HID_KEY_Y:          report.buttons |= (1 << 0); break; // Y (west)
                case HID_KEY_B:          report.buttons |= (1 << 1); break; // B (south)
                case HID_KEY_A:          report.buttons |= (1 << 2); break; // A (east)
                case HID_KEY_X:          report.buttons |= (1 << 3); break; // X (north)

                // bit4: L, bit5: R (shoulders)
                case HID_KEY_L:          report.buttons |= (1 << 4); break; // L (shoulder)
                case HID_KEY_R:          report.buttons |= (1 << 5); break; // R (shoulder)
                case HID_KEY_Q:          report.buttons |= (1 << 4); break; // L (compat)
                case HID_KEY_E:          report.buttons |= (1 << 5); break; // R (compat)

                // bit6/7: ZL/ZR (side hits)
                case HID_KEY_D:          report.buttons |= (1 << 6); break; // ZL (ka_left)
                case HID_KEY_K:          report.buttons |= (1 << 7); break; // ZR (ka_right)

                // bit8: Select (-), bit9: Start (+)
                case HID_KEY_TAB:        report.buttons |= (1 << 8); break; // Select (-)
                case HID_KEY_ESCAPE:     report.buttons |= (1 << 9); break; // Start (+)

                // bit10/11: LS/RS (center hits)
                case HID_KEY_F:          report.buttons |= (1 << 10); break; // LS (don_left)
                case HID_KEY_J:          report.buttons |= (1 << 11); break; // RS (don_right)

                // bit12: Home, bit13: Capture
                case HID_KEY_HOME:       report.buttons |= (1 << 12); break; // Home
            }
        }
        
        // Analog sticks centered (neutral = 128 = 0x80)
        report.x = 0x80;
        report.y = 0x80;
        report.z = 0x80;
        report.rz = 0x80;
        
        // Hat switch (D-pad) = neutral
        report.hat = 0x0f;
        
        return report;
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

        // Matrix scan (append results up to 6 total keys)
        for (int c = 0; c < 3 && index < 6; ++c) {
            // drive only this column high
            for (int cc = 0; cc < 3; ++cc) gpio_put(matrix_cols[cc], (cc == c) ? 1 : 0);
            sleep_us(5);
            for (int r = 0; r < 3 && index < 6; ++r) {
                bool val = gpio_get(matrix_rows[r]);
                int pos = r * 3 + c;
                if (val) {
                    // Special handling for corner button (row2, col2, pos=8)
                    if (pos == 8) {
                        // Corner button: also acts as HOME key but keeps its toggle behavior.
                        if (!corner_button_pressed && (main_timer - corner_button_last_change) > CORNER_DEBOUNCE_MS) {
                            corner_button_pressed = true;
                            corner_button_last_change = main_timer;

                            // Add HOME keycode to the report if there's room
                            if (index < 6) {
                                key_codes[index++] = HID_KEY_HOME;
                            }

                            // One-way switch: only switch from Keyboard -> Switch.
                            // To return to Keyboard a reset/reconnect is required.
                            if (g_usb_mode == USB_MODE_KEYBOARD) {
                                usb_mode_switch(USB_MODE_SWITCH_GAMEPAD);
                            }
                            updateLedState();
                            changed = true;
                        }
                    } else {
                        uint8_t code = matrix_map[pos];
                        if (code != HID_KEY_NONE) {
                            if (!matrix_button_state[pos] && (main_timer - matrix_button_last_change[pos]) > MATRIX_DEBOUNCE_MS) {
                                matrix_button_state[pos] = true;
                                matrix_button_last_change[pos] = main_timer;
                                if (index < 6) {
                                    key_codes[index++] = code;
                                }
                                changed = true;
                            }
                        }
                    }
                } else if (pos < 8 && matrix_button_state[pos]) {
                    // Release detected: clear the state so the next press can be reported again.
                    matrix_button_state[pos] = false;
                    matrix_button_last_change[pos] = main_timer;
                }
            }
            // release column
            gpio_put(matrix_cols[c], 0);
        }
        
        // Detect falling edge of corner button
        if (corner_button_pressed) {
            // Check if button is still pressed by reading col2
            gpio_put(matrix_cols[2], 1);
            sleep_us(5);
            bool corner_still_pressed = gpio_get(matrix_rows[2]);
            gpio_put(matrix_cols[2], 0);
            
            if (!corner_still_pressed && (main_timer - corner_button_last_change) > CORNER_DEBOUNCE_MS) {
                corner_button_pressed = false;
                corner_button_last_change = main_timer;
            }
        }

        return changed;
    }
};

#endif // KEYS_H
