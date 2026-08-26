#ifndef LED_H
#define LED_H

// RP2040-Zero のオンボードWS2812(GP16)で今のUSBモードを表示する。
// SW1が無い基板ではBOOTSEL長押しがモード切替の唯一の手段なので、
// どちらのモードにいるかが見えないと操作できない。
//
// pico_status_led は使わない。あのライブラリの colored_status_led_set_on_with_color() は
// 内部で「既に点灯中」なら書き込みをスキップするため、色を変えても反映されない。

#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

extern int kando[15];

#define LED_PIN 16

#define LED_PC      0x000030   // 青 = PCキーボードモード
#define LED_SWITCH  0x300000   // 赤 = Switchタタコンモード
#define LED_OFF     0x000000

static PIO led_pio;
static uint led_sm;
static uint32_t led_shown = 0xFFFFFFFF;

// 打の表示が入力遅延を食っていないかを実測するための集計。STATで返して0に戻す。
// 1回1µs未満なので time_us_32() の分解能(1µs)では個々の値は0か1にしかならないが、
// 切り捨ての位相が呼び出しごとにばらつくので、合計÷回数を取れば平均は出る。
// タイマ読み2回分のオーバーヘッドも込みなので、出る値は実際より少し大きい
static uint32_t led_us_sum = 0;
static uint32_t led_us_max = 0;
static uint32_t led_calls  = 0;

static void led_stat_reset(void) { led_us_sum = 0; led_us_max = 0; led_calls = 0; }

// 0xRRGGBB を渡す。PIOはGRB順で左詰め32bitを食う
static void led_put_raw(uint32_t rgb) {
    uint32_t g = (rgb >> 8) & 0xff, r = (rgb >> 16) & 0xff, b = rgb & 0xff;
    pio_sm_put_blocking(led_pio, led_sm, (g << 24) | (r << 16) | (b << 8));
}

// kando[13]（5-100%）で減光してから出す。0を保存済みの旧設定も既定値へ戻し、モード表示は消さない。
static void led_put(uint32_t rgb) {
    uint32_t t0 = time_us_32();

    int pct = kando[13];
    if (pct < 5 || pct > 100) pct = 30;
    uint32_t r = (((rgb >> 16) & 0xff) * pct) / 100;
    uint32_t g = (((rgb >> 8)  & 0xff) * pct) / 100;
    uint32_t b = (( rgb        & 0xff) * pct) / 100;
    led_put_raw((r << 16) | (g << 8) | b);

    uint32_t d = time_us_32() - t0;
    led_us_sum += d;
    if (d > led_us_max) led_us_max = d;
    led_calls++;
}

// 起動直後に赤→緑→青を出す。ここが光らなければピンかハードの問題で、
// 以降の表示ロジックを疑う必要がない
static void led_init(void) {
    led_pio = pio0;
    led_sm = (uint)pio_claim_unused_sm(led_pio, true);
    uint offset = pio_add_program(led_pio, &ws2812_program);
    ws2812_program_init(led_pio, led_sm, offset, LED_PIN, 800000, false);

    const uint32_t seq[] = {0x200000, 0x002000, 0x000020, LED_OFF};
    for (int i = 0; i < 4; i++) { led_put_raw(seq[i]); sleep_ms(100); }
}

// BOOTでブートROMへ渡す直前の印。ブートROMに移るとファームは止まるが、
// WS2812は最後に受け取った色を保持するのでマゼンタが残り続ける。これで
//   マゼンタ + UF2ドライブが出ない → BOOTは届いた。ホスト側の列挙の問題
//   マゼンタにならずに切断された   → BOOTが届いていない（別の原因で切れている）
// を実機を見るだけで切り分けられる。明るさ設定は通さない（0にしていると印が見えない）
static void led_boot_marker(void) {
    for (int i = 0; i < 3; i++) {
        led_put_raw(0x400040); sleep_ms(60);
        led_put_raw(LED_OFF);  sleep_ms(60);
    }
    led_put_raw(0x400040);
}

// PCは青、Switchは赤を接続状態や打鍵にかかわらず常時点灯する。
//
// 変化したときだけ書く。PIOのTX FIFOへ32bit積むだけなので1回あたり数十〜百数十サイクル
// （WS2812が24bitを吐く30µsはPIOが非同期でやるのでCPUは待たない。FIFOは4段あり、
// 書くのは色が変わる瞬間だけなので詰まらない）。
static void led_update(bool switch_mode) {
    uint32_t want = switch_mode ? LED_SWITCH : LED_PC;
    if (want != led_shown) { led_put(want); led_shown = want; }
}

#endif // LED_H
