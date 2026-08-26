#ifndef CALIBRATION_CAPTURE_H
#define CALIBRATION_CAPTURE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define CAL_SCHEMA_VERSION       1
#define CAL_WIRE_VERSION         2
#define CAL_TIMESTAMP_SCHEMA     1
#define CAL_TIMESTAMP_PHASE_MODULUS_US 1000
#define CAL_FIRMWARE_MAJOR       1
#define CAL_FIRMWARE_MINOR       35
#define CAL_MAX_CAPTURE_MS       1000
#define CAL_RING_BYTES           60000
#define CAL_SAMPLE_BYTES         10
#define CAL_PACKET_MAGIC_0       'H'
#define CAL_PACKET_MAGIC_1       'T'
#define CAL_PACKET_MAGIC_2       'C'
#define CAL_PACKET_MAGIC_3       'L'
#define CAL_PACKET_HEADER_BYTES  14
#define CAL_PACKET_PAYLOAD_BYTES 500
#define CAL_PACKET_CRC_BYTES     4
#define CAL_PACKET_MAX_BYTES     (CAL_PACKET_HEADER_BYTES + CAL_PACKET_PAYLOAD_BYTES + CAL_PACKET_CRC_BYTES)
#define CAL_TERMINAL_COUNTER_BYTES 20
#define CAL_TERMINAL_PHASE_OFFSET  CAL_TERMINAL_COUNTER_BYTES
#define CAL_TERMINAL_TIMESTAMP_SCHEMA_OFFSET 22
#define CAL_TERMINAL_PHASE_VALID_OFFSET 23
#define CAL_TERMINAL_PAYLOAD_BYTES 24
#define CAL_CAPS_SYNC            0xC9
#define CAL_STATUS_SYNC          0xCA
#define CAL_CAPS_LEN             30
#define CAL_STATUS_LEN           9

enum {
    CAL_PACKET_DATA = 1,
    CAL_PACKET_TERMINAL = 2,
};

enum {
    CAL_STATUS_IDLE = 0,
    CAL_STATUS_CAPTURING = 1,
    CAL_STATUS_DRAINING = 2,
    CAL_STATUS_COMPLETE = 3,
    CAL_STATUS_ABORTED = 4,
    CAL_STATUS_INVALID = 5,
    CAL_STATUS_UNSUPPORTED_MODE = 0x80,
    CAL_STATUS_BUSY = 0x81,
    CAL_STATUS_BAD_REQUEST = 0x82,
    CAL_STATUS_BAD_STATE = 0x83,
    CAL_STATUS_CONFLICT = 0x84,
};

enum {
    CAL_FLAG_OVERFLOW = 1 << 0,
    CAL_FLAG_DT_SATURATED = 1 << 1,
    CAL_FLAG_AUTO_STOP = 1 << 2,
};

// 16bit値をCPUのアラインメントに依存せずwireへ書く
static inline void cal_put_u16(uint8_t *out, uint16_t value) {
    out[0] = (uint8_t)(value & 0xFF);
    out[1] = (uint8_t)(value >> 8);
}

// 32bit値をCPUのアラインメントに依存せずwireへ書く
static inline void cal_put_u32(uint8_t *out, uint32_t value) {
    out[0] = (uint8_t)(value & 0xFF);
    out[1] = (uint8_t)((value >> 8) & 0xFF);
    out[2] = (uint8_t)((value >> 16) & 0xFF);
    out[3] = (uint8_t)(value >> 24);
}

// Ethernet/ZIPと同じ反転IEEE CRC32を計算する
static inline uint32_t cal_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)-(int32_t)(crc & 1u));
        }
    }
    return ~crc;
}

