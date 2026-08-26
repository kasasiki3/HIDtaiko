#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>
#include <unistd.h>
#include <cstdlib>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/irq.h"
#include "pico/bootrom.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "keyboard.h"
#include "switch_input.h"
#include "switch_cfg.h"
#include "calibration_capture.h"
#include "led.h"


uint8_t usb_mode = 0;        // 0=PCキーボード 1=Switchタタコン（起動時に確定）
uint8_t usb_mode_pref = 0;   // フラッシュに保存する起動時モード
static bool sw1_open = true;  // 直近のSW1の位置。true=オープン(PC側) false=GND(Switch側)
static calibration_profile_t calibration_profile = {};
static bool calibration_profile_crc_valid = false;
static uint8_t legacy_prefix_raw[CAL_PROFILE_LEGACY_PREFIX_BYTES] = {0};
static bool profile_preserve_prefix_once = false;
static constexpr uint32_t SETTINGS_FLASH_OFFSET = 0x1F0000;

// 起動モードを決める。SW1(GPIO8)がLOWなら無条件でSwitch、
// オープン（SW1未実装の基板を含む）ならフラッシュに保存した設定に従う
static void read_mode_switch(void) {
#ifdef FORCE_SWITCH_MODE
    usb_mode = 1;
    return;
#endif
    gpio_init(8);
    gpio_set_dir(8, GPIO_IN);
    gpio_pull_up(8);
    sleep_us(100);
    sw1_open = gpio_get(8);
    usb_mode = sw1_open ? usb_mode_pref : 1;
}

static void save_setting_to_flash(void) {
    uint16_t buf16[20] = {0};
    static_assert(sizeof(buf16) == CAL_PROFILE_LEGACY_PREFIX_BYTES, "legacy flash prefix changed");
    for (int i = 0; i < 10; i++) buf16[i] = static_cast<uint16_t>(kando[i]);
    for (int i = 0; i < 4; i++) buf16[10 + i] = key_map[i];
    for (int i = 0; i < 3; i++) buf16[14 + i] = static_cast<uint16_t>(kando[10 + i]);
    buf16[17] = usb_mode_pref;
    buf16[18] = static_cast<uint16_t>(kando[13]);
    buf16[19] = static_cast<uint16_t>(kando[14]);

    static_assert(FLASH_PAGE_SIZE == CAL_PROFILE_PAGE_BYTES, "profile page must match flash program size");
    uint8_t buffer[FLASH_PAGE_SIZE];
    bool preserve_prefix = cal_profile_consume_prefix_preservation(&profile_preserve_prefix_once);
    cal_profile_build_save_page(buffer, legacy_prefix_raw, (const uint8_t *)buf16,
                                &calibration_profile, preserve_prefix);

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(SETTINGS_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(SETTINGS_FLASH_OFFSET, buffer, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    if (!preserve_prefix) memcpy(legacy_prefix_raw, buf16, sizeof(legacy_prefix_raw));
    calibration_profile_crc_valid = true;
}

void load_setting_from_flash(void) {
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + SETTINGS_FLASH_OFFSET);
    memcpy(legacy_prefix_raw, flash_data, sizeof(legacy_prefix_raw));

    uint16_t buf16[20];
    memcpy(buf16, flash_data, sizeof(buf16));

    // 未書き込みのフラッシュは 0xFFFF。そのまま入れると感度65535で一切反応しない機体になるので、
    // keyboard.h の初期値を残す。感度の上限は500程度なので 0xFFFF は保存値と衝突しない
    for (int i = 0; i < 10; i++) {
        if (buf16[i] != 0xFFFF) kando[i] = static_cast<int>(buf16[i]);
    }
    for (int i = 0; i < 4; i++) {
        uint16_t v = buf16[10 + i];
        if (v >= 4 && v <= 0xFF) key_map[i] = static_cast<uint8_t>(v);
    }
    // 旧フォーマットのフラッシュでは 0xFFFF が入るため既定値へ倒す
    uint16_t v;
    v = buf16[14]; kando[10] = (v >= 10 && v <= 200) ? v : 25;
    v = buf16[15]; kando[11] = (v <= 2) ? v : 1;
    v = buf16[16]; kando[12] = (v <= 2) ? v : 0;
    v = buf16[17]; usb_mode_pref = (v <= 1) ? (uint8_t)v : 0;
    v = buf16[18]; kando[13] = (v <= 100) ? v : 30;
    v = buf16[19]; kando[14] = (v <= 1) ? v : 0;

    // extensionが無い旧SAVEや破損時はcandidateもlegacy相当へ戻し、pairを有効にしない
    calibration_profile_crc_valid = cal_profile_decode_extension(
        &calibration_profile, flash_data + CAL_PROFILE_LEGACY_PREFIX_BYTES,
        FLASH_PAGE_SIZE - CAL_PROFILE_LEGACY_PREFIX_BYTES);
    if (!calibration_profile_crc_valid) cal_profile_from_legacy(&calibration_profile, kando);
    profile_preserve_prefix_once = false;
}


void hid_task(void);

// SAVE で起動モードが変わったときの実行予定時刻(ms)。0なら予定なし。
// 保存を先に確定させ、応答を送り切ってから列挙し直すために一拍置く
static uint32_t mode_apply_at = 0;

// 打を検出してから、そのレポートがUSBへ渡るまでの実測(µs)。
// **PCモードとSwitchモードで同じ指標を取る**のが目的。「Switchモードだけ遅い」を
// 推測でなく数字で切り分けられるようにする。両モードとも経路上は
// 「検出→即送信」のはずなので、差が出るならそこに原因がある
static uint64_t lat_hit_us = 0;   // 未送信の打の検出時刻。0なら待ちなし
static uint32_t lat_max = 0;
static uint32_t lat_cnt = 0;
static uint64_t lat_sum = 0;

// 打を検出した瞬間に呼ぶ。既に待ちがあるなら古いほうを残す（先に検出した打の遅延を測る）
static void lat_mark(void) {
    if (lat_hit_us == 0) lat_hit_us = to_us_since_boot(get_absolute_time());
}

// レポートを実際にUSBへ渡せた瞬間に呼ぶ
static void lat_done(void) {
    if (lat_hit_us == 0) return;
    uint32_t d = (uint32_t)(to_us_since_boot(get_absolute_time()) - lat_hit_us);
    lat_hit_us = 0;
    lat_sum += d;
    lat_cnt++;
    if (d > lat_max) lat_max = d;
}

static void lat_reset(void) { lat_hit_us = 0; lat_max = 0; lat_cnt = 0; lat_sum = 0; }
static uint32_t lat_avg(void) { return lat_cnt ? (uint32_t)(lat_sum / lat_cnt) : 0; }

KeyBoard keyboard;
CalibrationCapture calibration_capture;
void serial_task(void);
static void calibration_cdc_poll(void);

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void)itf;
    (void)dtr;
    (void)rts;
}

