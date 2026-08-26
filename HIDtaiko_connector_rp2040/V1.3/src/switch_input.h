#ifndef SWITCH_INPUT_H
#define SWITCH_INPUT_H

// Switchモードのボタン割り当て。実機なしで検証できるよう、ハード依存を持たせない。
// test/test_switch_input.cpp がこのヘッダだけを include してホストで回す。

#include <stdint.h>

extern int kando[15];

// Switchのボタンビット
#define SW_Y  0x0001
#define SW_B  0x0002
#define SW_A  0x0004
#define SW_X  0x0008
#define SW_L  0x0010
#define SW_R  0x0020
#define SW_ZL 0x0040
#define SW_ZR 0x0080
#define SW_LS 0x0400
#define SW_RS 0x0800

// 十字キーはビットではなく4bitの数値1個（hat）で、同時に1方向しか表せない。
// ビットマスクとして扱えるよう仮想ビットに置き、送信直前に方向値へ畳む
#define SW_UP     (1u << 16)
#define SW_RIGHT  (1u << 17)
#define SW_DOWN   (1u << 18)
#define SW_LEFT   (1u << 19)
#define SW_HAT_MASK (SW_UP | SW_RIGHT | SW_DOWN | SW_LEFT)
#define SW_NBITS 20

// パッドごとの割り当て先。センサー番号 0=左縁 1=左面 2=右面 3=右縁。
// パッドを跨いで共有しないので、左右同時打（大ドン・大カツ）が確実に区別できる。
// 十字キーは仮想ビット(16..19)なので uint32_t で持つ（送信直前に sw_hat() が方向値へ畳む）。
//
// kando[14] でキー配置を選ぶ。違いは「1本目に何を置くか」だけで、使うボタンの集合は同じ。
// 遅い打は必ず1本目が出る（assign_hit のコメント参照）ので、ここが操作感を決める。
//
//   0 Proコン配置（既定）: 1本目が 方向←→ / B / A。「太鼓の達人 Nintendo Switch ば〜じょん！」
//     公式解説書 操作方法(1) でメニュー操作を兼ねているボタンなので、Switch本体のホーム画面まで
//     太鼓を叩くだけで操作できる。
//   1 タタコン配置: 1本目が実タタコン(NSW-079)と同じ ZL/LS/RS/ZR。公式解説書 操作方法(2) の
//     「左右のふちを叩いて項目の選択、面を叩いて決定」がそのまま効く。本体メニューは操作できない。
static const uint32_t pad_btns[2][4][4] = {
    {   // 0 Proコン配置
        {SW_LEFT,  SW_ZL, SW_L,  0    },   // 0 左縁 カッ(左)
        {SW_RIGHT, SW_LS, 0,     0    },   // 1 左面 ドン(左)
        {SW_B,     SW_RS, SW_Y,  0    },   // 2 右面 ドン(右)
        {SW_A,     SW_ZR, SW_R,  SW_X },   // 3 右縁 カッ(右)
    },
    {   // 1 タタコン配置
        {SW_ZL, SW_L,     SW_LEFT, 0    },
        {SW_LS, SW_RIGHT, 0,       0    },
        {SW_RS, SW_B,     SW_Y,    0    },
        {SW_ZR, SW_R,     SW_A,    SW_X },
    },
};
static const uint8_t pad_max[4] = {3, 2, 3, 4};

static uint32_t sw_buttons = 0;              // 下位16bit=ボタン、16..19=十字キー
static uint32_t sw_release_at[SW_NBITS] = {0};

// パッドの n 本目に対応するマスクを返す
static uint32_t pad_button(uint8_t pad, uint8_t idx) {
    return pad_btns[kando[14] ? 1 : 0][pad][idx];
}

// kando[11] が指す分散段数。パッドが持っている本数を超えない
static uint8_t spread_count(uint8_t pad) {
    uint8_t n;
    switch (kando[11]) {
        case 0:  n = 1; break;
        case 2:  n = 4; break;
        default: n = 2; break;
    }
    return n > pad_max[pad] ? pad_max[pad] : n;
}

// 打を1つ、そのパッドの空いているボタンへ割り当てる。
//
// **必ず1本目から順に空きを探す。** 打が遅ければ1本目は既に解放済みなので毎回1本目が出て、
// メニュー操作（＝1本目に置いたボタン）がそのまま効く。速くなって1本目が押しっぱなしに
// なったときだけ2本目以降へこぼれる。つまり「何打/秒を超えたら交互にする」という閾値を
// 決める必要がない。1ボタンの限界（T_HOLD）そのものが切り替え条件になる。
static void assign_hit(uint8_t pad, uint32_t now) {
    uint8_t n = spread_count(pad);
    uint8_t pick = 0;

    for (uint8_t idx = 0; idx < n; idx++) {
        if (!(sw_buttons & pad_button(pad, idx))) { pick = idx; goto found; }
    }
    // 全て押下中なら最も古く押されたものを解放して使う
    {
        uint8_t oldest = 0;
        for (uint8_t idx = 1; idx < n; idx++) {
            if (sw_release_at[__builtin_ctz(pad_button(pad, idx))] <
                sw_release_at[__builtin_ctz(pad_button(pad, oldest))])
                oldest = idx;
        }
        pick = oldest;
        sw_buttons &= ~pad_button(pad, pick);
    }
found:
    {
        uint32_t btn = pad_button(pad, pick);
        sw_buttons |= btn;
        sw_release_at[__builtin_ctz(btn)] = now + (uint32_t)kando[10];
    }
}

// 解放予定時刻を過ぎたボタンを離す
static void release_expired(uint32_t now) {
    for (uint8_t b = 0; b < SW_NBITS; b++) {
        if ((sw_buttons & (1u << b)) && now >= sw_release_at[b])
            sw_buttons &= ~(1u << b);
    }
}

// hit_mask のビットをパッドごとに振り分ける
static void assign_from_mask(uint8_t m, uint32_t now) {
    for (uint8_t s = 0; s < 4; s++) {
        if (m & (1 << s)) assign_hit(s, now);
    }
}

// 仮想ビットを hat の方向値へ畳む（0=上 2=右 4=下 6=左 8=中立）。
// 相反する方向が同時に立ったら、あとから押されたほうを優先する
static uint8_t sw_hat(void) {
    uint32_t h = sw_buttons & SW_HAT_MASK;
    if (!h) return 0x08;
    uint8_t best = 0xFF;
    uint32_t best_t = 0;
    const uint32_t dirs[4] = {SW_UP, SW_RIGHT, SW_DOWN, SW_LEFT};
    for (uint8_t i = 0; i < 4; i++) {
        if (!(h & dirs[i])) continue;
        uint32_t t = sw_release_at[__builtin_ctz(dirs[i])];
        if (best == 0xFF || t > best_t) { best = i; best_t = t; }
    }
    return (uint8_t)(best * 2);
}

#endif // SWITCH_INPUT_H