// WebHIDの1バイトずつの読み戻し用に版付きcapabilityを組み立てる
static inline uint8_t cal_build_caps(uint8_t *out, uint8_t state, uint8_t session, uint8_t take) {
    out[0] = CAL_CAPS_SYNC;
    out[1] = CAL_SCHEMA_VERSION;
    out[2] = CAL_FIRMWARE_MAJOR;
    out[3] = CAL_FIRMWARE_MINOR;
    cal_put_u16(out + 4, CAL_MAX_CAPTURE_MS);
    cal_put_u16(out + 6, CAL_RING_BYTES);
    out[8] = CAL_SAMPLE_BYTES;
    out[9] = CAL_PACKET_HEADER_BYTES;
    cal_put_u16(out + 10, CAL_PACKET_PAYLOAD_BYTES);
    out[12] = 0x43;
    out[13] = 0x07;
    out[14] = state;
    out[15] = CAL_WIRE_VERSION;
    out[16] = session;
    out[17] = take;
    out[18] = CAL_PACKET_DATA;
    out[19] = CAL_PACKET_TERMINAL;
    out[20] = 3;
    out[21] = 2;
    out[22] = 1;
    out[23] = 0;
    cal_put_u16(out + 24, CAL_TERMINAL_PAYLOAD_BYTES);
    out[26] = CAL_TIMESTAMP_SCHEMA;
    out[27] = CAL_TERMINAL_PHASE_OFFSET;
    cal_put_u16(out + 28, CAL_TIMESTAMP_PHASE_MODULUS_US);
    return CAL_CAPS_LEN;
}

// 各制御コマンドの成否と現在状態を明示するバーストを組み立てる
static inline uint8_t cal_build_status(uint8_t *out, uint8_t result, uint8_t state, uint8_t session,
                                       uint8_t take, uint8_t flags, uint8_t command) {
    out[0] = CAL_STATUS_SYNC;
    out[1] = CAL_SCHEMA_VERSION;
    out[2] = result;
    out[3] = state;
    out[4] = session;
    out[5] = take;
    out[6] = flags;
    out[7] = command;
    out[8] = 0;
    return CAL_STATUS_LEN;
}

class CalibrationCapture {
public:
    // 新しいテイクを初期化し、指定時刻から取得を開始する
    uint8_t start(uint8_t schema, uint8_t session, uint8_t take, uint16_t duration_ms,
                  uint8_t requested_flags, uint64_t now_us) {
        if (busy()) return CAL_STATUS_BUSY;
        if (schema != CAL_SCHEMA_VERSION || duration_ms == 0 || duration_ms > CAL_MAX_CAPTURE_MS ||
            requested_flags != 0) return CAL_STATUS_BAD_REQUEST;

        state_ = CAL_STATUS_CAPTURING;
        session_ = session;
        take_ = take;
        flags_ = 0;
        duration_ms_ = duration_ms;
        start_us_ = now_us;
        ring_read_ = ring_write_ = ring_size_ = 0;
        tx_len_ = tx_pos_ = tx_payload_frames_ = 0;
        tx_type_ = 0;
        sequence_ = 0;
        attempted_frames_ = captured_frames_ = streamed_frames_ = 0;
        dropped_frames_ = overflow_count_ = 0;
        first_sample_phase_us_ = 0;
        first_sample_phase_valid_ = false;
        invalid_ = false;
        return state_;
    }

    // 明示STOPを受けたら新規frameを止め、リングとterminalの排出へ移る
    uint8_t stop() {
        if (state_ == CAL_STATUS_DRAINING) return state_;
        if (state_ != CAL_STATUS_CAPTURING) return CAL_STATUS_BAD_STATE;
        state_ = CAL_STATUS_DRAINING;
        return state_;
    }

    // ABORTは未送信データと送信途中packetを破棄し、ホスト側もsession/take単位で捨てる
    uint8_t abort() {
        if (!busy()) return CAL_STATUS_BAD_STATE;
        ring_read_ = ring_write_ = ring_size_ = 0;
        tx_len_ = tx_pos_ = tx_payload_frames_ = 0;
        tx_type_ = 0;
        state_ = CAL_STATUS_ABORTED;
        return state_;
    }

    // 指定時間を越えたテイクを自動停止し、上限外のrawを記録しない
    void tick(uint64_t now_us) {
        if (state_ != CAL_STATUS_CAPTURING) return;
        if (now_us - start_us_ >= (uint64_t)duration_ms_ * 1000u) {
            flags_ |= CAL_FLAG_AUTO_STOP;
            state_ = CAL_STATUS_DRAINING;
        }
    }