volatile bool data_received = false;

void tud_cdc_rx_cb(uint8_t itf) {
    data_received = true;
}

/*------------- メイン関数 -------------*/
int main(void) {
    board_init();
    led_init();                  // 起動直後に赤→緑→青。ここが光らなければLEDのハード側の問題
    load_setting_from_flash();   // usb_mode_pref を読んでからでないとモードが決まらない
    read_mode_switch();
    tusb_init();
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(28);
    adc_gpio_init(29);
    stdio_init_all();

    while (1) {
        tud_task();
        hid_task();
        calibration_cdc_poll();
        led_update(usb_mode != 0);
        if (data_received) {
            data_received = false;
            serial_task();
        }
    }
    return 0;
}

//-----------------------USB-----------------------
static char serial_line_buf[128] = {0};
static int serial_line_pos = 0;

// ファーム更新モードへ入る。ホストに切断と見なさせてからブートROMへ渡す。
// いきなり reset_usb_boot するとホストが古いデバイスノードを掴んだままになり、
// RPI-RP2 として列挙し直されないことがある。モード切替(apply_usb_mode)と同じ手順。
// シリアルのBOOTとHIDの0x08(BOOT)の両方から呼ぶ
static void enter_bootrom(void) {
    if (calibration_capture.busy()) calibration_capture.abort();
    led_boot_marker();      // ここまで来たことが実機に残る（led.h のコメント参照）
    tud_disconnect();
    sleep_ms(100);
    reset_usb_boot(0, 0);   // この関数から戻らない
}

