#ifndef SWITCH_CFG_H
#define SWITCH_CFG_H

// Switchモード/PCモードでPCから設定を変えるためのHIDレポート上のプロトコル。
// ハード依存を持たせないので switch_test/host_test/test_switch_cfg.cpp がそのまま回せる。
//
// SwitchモードにはCDCが無いので感度変更サイトがシリアルで繋げない。
// HORI NSW-079互換の記述子には元から8バイトのOUTPUTレポート(usage 0x2621)があるので、
// **記述子を1バイトも変えずに** そこを設定コマンドの入口として使う。
// PCモード(VID 0xCAFE)には設定用のベンダー定義HIDインターフェース(instance 1)を足し、
// **同じバイト列プロトコル**をそのOUTPUTレポートでも喋る。
// 読み戻しは入力レポートのvendorバイトを1バイトずつ送るバースト。
// Switch実機はOUTPUTを送ってこないので、実機側の挙動には影響しない。

#include <stdint.h>
#include <stddef.h>

#define CFG_MAGIC 0x55
#define CFG_SYNC  0xA5   // 設定の読み戻しバーストの先頭。ホストはこれが来るまで読み捨てる
#define CFG_SYNC_STAT 0x5A   // 計測値のバーストの先頭。設定と取り違えないよう別の値にする
#define CFG_NKANDO 15
#define CFG_DUMP_LEN (1 + 2 * CFG_NKANDO + 1)   // SYNC + kando(LE16) + usb_mode_pref

// 計測値。SwitchモードにはCDCが無く `STAT` が読めないので、同じ数字をHID経由で返す。
// 「Switchモードだけ遅い」を推測でなく数字で切り分けるため、PCモードと同じ指標を取る
#define CFG_STAT_N   10
#define CFG_STAT_LEN (1 + 2 * CFG_STAT_N)
#define CFG_SYNC_KEY 0x6A          // キーマップ読み戻しバーストの先頭（0xA5/0x5Aと別の値）
#define CFG_KEY_N    4
#define CFG_KEY_LEN  (1 + CFG_KEY_N)

// V1.35のPC自動校正コマンド。連続波形はCDCへ送り、HIDは制御と状態通知だけに使う
#define CFG_CMD_SET            0x01
#define CFG_CMD_SAVE           0x02
#define CFG_CMD_DUMP           0x03
#define CFG_CMD_MODE           0x04
#define CFG_CMD_STAT           0x05
#define CFG_CMD_SETKEY         0x06
#define CFG_CMD_GETKEY         0x07
#define CFG_CMD_BOOT           0x08
#define CFG_CMD_CAL_CAPS       0x09
#define CFG_CMD_CAPTURE_START  0x0A
#define CFG_CMD_CAPTURE_STOP   0x0B
#define CFG_CMD_CAPTURE_ABORT  0x0C
#define CFG_CMD_PROFILE_DUMP   0x0D
#define CFG_CMD_PROFILE_SET    0x0E
#define CFG_CMD_PROFILE_MODE   0x0F
#define CFG_CMD_FLASH_DUMP     0x10
#define CFG_PROFILE_FIELD_N    21
enum {
    CFG_STAT_SMIN = 0,   // サンプリング周期 min(µs)
    CFG_STAT_SAVG,       // 同 avg
    CFG_STAT_SMAX,       // 同 max
    CFG_STAT_LATMAX,     // 打の検出→レポート送信 の最大(µs)
    CFG_STAT_LATAVG,     // 同 平均
    CFG_STAT_LATN,       // 同 件数
    CFG_STAT_H0, CFG_STAT_H1, CFG_STAT_H2, CFG_STAT_H3,   // センサーごとの検出打数
};

// OUTPUTレポートの解釈結果。呼び側がフラッシュ書き込みと送信を実行する
typedef struct {
    bool save;   // フラッシュへ保存する
    bool dump;   // 現在値を読み戻す
    bool stat;   // 計測値を読み戻す
    bool keys;   // キーマップを読み戻す
    bool boot;   // ファーム更新モードへ入る
    bool cal_caps;
    bool capture_start;
    bool capture_stop;
    bool capture_abort;
    bool profile_dump;
    bool profile_set;
    bool profile_mode;
    bool flash_dump;
    bool valid;
    uint8_t command;
    uint8_t schema;
    uint8_t session;
    uint8_t take;
    uint8_t flags;
    uint16_t duration_ms;
    uint8_t profile_field;
    uint16_t profile_value;
} cfg_action_t;

// 取得中に変更すると波形と設定の対応が壊れるコマンドだけを列挙する
static inline bool cfg_command_conflicts_with_capture(uint8_t command) {
    return command == CFG_CMD_SET || command == CFG_CMD_SAVE || command == CFG_CMD_MODE ||
           command == CFG_CMD_SETKEY || command == CFG_CMD_BOOT ||
           command == CFG_CMD_PROFILE_SET || command == CFG_CMD_PROFILE_MODE;
}

// CDC取得やV1.35 profileに依存する連続command範囲はPC modeだけで受ける
static inline bool cfg_command_pc_only(uint8_t command) {
    return command >= CFG_CMD_CAL_CAPS && command <= CFG_CMD_FLASH_DUMP;
}