    // 完了したkeyboard updateを論理index 0..3のまま10byte frameへ詰める
    void record_sample(uint32_t dt_us, const uint16_t adc[4], uint64_t now_us) {
        tick(now_us);
        if (state_ != CAL_STATUS_CAPTURING) return;
        attempted_frames_++;

        uint16_t wire_dt = (uint16_t)dt_us;
        if (dt_us > 0xFFFFu) {
            wire_dt = 0xFFFFu;
            flags_ |= CAL_FLAG_DT_SATURATED;
            invalid_ = true;
        }

        // 1frameを丸ごと格納できない場合は部分書込みせず、テイクを即時無効化する
        if (CAL_RING_BYTES - ring_size_ < CAL_SAMPLE_BYTES) {
            dropped_frames_++;
            overflow_count_++;
            flags_ |= CAL_FLAG_OVERFLOW;
            invalid_ = true;
            state_ = CAL_STATUS_DRAINING;
            return;
        }

        uint8_t frame[CAL_SAMPLE_BYTES];
        cal_put_u16(frame, wire_dt);
        for (uint8_t i = 0; i < 4; i++) cal_put_u16(frame + 2 + i * 2, adc[i]);
        if (!first_sample_phase_valid_) {
            first_sample_phase_us_ = (uint16_t)(now_us % CAL_TIMESTAMP_PHASE_MODULUS_US);
            first_sample_phase_valid_ = true;
        }
        ring_push(frame, sizeof(frame));
        captured_frames_++;
    }

    // 送信可能なpacketがあれば未送信部分を返し、CDCが空ならnullptrを返す
    const uint8_t *tx_data(size_t *len) {
        if (tx_pos_ == tx_len_) prepare_packet();
        if (tx_pos_ == tx_len_) {
            *len = 0;
            return nullptr;
        }
        *len = tx_len_ - tx_pos_;
        return tx_packet_ + tx_pos_;
    }

    // TinyUSBへ実際に渡せたbyte数だけ進め、packet完了時に送信件数を確定する
    void tx_advance(size_t count) {
        size_t remaining = tx_len_ - tx_pos_;
        if (count > remaining) count = remaining;
        tx_pos_ += count;
        if (tx_pos_ != tx_len_) return;

        if (tx_type_ == CAL_PACKET_DATA) streamed_frames_ += tx_payload_frames_;
        if (tx_type_ == CAL_PACKET_TERMINAL) {
            state_ = invalid_ ? CAL_STATUS_INVALID : CAL_STATUS_COMPLETE;
        }
        tx_len_ = tx_pos_ = tx_payload_frames_ = 0;
        tx_type_ = 0;
    }

    uint8_t state() const { return state_; }
    uint8_t session() const { return session_; }
    uint8_t take() const { return take_; }
    uint8_t flags() const { return flags_; }
    bool capturing() const { return state_ == CAL_STATUS_CAPTURING; }
    bool busy() const { return state_ == CAL_STATUS_CAPTURING || state_ == CAL_STATUS_DRAINING; }
    size_t ring_size() const { return ring_size_; }
    uint32_t attempted_frames() const { return attempted_frames_; }
    uint32_t captured_frames() const { return captured_frames_; }
    uint32_t streamed_frames() const { return streamed_frames_; }
    uint32_t dropped_frames() const { return dropped_frames_; }
    uint32_t overflow_count() const { return overflow_count_; }

private:
    // byte ringへwrapを吸収して順番どおり格納する
    void ring_push(const uint8_t *data, size_t len) {
        for (size_t i = 0; i < len; i++) {
            ring_[ring_write_] = data[i];
            ring_write_ = (ring_write_ + 1) % CAL_RING_BYTES;
        }
        ring_size_ += len;
    }

    // byte ringからpacket payloadへwrapを吸収して取り出す
    void ring_pop(uint8_t *out, size_t len) {
        for (size_t i = 0; i < len; i++) {
            out[i] = ring_[ring_read_];
            ring_read_ = (ring_read_ + 1) % CAL_RING_BYTES;
        }
        ring_size_ -= len;
    }