static void process_serial_line(const char *line) {
    if (calibration_capture.busy()) return;
    if (strncmp(line, "SENS:", 5) == 0) {
        bool changed = false;
        char tmp[128];
        strncpy(tmp, line + 5, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        char *tok = strtok(tmp, ":");
        while (tok) {
            char *eq = strchr(tok, '=');
            if (eq) {
                *eq = '\0';
                int k = atoi(tok), v = atoi(eq + 1);
                if (k >= 0 && k < 15) {
                    kando[k] = v;
                    changed = true;
                }
            }
            tok = strtok(NULL, ":");
        }
        // 手動legacy設定を直ちに有効化し、次のSAVEは更新済みprefixを保存する
        if (changed) {
            cal_profile_activate_legacy(&calibration_profile, &profile_preserve_prefix_once);
            calibration_profile_crc_valid = true;
        }
        printf("OK\n");
    } else if (strncmp(line, "KEY:", 4) == 0) {
        bool changed = false;
        char tmp[32];
        strncpy(tmp, line + 4, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        char *tok = strtok(tmp, ":");
        while (tok) {
            char *eq = strchr(tok, '=');
            if (eq) {
                *eq = '\0';
                int k = atoi(tok), v = atoi(eq + 1);
                if (k >= 0 && k < 4) {
                    key_map[k] = static_cast<uint8_t>(v);
                    changed = true;
                }
            }
            tok = strtok(NULL, ":");
        }
        if (changed) cal_profile_cancel_prefix_preservation(&profile_preserve_prefix_once);
        printf("OK\n");
    } else if (strncmp(line, "MODE:", 5) == 0) {
        // 次回起動時のモード。0=PCキーボード 1=Switchタタコン。SAVEで確定する
        int v = atoi(line + 5);
        if (v == 0 || v == 1) {
            usb_mode_pref = (uint8_t)v;
            cal_profile_cancel_prefix_preservation(&profile_preserve_prefix_once);
        }
        printf("OK\n");
    } else if (strcmp(line, "GETS") == 0) {
        for (int i = 0; i < 15; i++) printf("%d:%d\n", i, kando[i]);
        printf("mode:%d\n", usb_mode_pref);
        printf("OK\n");
    } else if (strcmp(line, "STAT") == 0) {
        // サンプリング実周期(µs)を返して統計をリセットする
        printf("min:%u\n", keyboard.stat_min == 0xFFFFFFFF ? 0 : keyboard.stat_min);
        printf("max:%u\n", keyboard.stat_max);
        printf("avg:%u\n", keyboard.stat_cnt ? (uint32_t)(keyboard.stat_sum / keyboard.stat_cnt) : 0);
        printf("cnt:%u\n", keyboard.stat_cnt);
        for (int i = 0; i < 4; i++) printf("d%d:%u\n", i, keyboard.delta_max[i]);
        for (int i = 0; i < 4; i++) printf("h%d:%u\n", i, keyboard.hit_cnt[i]);
        // SW1(GPIO8)の実測値。1=オープン(PCモード側) 0=GND(Switchモード側)。
        // SW1未実装の基板ではプルアップで常に1になり、Switchモードへ入れない
        printf("sw1:%d\n", gpio_get(8) ? 1 : 0);
        printf("mode:%d\n", usb_mode);
        // 打の表示(WS2812への書き込み)がサンプリングをどれだけ食っているかの実測
        printf("ledsum:%u\n", led_us_sum);
        printf("ledmax:%u\n", led_us_max);
        printf("ledn:%u\n", led_calls);
        // 打の検出→レポート送信の実測。Switchモードでも同じ数字をHID経由で読める
        printf("latmax:%u\n", lat_max);
        printf("latavg:%u\n", lat_avg());
        printf("latn:%u\n", lat_cnt);
        printf("OK\n");
        keyboard.stat_reset();   // 応答を全部流し終えてからリセットする
        led_stat_reset();
        lat_reset();
    } else if (strcmp(line, "GETK") == 0) {
        for (int i = 0; i < 4; i++) printf("%d:%d\n", i, key_map[i]);
        printf("OK\n");
    } else if (strcmp(line, "SAVE") == 0) {
        save_setting_to_flash();
        printf("OK\n");
        // 起動モードが変わっていたらその場で列挙し直す（挿し直し不要）。
        // 保存とOKの送出を先に済ませてから切るので、ページは「保存完了」を見てから切断を知る
        if (usb_mode_pref != usb_mode) mode_apply_at = keyboard.millis() + 100;
    } else if (strcmp(line, "LOAD") == 0) {
        load_setting_from_flash();
        printf("OK\n");
    } else if (strcmp(line, "BOOT") == 0) {
        enter_bootrom();
    }
}

void serial_task(void) {
    char buf[64];
    int len;

    // バイナリ取得中は旧テキストコマンドを捨て、CDCの出力形式を混在させない
    if (calibration_capture.busy()) {
        while (tud_cdc_read(buf, sizeof(buf)) > 0);
        serial_line_pos = 0;
        return;
    }

    while ((len = tud_cdc_read(buf, sizeof(buf))) > 0) {
    for (int i = 0; i < len; i++) {
        char c = buf[i];
        if (c == '\n' || c == '\r') {
            if (serial_line_pos > 0) {
                serial_line_buf[serial_line_pos] = '\0';
                process_serial_line(serial_line_buf);
                serial_line_pos = 0;
            }
        } else if (serial_line_pos < (int)sizeof(serial_line_buf) - 1) {
            serial_line_buf[serial_line_pos++] = c;
        }
    }
    }
}

// ADC update外でTinyUSBの空きだけを書き、送信待ちによるbusy waitを作らない
static void calibration_cdc_poll(void) {
    if (usb_mode != 0 || !calibration_capture.busy() || !tud_cdc_connected()) return;
    size_t remaining = 0;
    const uint8_t *data = calibration_capture.tx_data(&remaining);
    if (data == nullptr || remaining == 0) return;

    uint32_t available = tud_cdc_write_available();
    if (available == 0) return;
    if (remaining > available) remaining = available;
    uint32_t written = tud_cdc_write(data, (uint32_t)remaining);
    calibration_capture.tx_advance(written);
    if (written != 0) tud_cdc_write_flush();
}

//-----------------------Switchモード-----------------------
// BOOTSELボタンの状態を読む。フラッシュを一時停止するのでRAM実行が必須
static bool __no_inline_not_in_flash_func(get_bootsel_button)(void) {
    const uint CS_PIN_INDEX = 1;
    uint32_t flags = save_and_disable_interrupts();
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);
    for (volatile int i = 0; i < 1000; ++i);
    bool pressed = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);
    restore_interrupts(flags);
    return pressed;
}