// OUTPUTレポートを適用する。
//   [0]=0x55 [1]=cmd
//     0x01 SET  : [2]=index [3]=値lo [4]=値hi
//     0x02 SAVE
//     0x03 DUMP
//     0x04 MODE : [2]=0|1 次回起動時のモード
//     0x05 STAT : 計測値を読み戻す（読んだら統計はリセットされる）
//     0x06 SETKEY : [2]=index(0..3) [3]=HIDキーコード。4..0xFFのときだけ書く
//     0x07 GETKEY : キーマップを読み戻す（CFG_SYNC_KEY + 4バイトのバースト）
//     0x08 BOOT   : ファーム更新モードへ入る（呼び側はコールバック内でリセットしない）
//     0x09 CAL_CAPS      : V1.35校正能力と現在状態を読み戻す
//     0x0A CAPTURE_START : [2]=schema [3]=session [4]=take [5..6]=duration_ms [7]=flags
//     0x0B CAPTURE_STOP  : 現在テイクを確定し、残りのCDCデータを排出する
//     0x0C CAPTURE_ABORT : 現在テイクを破棄する
//     0x0D PROFILE_DUMP  : schema/mode/全21field/CRC状態を読み戻す
//     0x0E PROFILE_SET   : [2]=field(0..20) [3..4]=値LE16。RAM candidateだけを変える
//     0x0F PROFILE_MODE  : [2]=0 legacy | 1 pair-delay-v1。RAM candidateだけを変える
//     0x10 FLASH_DUMP    : 実flash先頭94byteを同期byte付きで読み戻す
static inline cfg_action_t cfg_apply(const uint8_t *buf, size_t len, int *kando, uint8_t *mode_pref, uint8_t *key_map) {
    cfg_action_t act = {};
    if (len < 2 || buf[0] != CFG_MAGIC) return act;
    act.command = buf[1];

    switch (buf[1]) {
        case CFG_CMD_SET:
            if (len >= 5 && buf[2] < CFG_NKANDO) {
                kando[buf[2]] = buf[3] | (buf[4] << 8);
                act.valid = true;
            }
            break;
        case CFG_CMD_SAVE: act.save = act.valid = true; break;
        case CFG_CMD_DUMP: act.dump = act.valid = true; break;
        case CFG_CMD_MODE:
            if (len >= 3 && buf[2] <= 1) {
                *mode_pref = buf[2];
                act.valid = true;
            }
            break;
        case CFG_CMD_STAT: act.stat = act.valid = true; break;
        case CFG_CMD_SETKEY:
            if (len >= 4 && buf[2] < CFG_KEY_N && buf[3] >= 4) {
                key_map[buf[2]] = buf[3];
                act.valid = true;
            }
            break;
        case CFG_CMD_GETKEY: act.keys = act.valid = true; break;
        case CFG_CMD_BOOT: act.boot = act.valid = true; break;
        case CFG_CMD_CAL_CAPS: act.cal_caps = act.valid = true; break;
        case CFG_CMD_CAPTURE_START:
            if (len >= 8) {
                act.capture_start = act.valid = true;
                act.schema = buf[2];
                act.session = buf[3];
                act.take = buf[4];
                act.duration_ms = (uint16_t)(buf[5] | (buf[6] << 8));
                act.flags = buf[7];
            }
            break;
        case CFG_CMD_CAPTURE_STOP: act.capture_stop = act.valid = true; break;
        case CFG_CMD_CAPTURE_ABORT: act.capture_abort = act.valid = true; break;
        case CFG_CMD_PROFILE_DUMP: act.profile_dump = act.valid = true; break;
        case CFG_CMD_PROFILE_SET:
            if (len >= 5 && buf[2] < CFG_PROFILE_FIELD_N) {
                act.profile_set = act.valid = true;
                act.profile_field = buf[2];
                act.profile_value = (uint16_t)(buf[3] | (buf[4] << 8));
            }
            break;
        case CFG_CMD_PROFILE_MODE:
            if (len >= 3 && buf[2] <= 1) {
                act.profile_mode = act.valid = true;
                act.profile_value = buf[2];
            }
            break;
        case CFG_CMD_FLASH_DUMP: act.flash_dump = act.valid = true; break;
    }
    return act;
}

// 読み戻しバーストの中身を組み立てる。out は CFG_DUMP_LEN バイト必要
static inline uint8_t cfg_build_dump(uint8_t *out, const int *kando, uint8_t mode_pref) {
    uint8_t n = 0;
    out[n++] = CFG_SYNC;
    for (int i = 0; i < CFG_NKANDO; i++) {
        uint16_t v = (uint16_t)kando[i];
        out[n++] = (uint8_t)(v & 0xFF);
        out[n++] = (uint8_t)(v >> 8);
    }
    out[n++] = mode_pref;
    return n;
}

// 計測値のバーストを組み立てる。out は CFG_STAT_LEN バイト必要。
// vals は CFG_STAT_N 個（範囲は呼び側で 0..65535 に丸めておくこと）
static inline uint8_t cfg_build_stat(uint8_t *out, const uint16_t *vals) {
    uint8_t n = 0;
    out[n++] = CFG_SYNC_STAT;
    for (int i = 0; i < CFG_STAT_N; i++) {
        out[n++] = (uint8_t)(vals[i] & 0xFF);
        out[n++] = (uint8_t)(vals[i] >> 8);
    }
    return n;
}

// キーマップの読み戻しバーストを組み立てる。out は CFG_KEY_LEN バイト必要
static inline uint8_t cfg_build_keys(uint8_t *out, const uint8_t *key_map) {
    uint8_t n = 0;
    out[n++] = CFG_SYNC_KEY;
    for (int i = 0; i < CFG_KEY_N; i++) out[n++] = key_map[i];
    return n;
}

#endif // SWITCH_CFG_H