    // 共通headerと末尾CRCを組み立て、sequenceはpacket生成ごとに1つ進める
    void build_packet(uint8_t type, uint16_t payload_len, uint8_t status) {
        tx_packet_[0] = CAL_PACKET_MAGIC_0;
        tx_packet_[1] = CAL_PACKET_MAGIC_1;
        tx_packet_[2] = CAL_PACKET_MAGIC_2;
        tx_packet_[3] = CAL_PACKET_MAGIC_3;
        tx_packet_[4] = CAL_WIRE_VERSION;
        tx_packet_[5] = type;
        tx_packet_[6] = session_;
        tx_packet_[7] = take_;
        cal_put_u16(tx_packet_ + 8, sequence_++);
        cal_put_u16(tx_packet_ + 10, payload_len);
        tx_packet_[12] = flags_;
        tx_packet_[13] = status;
        uint32_t crc = cal_crc32(tx_packet_, CAL_PACKET_HEADER_BYTES + payload_len);
        cal_put_u32(tx_packet_ + CAL_PACKET_HEADER_BYTES + payload_len, crc);
        tx_type_ = type;
        tx_len_ = CAL_PACKET_HEADER_BYTES + payload_len + CAL_PACKET_CRC_BYTES;
        tx_pos_ = 0;
    }

    // 取得中は500byte単位、停止後は残量単位でDATAを作り、最後にterminalを1つ作る
    void prepare_packet() {
        if (tx_pos_ != tx_len_) return;
        size_t payload_len = 0;
        if (ring_size_ >= CAL_PACKET_PAYLOAD_BYTES) payload_len = CAL_PACKET_PAYLOAD_BYTES;
        else if (state_ == CAL_STATUS_DRAINING) payload_len = ring_size_;

        if (payload_len != 0) {
            payload_len -= payload_len % CAL_SAMPLE_BYTES;
            ring_pop(tx_packet_ + CAL_PACKET_HEADER_BYTES, payload_len);
            tx_payload_frames_ = (uint16_t)(payload_len / CAL_SAMPLE_BYTES);
            build_packet(CAL_PACKET_DATA, (uint16_t)payload_len, CAL_STATUS_CAPTURING);
            return;
        }

        // DRAININGで全DATAをpacket化した後に全frame件数と無効理由を確定する
        if (state_ == CAL_STATUS_DRAINING) {
            uint8_t *payload = tx_packet_ + CAL_PACKET_HEADER_BYTES;
            cal_put_u32(payload + 0, attempted_frames_);
            cal_put_u32(payload + 4, captured_frames_);
            cal_put_u32(payload + 8, streamed_frames_);
            cal_put_u32(payload + 12, dropped_frames_);
            cal_put_u32(payload + 16, overflow_count_);
            cal_put_u16(payload + CAL_TERMINAL_PHASE_OFFSET, first_sample_phase_us_);
            payload[CAL_TERMINAL_TIMESTAMP_SCHEMA_OFFSET] = CAL_TIMESTAMP_SCHEMA;
            payload[CAL_TERMINAL_PHASE_VALID_OFFSET] = first_sample_phase_valid_ ? 1 : 0;
            build_packet(CAL_PACKET_TERMINAL, CAL_TERMINAL_PAYLOAD_BYTES,
                         invalid_ ? CAL_STATUS_INVALID : CAL_STATUS_COMPLETE);
        }
    }

    uint8_t ring_[CAL_RING_BYTES] = {0};
    uint8_t tx_packet_[CAL_PACKET_MAX_BYTES] = {0};
    size_t ring_read_ = 0;
    size_t ring_write_ = 0;
    size_t ring_size_ = 0;
    size_t tx_len_ = 0;
    size_t tx_pos_ = 0;
    uint16_t tx_payload_frames_ = 0;
    uint16_t sequence_ = 0;
    uint16_t duration_ms_ = 0;
    uint64_t start_us_ = 0;
    uint32_t attempted_frames_ = 0;
    uint32_t captured_frames_ = 0;
    uint32_t streamed_frames_ = 0;
    uint32_t dropped_frames_ = 0;
    uint32_t overflow_count_ = 0;
    uint16_t first_sample_phase_us_ = 0;
    uint8_t state_ = CAL_STATUS_IDLE;
    uint8_t session_ = 0;
    uint8_t take_ = 0;
    uint8_t flags_ = 0;
    uint8_t tx_type_ = 0;
    bool first_sample_phase_valid_ = false;
    bool invalid_ = false;
};

static_assert(sizeof(CalibrationCapture) <= 65536, "calibration buffering must stay within 64KiB");

#endif // CALIBRATION_CAPTURE_H