static uint32_t sw_sent = 0xFFFFFFFF;      // 前回送信値。初回必ず送るため異なる値で初期化
static uint32_t boot_poll_at = 0;          // 次にBOOTボタンを読む時刻(ms)

static uint8_t  init_seq_step = 0;      // 0..11 が押下/解放の12回、12で完了
static uint32_t init_seq_next_ms = 0;

// 起動時の認識用ダミー入力。完了したら true を返す
static bool init_sequence_done(uint32_t now) {
    if (init_seq_step >= 12) return true;
    if (init_seq_next_ms != 0 && now < init_seq_next_ms) return false;
    sw_buttons = (init_seq_step % 2 == 0) ? SW_LS : 0x0000;
    init_seq_next_ms = now + 400;
    init_seq_step++;
    return false;
}

// 現在のsw_buttonsを1レポート送る。vendorバイトは設定の読み戻しに使う（下記CFG参照）
static bool switch_send_raw(uint8_t vendor) {
    switch_report_t rep = {0};
    rep.buttons = (uint16_t)(sw_buttons & 0xFFFF);
    rep.hat = sw_hat();
    rep.lx = rep.ly = rep.rx = rep.ry = 0x80;
    rep.vendor = vendor;
    return tud_hid_report(0, &rep, sizeof(rep));
}

// 状態が変わったときだけ送る
static void switch_send(void) {
    if (!tud_hid_ready() || sw_buttons == sw_sent) return;
    if (switch_send_raw(0x00)) { sw_sent = sw_buttons; lat_done(); }
}

// PC/SwitchモードのPCからの設定変更（プロトコルは switch_cfg.h）。
// V1.35 flash readbackが全設定バースト中で最長なので、その固定長へ合わせる
static_assert(CAL_FLASH_DUMP_LEN >= CAL_PROFILE_DUMP_LEN && CAL_FLASH_DUMP_LEN >= CFG_DUMP_LEN &&
              CAL_FLASH_DUMP_LEN >= CAL_CAPS_LEN,
              "configuration tx buffer is too small");
