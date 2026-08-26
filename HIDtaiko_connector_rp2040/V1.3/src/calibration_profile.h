#ifndef CALIBRATION_PROFILE_H
#define CALIBRATION_PROFILE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define CAL_PROFILE_LEGACY_PREFIX_BYTES 40
#define CAL_PROFILE_PAGE_BYTES          256
#define CAL_PROFILE_SCHEMA              1
#define CAL_PROFILE_EXTENSION_BYTES     54
#define CAL_PROFILE_FIELD_COUNT         21
#define CAL_PROFILE_ROUTE_COUNT         12
#define CAL_PROFILE_DUMP_SYNC           0xCB
#define CAL_PROFILE_DUMP_LEN            53
#define CAL_FLASH_DUMP_SYNC             0xCC
#define CAL_FLASH_DUMP_BYTES            (CAL_PROFILE_LEGACY_PREFIX_BYTES + CAL_PROFILE_EXTENSION_BYTES)
#define CAL_FLASH_DUMP_LEN              (1 + CAL_FLASH_DUMP_BYTES)

enum {
    CAL_PROFILE_MODE_LEGACY = 0,
    CAL_PROFILE_MODE_PAIR_DELAY_V1 = 1,
};

typedef struct {
    uint8_t mode;
    uint16_t threshold[4];
    uint16_t self_face_b;
    uint16_t self_edge_c;
    uint16_t pc_h;
    uint16_t self_face_a;
    uint16_t self_edge_a;
    uint16_t pair_delay_ms[CAL_PROFILE_ROUTE_COUNT];
} calibration_profile_t;

// profile wireのlittle-endian 16bit値を読む
static inline uint16_t cal_profile_get_u16(const uint8_t *data) {
    return (uint16_t)(data[0] | (data[1] << 8));
}

// profile wireへlittle-endian 16bit値を書く
static inline void cal_profile_put_u16(uint8_t *out, uint16_t value) {
    out[0] = (uint8_t)(value & 0xFF);
    out[1] = (uint8_t)(value >> 8);
}

// profile wireのlittle-endian 32bit値を読む
static inline uint32_t cal_profile_get_u32(const uint8_t *data) {
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

// profile wireへlittle-endian 32bit値を書く
static inline void cal_profile_put_u32(uint8_t *out, uint32_t value) {
    out[0] = (uint8_t)(value & 0xFF);
    out[1] = (uint8_t)((value >> 8) & 0xFF);
    out[2] = (uint8_t)((value >> 16) & 0xFF);
    out[3] = (uint8_t)(value >> 24);
}

// extensionの完全性確認に反転IEEE CRC32を使う
static inline uint32_t cal_profile_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)-(int32_t)(crc & 1u));
        }
    }
    return ~crc;
}

// source-majorで対角を飛ばした12方向のindexを返す
static inline uint8_t cal_profile_route_index(uint8_t source, uint8_t target) {
    if (source >= 4 || target >= 4 || source == target) return 0xFF;
    return (uint8_t)(source * 3 + target - (target > source ? 1 : 0));
}

// Donは論理1/2、Kaは論理0/3として同種判定する
static inline bool cal_profile_same_type(uint8_t first, uint8_t second) {
    bool first_face = first == 1 || first == 2;
    bool second_face = second == 1 || second == 2;
    return first_face == second_face;
}

// extensionが無い機体でも安全な候補編集値をlegacy prefixから作る
static inline void cal_profile_from_legacy(calibration_profile_t *profile, const int *kando) {
    memset(profile, 0, sizeof(*profile));
    profile->mode = CAL_PROFILE_MODE_LEGACY;
    for (uint8_t i = 0; i < 4; i++) profile->threshold[i] = (uint16_t)kando[i];
    profile->self_face_b = (uint16_t)kando[4];
    profile->self_edge_c = (uint16_t)kando[5];
    profile->pc_h = (uint16_t)kando[7];
    profile->self_face_a = (uint16_t)kando[8];
    profile->self_edge_a = (uint16_t)kando[9];

    // legacy Dは異種8方向へ移し、同種4方向は既存B/Cだけで同じ境界を保つ
    for (uint8_t source = 0; source < 4; source++) {
        for (uint8_t target = 0; target < 4; target++) {
            uint8_t route = cal_profile_route_index(source, target);
            if (route != 0xFF && !cal_profile_same_type(source, target)) {
                profile->pair_delay_ms[route] = (uint16_t)kando[6];
            }
        }
    }
}