static_assert(CFG_PROFILE_FIELD_N == CAL_PROFILE_FIELD_COUNT, "profile field protocol mismatch");
static uint8_t cfg_tx[CAL_FLASH_DUMP_LEN] = {0};
static uint8_t  cfg_tx_len = 0;
static uint8_t  cfg_tx_pos = 0;
static volatile bool cfg_save_req = false;   // フラッシュ書き込みはUSBコールバック内でやらない
static volatile bool cfg_flash_dump_req = false; // SAVE直後の要求は書き込み完了後に実フラッシュから組み立てる
static volatile bool cfg_boot_req = false;   // リセットはメインループでやる（enter_bootrom参照）

// フラッシュ書き込みは割り込みを数十ms止めるのでUSBコールバックの中ではやらない。
// 両モード共通のメインループから呼ぶ
static void cfg_save_poll(void) {
    if (!cfg_save_req || calibration_capture.busy()) return;
    cfg_save_req = false;
    save_setting_to_flash();
    if (usb_mode_pref != usb_mode) mode_apply_at = keyboard.millis() + 100;
}

// SAVEと同じループで要求されたreadbackは、erase/programの完了後にだけXIPから返す
static void cfg_flash_dump_poll(void) {
    if (!cfg_flash_dump_req || cfg_save_req) return;
    cfg_flash_dump_req = false;
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + SETTINGS_FLASH_OFFSET);
    cfg_tx_len = cal_profile_build_flash_dump(cfg_tx, flash_data);
    cfg_tx_pos = 0;
}

// Switchモードの送信処理。打の割り当て・解放・BOOTボタン・レポート送信を行う
static void switch_task(bool hit) {
    if (!tud_mounted()) return;
    uint32_t now = keyboard.millis();

    if (!init_sequence_done(now)) {
        switch_send();
        return;
    }

    // 読み戻しバースト中は1msに1バイトずつ吐くことだけをする
    if (cfg_tx_pos < cfg_tx_len) {
        if (tud_hid_ready() && switch_send_raw(cfg_tx[cfg_tx_pos])) cfg_tx_pos++;
        return;
    }

    release_expired(now);

    // 発火したセンサーをボタンへ割り当てる
    if (hit) { lat_mark(); assign_from_mask(keyboard.hit_mask, now); }

    // 状態が変わったときだけ送る
    switch_send();
}

// 指定モードでUSBを列挙し直す。persist=true のときだけ次回起動用の設定も書き換える
static void apply_usb_mode(uint8_t want, bool persist) {
    if (calibration_capture.busy()) calibration_capture.abort();
    if (persist) {
        usb_mode_pref = want;
        cal_profile_cancel_prefix_preservation(&profile_preserve_prefix_once);
        save_setting_to_flash();
    }
    tud_disconnect();
    sleep_ms(300);              // ホストに切断と見なさせる。切断中なので止めても害はない
    usb_mode = want;
    sw_buttons = 0;
    sw_sent = 0xFFFFFFFF;
    init_seq_step = 0;          // Switchへ戻るときは認識用ダミー入力をやり直す
    init_seq_next_ms = 0;
    tud_connect();
}

// SW1(GPIO8)はスライドスイッチ（SS12D00・GNDに落とすかオープンか）なので、
// 位置そのものが今いてほしいモードを表す。動かされたらその場で列挙し直す。
// フラッシュには書かない：スライド位置は物理状態で、サイトやBOOTSELが決める
// 「SW1が無い基板の起動時モード(usb_mode_pref)」とは別物
static void sw1_task(uint32_t now) {
#ifndef FORCE_SWITCH_MODE
    static bool pending = false;
    static bool pending_level = false;
    static uint32_t pending_since = 0;

    bool level = gpio_get(8);
    if (level != sw1_open) {
        // スライドスイッチはバウンスするので、50ms同じ位置が続いてから採用する
        if (!pending || level != pending_level) { pending = true; pending_level = level; pending_since = now; return; }
        if (now - pending_since < 50) return;
        pending = false;
        sw1_open = level;
        uint8_t want = sw1_open ? usb_mode_pref : 1;
        if (want != usb_mode) apply_usb_mode(want, false);
    } else {
        pending = false;
    }
#else
    (void)now;
#endif
}

// BOOTSELの単押しでPC/Switchを入れ替える。押した長さは問わず、**離した瞬間に1回**。
// Switchモードの A（決定）はBOOTSELに割り当てない。右縁の1本目が既にAなので、
// 太鼓を叩くだけでメニューは操作でき、ボタンの意味が押し方で変わるほうが分かりにくい。
//
// 読み取りはフラッシュを数十µs止めて打を取りこぼしうるので、直近500ms打がないときだけ読む。
// つまり演奏中は反応しない＝誤爆するのは曲間だけ
static void bootsel_task(uint32_t now, bool hit) {
    static uint32_t last_hit_ms = 0;
    static bool was_pressed = false;
    if (hit) { last_hit_ms = now; return; }
    if (now < boot_poll_at || now - last_hit_ms < 500) return;
    boot_poll_at = now + 100;

    bool pressed = get_bootsel_button();
    if (was_pressed && !pressed) {
        was_pressed = false;
        apply_usb_mode(usb_mode ? 0 : 1, true);
        return;
    }
    was_pressed = pressed;
}

static void send_hid_report(bool keys_pressed)
{
    static bool send_empty = false;
    static uint32_t last_send_time = 0;
    static bool waiting = false;
    static bool pending = false;
    static uint8_t pending_codes[6] = {0};

    // 打をまず控える。ここで控えないと、直前のレポートが送信中(tud_hid_ready()==false)の
    // タイミングに当たった打が、次のupdate()でkey_codesを消されて消滅する
    if (keys_pressed && !pending) {
        memcpy(pending_codes, keyboard.key_codes, sizeof(pending_codes));
        pending = true;
        lat_mark();
    }

    uint32_t now = keyboard.millis();

    if (!tud_hid_ready()) return;

    if (pending && !waiting)
    {
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, pending_codes);
        lat_done();
        pending = false;
        send_empty = true;
        waiting = true;
        last_send_time = now;
    }
    else if (waiting)
    {
        uint32_t hold_ms = cal_profile_pair_active(&calibration_profile, usb_mode)
                               ? calibration_profile.pc_h
                               : (uint32_t)kando[7];
        if (now - last_send_time >= hold_ms) {
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            waiting = false;
            send_empty = false;   // ここで解放を送ったので、下のelseで二重に送らない
        }
    }
    else
    {
        if (send_empty) {
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            send_empty = false;
        }
    }
}