// field index 0..20を閾値・self値・12方向delayへ写像する
static inline bool cal_profile_get_field(const calibration_profile_t *profile, uint8_t index, uint16_t *value) {
    if (index < 4) *value = profile->threshold[index];
    else if (index == 4) *value = profile->self_face_b;
    else if (index == 5) *value = profile->self_edge_c;
    else if (index == 6) *value = profile->pc_h;
    else if (index == 7) *value = profile->self_face_a;
    else if (index == 8) *value = profile->self_edge_a;
    else if (index < CAL_PROFILE_FIELD_COUNT) *value = profile->pair_delay_ms[index - 9];
    else return false;
    return true;
}

// field index 0..20だけをcandidate profileへ書き、legacy kandoには触らない
static inline bool cal_profile_set_field(calibration_profile_t *profile, uint8_t index, uint16_t value) {
    if (index < 4) profile->threshold[index] = value;
    else if (index == 4) profile->self_face_b = value;
    else if (index == 5) profile->self_edge_c = value;
    else if (index == 6) profile->pc_h = value;
    else if (index == 7) profile->self_face_a = value;
    else if (index == 8) profile->self_edge_a = value;
    else if (index < CAL_PROFILE_FIELD_COUNT) profile->pair_delay_ms[index - 9] = value;
    else return false;
    return true;
}

// profileを固定54byte extensionへ直列化し、末尾CRCでheaderと全fieldを保護する
static inline uint16_t cal_profile_encode_extension(uint8_t *out, const calibration_profile_t *profile) {
    memset(out, 0, CAL_PROFILE_EXTENSION_BYTES);
    out[0] = 'H';
    out[1] = 'T';
    out[2] = 'P';
    out[3] = 'D';
    out[4] = CAL_PROFILE_SCHEMA;
    cal_profile_put_u16(out + 5, CAL_PROFILE_EXTENSION_BYTES);
    out[7] = profile->mode;
    for (uint8_t i = 0; i < CAL_PROFILE_FIELD_COUNT; i++) {
        uint16_t value = 0;
        cal_profile_get_field(profile, i, &value);
        cal_profile_put_u16(out + 8 + i * 2, value);
    }
    cal_profile_put_u32(out + 50, cal_profile_crc32(out, 50));
    return CAL_PROFILE_EXTENSION_BYTES;
}

// magic/schema/length/mode/CRCが1つでも不正ならprofileを採用しない
static inline bool cal_profile_decode_extension(calibration_profile_t *profile, const uint8_t *data, size_t available) {
    if (available < CAL_PROFILE_EXTENSION_BYTES) return false;
    if (data[0] != 'H' || data[1] != 'T' || data[2] != 'P' || data[3] != 'D') return false;
    if (data[4] != CAL_PROFILE_SCHEMA || cal_profile_get_u16(data + 5) != CAL_PROFILE_EXTENSION_BYTES) return false;
    if (data[7] > CAL_PROFILE_MODE_PAIR_DELAY_V1) return false;
    if (cal_profile_get_u32(data + 50) != cal_profile_crc32(data, 50)) return false;

    calibration_profile_t decoded = {};
    decoded.mode = data[7];
    for (uint8_t i = 0; i < CAL_PROFILE_FIELD_COUNT; i++) {
        cal_profile_set_field(&decoded, i, cal_profile_get_u16(data + 8 + i * 2));
    }
    *profile = decoded;
    return true;
}

// 既存40byteをそのまま複写し、その直後だけへextensionを置く
static inline void cal_profile_build_flash_page(uint8_t *out, const uint8_t *legacy_prefix,
                                                const calibration_profile_t *profile) {
    memset(out, 0, CAL_PROFILE_PAGE_BYTES);
    memcpy(out, legacy_prefix, CAL_PROFILE_LEGACY_PREFIX_BYTES);
    cal_profile_encode_extension(out + CAL_PROFILE_LEGACY_PREFIX_BYTES, profile);
}

// candidate確定直後の1回だけ保存済みraw prefix、それ以外は再構築prefixを選ぶ
static inline void cal_profile_build_save_page(uint8_t *out, const uint8_t *stored_prefix,
                                               const uint8_t *rebuilt_prefix,
                                               const calibration_profile_t *profile,
                                               bool preserve_prefix_once) {
    const uint8_t *prefix = preserve_prefix_once ? stored_prefix : rebuilt_prefix;
    cal_profile_build_flash_page(out, prefix, profile);
}

// candidate編集後の次回SAVEだけraw prefixを選ぶ予約を立てる
static inline void cal_profile_arm_prefix_preservation(bool *preserve_prefix_once) {
    *preserve_prefix_once = true;
}

// key/mode等の通常設定変更ではcandidate用のprefix保持予約を破棄する
static inline void cal_profile_cancel_prefix_preservation(bool *preserve_prefix_once) {
    *preserve_prefix_once = false;
}

// legacy感度変更を有効にし、pair candidateとprefix保持予約を同時に外す
static inline void cal_profile_activate_legacy(calibration_profile_t *profile, bool *preserve_prefix_once) {
    profile->mode = CAL_PROFILE_MODE_LEGACY;
    *preserve_prefix_once = false;
}

// SAVE開始時に保持予約を1回だけ取り出して必ず消費する
static inline bool cal_profile_consume_prefix_preservation(bool *preserve_prefix_once) {
    bool preserve = *preserve_prefix_once;
    *preserve_prefix_once = false;
    return preserve;
}

// WebHID readbackへschema/mode/全field/現在CRCと検証結果を並べる
static inline uint8_t cal_profile_build_dump(uint8_t *out, const calibration_profile_t *profile, bool crc_valid) {
    uint8_t extension[CAL_PROFILE_EXTENSION_BYTES];
    cal_profile_encode_extension(extension, profile);
    out[0] = CAL_PROFILE_DUMP_SYNC;
    out[1] = CAL_PROFILE_SCHEMA;
    cal_profile_put_u16(out + 2, CAL_PROFILE_EXTENSION_BYTES);
    out[4] = profile->mode;
    out[5] = crc_valid ? 1 : 0;
    out[6] = CAL_PROFILE_FIELD_COUNT;
    for (uint8_t i = 0; i < CAL_PROFILE_FIELD_COUNT; i++) {
        uint16_t value = 0;
        cal_profile_get_field(profile, i, &value);
        cal_profile_put_u16(out + 7 + i * 2, value);
    }
    memcpy(out + 49, extension + 50, 4);
    return CAL_PROFILE_DUMP_LEN;
}

// WebHIDから実フラッシュのlegacy prefixとextensionをそのまま検証できる形へ包む
static inline uint8_t cal_profile_build_flash_dump(uint8_t *out, const uint8_t *flash_data) {
    out[0] = CAL_FLASH_DUMP_SYNC;
    memcpy(out + 1, flash_data, CAL_FLASH_DUMP_BYTES);
    return CAL_FLASH_DUMP_LEN;
}

// Switchまたはlegacy modeではpair detectorを絶対に有効にしない
static inline bool cal_profile_pair_active(const calibration_profile_t *profile, uint8_t usb_mode) {
    return usb_mode == 0 && profile->mode == CAL_PROFILE_MODE_PAIR_DELAY_V1;
}

class PairDelayGate {
public:
    // mode切替時に受理履歴だけを消し、ADC baselineはkeyboard側に残す
    void reset() {
        memset(last_hit_ms_, 0, sizeof(last_hit_ms_));
        last_face_ms_ = last_edge_ms_ = 0;
        seen_mask_ = 0;
    }

    // 現行順0->3で正の1サンプル差分を評価し、受理時だけsource履歴を更新する
    bool accept(uint8_t target, int32_t delta, uint32_t now_ms, const calibration_profile_t *profile) {
        if (target >= 4 || delta <= (int32_t)profile->threshold[target]) return false;
        bool face = target == 1 || target == 2;
        uint32_t type_elapsed = now_ms - (face ? last_face_ms_ : last_edge_ms_);
        uint16_t type_delay = face ? profile->self_face_b : profile->self_edge_c;
        if (type_elapsed < type_delay) return false;

        uint32_t self_elapsed = now_ms - last_hit_ms_[target];
        uint16_t self_delay = face ? profile->self_face_a : profile->self_edge_a;
        if (self_elapsed <= self_delay) return false;

        // 同種routeはlegacy B/Cと同じ>=、Don<->Ka routeはlegacy Dと同じ>を使う
        for (uint8_t source = 0; source < 4; source++) {
            if (source == target || (seen_mask_ & (1u << source)) == 0) continue;
            uint8_t route = cal_profile_route_index(source, target);
            uint32_t elapsed = now_ms - last_hit_ms_[source];
            uint16_t delay = profile->pair_delay_ms[route];
            if (cal_profile_same_type(source, target)) {
                if (elapsed < delay) return false;
            } else if (elapsed <= delay) {
                return false;
            }
        }

        last_hit_ms_[target] = now_ms;
        if (face) last_face_ms_ = now_ms;
        else last_edge_ms_ = now_ms;
        seen_mask_ |= (uint8_t)(1u << target);
        return true;
    }

private:
    uint32_t last_hit_ms_[4] = {0};
    uint32_t last_face_ms_ = 0;
    uint32_t last_edge_ms_ = 0;
    uint8_t seen_mask_ = 0;
};

#endif // CALIBRATION_PROFILE_H