void hid_task(void)
{
    bool const pair_active = cal_profile_pair_active(&calibration_profile, usb_mode);
    bool const keys_pressed = keyboard.update(pair_active, &calibration_profile);
    if (calibration_capture.capturing()) {
        if (keyboard.sample_ready) {
            calibration_capture.record_sample(keyboard.last_dt_us, keyboard.last_adc,
                                              keyboard.last_sample_timestamp_us);
        } else {
            uint64_t const now_us = to_us_since_boot(get_absolute_time());
            calibration_capture.tick(now_us);
        }
    }
    uint32_t const now_ms = keyboard.millis();

    if (mode_apply_at && now_ms >= mode_apply_at) {
        mode_apply_at = 0;
        apply_usb_mode(usb_mode_pref, false);   // 保存はSAVEで済んでいるので書き直さない
    }
    cfg_save_poll();
    cfg_flash_dump_poll();
    // BOOTはコールバック内でリセットせず、ここでホストに切断と見なさせてからブートROMへ渡す
    if (cfg_boot_req) {
        cfg_boot_req = false;
        enter_bootrom();
    }
    sw1_task(now_ms);
    if (tud_mounted()) bootsel_task(now_ms, keys_pressed);

    if (usb_mode) {
        switch_task(keys_pressed);
        return;
    }

    // 設定チャネル(instance 1)の読み戻しバーストを1バイトずつ流す。
    // キーボード(instance 0)とは別インターフェースなので、打鍵送信を止める必要はない
    if (cfg_tx_pos < cfg_tx_len) {
        uint8_t rep[8] = {0};
        rep[7] = cfg_tx[cfg_tx_pos];
        if (tud_hid_n_ready(1) && tud_hid_n_report(1, 0, rep, sizeof(rep))) cfg_tx_pos++;
    }

    if (tud_suspended() && keys_pressed)
    {
        tud_remote_wakeup();
    }
    else
    {
        send_hid_report(keys_pressed);
    }
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
{
    (void)instance;
    (void)report;
    (void)len;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

// PC/SwitchモードのOUTPUTレポートを設定コマンドとして解釈する（switch_cfg.h）。
// PCモードはキーボード(instance 0)ではなく設定用インターフェース(instance 1)だけ受ける
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    (void)report_id;
    (void)report_type;
    if (usb_mode == 0 && instance != 1) return;

    uint8_t command = (bufsize >= 2 && buffer[0] == CFG_MAGIC) ? buffer[1] : 0;

    // 送信中の1byte burstを別commandで上書きせず、ホスト側の直列制御契約を強制する
    if (cfg_tx_pos < cfg_tx_len) return;

    // 校正コマンドはCDCを持つPCモードだけで受け、Switch側には明示的な拒否状態を返す
    if (cfg_command_pc_only(command) && usb_mode != 0) {
        cfg_tx_len = cal_build_status(cfg_tx, CAL_STATUS_UNSUPPORTED_MODE, calibration_capture.state(),
                                      calibration_capture.session(), calibration_capture.take(),
                                      calibration_capture.flags(), command);
        cfg_tx_pos = 0;
        return;
    }

    // 取得中は波形と設定の対応を壊すSET/SAVE/MODE/SETKEY/BOOTを適用前に拒否する
    if (calibration_capture.busy() && cfg_command_conflicts_with_capture(command)) {
        cfg_tx_len = cal_build_status(cfg_tx, CAL_STATUS_CONFLICT, calibration_capture.state(),
                                      calibration_capture.session(), calibration_capture.take(),
                                      calibration_capture.flags(), command);
        cfg_tx_pos = 0;
        return;
    }

    cfg_action_t act = cfg_apply(buffer, bufsize, kando, &usb_mode_pref, key_map);

    // V1.3手動感度設定はlegacy detectorへ戻し、それ以外の通常変更もprefix保持予約を破棄する
    if (act.valid && act.command == CFG_CMD_SET) {
        cal_profile_activate_legacy(&calibration_profile, &profile_preserve_prefix_once);
        calibration_profile_crc_valid = true;
    } else if (act.valid && (act.command == CFG_CMD_SETKEY || act.command == CFG_CMD_MODE)) {
        cal_profile_cancel_prefix_preservation(&profile_preserve_prefix_once);
    }
    if (act.save) cfg_save_req = true;
    if (act.dump) {
        cfg_tx_len = cfg_build_dump(cfg_tx, kando, usb_mode_pref);
        cfg_tx_pos = 0;
    }
    if (act.keys) {
        cfg_tx_len = cfg_build_keys(cfg_tx, key_map);
        cfg_tx_pos = 0;
    }
    if (act.boot) cfg_boot_req = true;
    if (act.stat) {
        // 16bitに収まらない値は頭打ちにする。遅延が65msを超えていれば
        // 「65535」で頭打ちでも「桁が違う」ことは分かるので十分
        auto clamp = [](uint32_t v) -> uint16_t { return v > 0xFFFF ? 0xFFFF : (uint16_t)v; };
        uint16_t v[CFG_STAT_N];
        v[CFG_STAT_SMIN]   = clamp(keyboard.stat_min == 0xFFFFFFFF ? 0 : keyboard.stat_min);
        v[CFG_STAT_SAVG]   = clamp(keyboard.stat_cnt ? (uint32_t)(keyboard.stat_sum / keyboard.stat_cnt) : 0);
        v[CFG_STAT_SMAX]   = clamp(keyboard.stat_max);
        v[CFG_STAT_LATMAX] = clamp(lat_max);
        v[CFG_STAT_LATAVG] = clamp(lat_avg());
        v[CFG_STAT_LATN]   = clamp(lat_cnt);
        for (int i = 0; i < 4; i++) v[CFG_STAT_H0 + i] = clamp(keyboard.hit_cnt[i]);
        cfg_tx_len = cfg_build_stat(cfg_tx, v);
        cfg_tx_pos = 0;
        keyboard.stat_reset();   // CDCのSTATと同じく、読んだらリセットする
        lat_reset();
    }

    // CAPSは対応版・上限・wire layout・実ADC対応を版付きバーストで返す
    if (act.cal_caps) {
        cfg_tx_len = cal_build_caps(cfg_tx, calibration_capture.state(), calibration_capture.session(),
                                    calibration_capture.take());
        cfg_tx_pos = 0;
    }

    // STARTはCDCを開いてからだけ許可し、旧テキストの残りを破棄してbinary先頭を揃える
    if (act.capture_start) {
        uint8_t result;
        if (cfg_save_req || cfg_boot_req || mode_apply_at != 0) {
            result = CAL_STATUS_CONFLICT;
        } else if (!tud_cdc_connected()) {
            result = CAL_STATUS_BAD_STATE;
        } else {
            result = calibration_capture.start(act.schema, act.session, act.take, act.duration_ms,
                                               act.flags, to_us_since_boot(get_absolute_time()));
            if (result == CAL_STATUS_CAPTURING) {
                tud_cdc_read_flush();
                tud_cdc_write_clear();
                serial_line_pos = 0;
            }
        }
        cfg_tx_len = cal_build_status(cfg_tx, result, calibration_capture.state(),
                                      calibration_capture.session(), calibration_capture.take(),
                                      calibration_capture.flags(), act.command);
        cfg_tx_pos = 0;
    } else if (command == CFG_CMD_CAPTURE_START) {
        cfg_tx_len = cal_build_status(cfg_tx, CAL_STATUS_BAD_REQUEST, calibration_capture.state(),
                                      calibration_capture.session(), calibration_capture.take(),
                                      calibration_capture.flags(), command);
        cfg_tx_pos = 0;
    }

    // STOP/ABORTも結果を返し、ブラウザがbusyや二重操作を推測しなくてよいようにする
    if (act.capture_stop || act.capture_abort) {
        uint8_t result = act.capture_stop ? calibration_capture.stop() : calibration_capture.abort();
        cfg_tx_len = cal_build_status(cfg_tx, result, calibration_capture.state(),
                                      calibration_capture.session(), calibration_capture.take(),
                                      calibration_capture.flags(), act.command);
        cfg_tx_pos = 0;
    }

    // PROFILE_DUMPはschema/mode/21field/現在CRCを1byteずつ返す
    if (act.profile_dump) {
        cfg_tx_len = cal_profile_build_dump(cfg_tx, &calibration_profile, calibration_profile_crc_valid);
        cfg_tx_pos = 0;
    }

    // FLASH_DUMPはメインループへ渡し、直前のSAVEがあれば必ずその完了後にXIPから返す
    if (act.flash_dump) {
        cfg_flash_dump_req = true;
    }

    // PROFILE_SET/MODEはRAM candidateだけを変え、SAVEまではlegacy prefixもflashも変更しない
    if (act.profile_set || act.profile_mode) {
        bool applied;
        if (act.profile_set) {
            applied = cal_profile_set_field(&calibration_profile, act.profile_field, act.profile_value);
        } else {
            calibration_profile.mode = (uint8_t)act.profile_value;
            applied = true;
        }
        if (applied) {
            calibration_profile_crc_valid = true;
            if (calibration_profile.mode == CAL_PROFILE_MODE_PAIR_DELAY_V1) {
                cal_profile_arm_prefix_preservation(&profile_preserve_prefix_once);
            } else {
                cal_profile_cancel_prefix_preservation(&profile_preserve_prefix_once);
            }
        }
        cfg_tx_len = cal_build_status(cfg_tx, applied ? CAL_STATUS_IDLE : CAL_STATUS_BAD_REQUEST,
                                      calibration_capture.state(), calibration_capture.session(),
                                      calibration_capture.take(), calibration_capture.flags(), act.command);
        cfg_tx_pos = 0;
    } else if (command == CFG_CMD_PROFILE_SET || command == CFG_CMD_PROFILE_MODE) {
        cfg_tx_len = cal_build_status(cfg_tx, CAL_STATUS_BAD_REQUEST, calibration_capture.state(),
                                      calibration_capture.session(), calibration_capture.take(),
                                      calibration_capture.flags(), command);
        cfg_tx_pos = 0;
    }
}

void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
}
