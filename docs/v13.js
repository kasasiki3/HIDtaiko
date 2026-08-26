let kando = [200, 200, 200, 200, 200, 200, 200, 200, 200, 0, 25, 1, 0, 30, 0, 1];

// HID key codes: index = key_map[0..3] (左縁, 左面, 右面, 右縁)
let key_map = [0x07, 0x09, 0x0D, 0x0E]; // D, F, J, K

const KEY_OPTIONS = [
  {label: 'A', code: 0x04}, {label: 'B', code: 0x05}, {label: 'C', code: 0x06},
  {label: 'D', code: 0x07}, {label: 'E', code: 0x08}, {label: 'F', code: 0x09},
  {label: 'G', code: 0x0A}, {label: 'H', code: 0x0B}, {label: 'I', code: 0x0C},
  {label: 'J', code: 0x0D}, {label: 'K', code: 0x0E}, {label: 'L', code: 0x0F},
  {label: 'M', code: 0x10}, {label: 'N', code: 0x11}, {label: 'O', code: 0x12},
  {label: 'P', code: 0x13}, {label: 'Q', code: 0x14}, {label: 'R', code: 0x15},
  {label: 'S', code: 0x16}, {label: 'T', code: 0x17}, {label: 'U', code: 0x18},
  {label: 'V', code: 0x19}, {label: 'W', code: 0x1A}, {label: 'X', code: 0x1B},
  {label: 'Y', code: 0x1C}, {label: 'Z', code: 0x1D},
  {label: 'Space', code: 0x2C},
  {label: '←', code: 0x50}, {label: '→', code: 0x4F},
  {label: '↑', code: 0x52}, {label: '↓', code: 0x51},
];

const BUTTON_CLICK_EVENT_keymap_send = document.getElementById("keyMapSendButton");
const BUTTON_CLICK_EVENT_keymap_request = document.getElementById("keyMapRequestButton");
const BUTTON_CLICK_EVENT_DISC= document.getElementById("disconnection");
const BUTTON_CLICK_EVENT_send = document.getElementById("sendButton");
const BUTTON_CLICK_EVENT_request = document.getElementById("requestButton");
const BUTTON_CLICK_EVENT_stat_reset = document.getElementById("statResetButton");
const BUTTON_CLICK_EVENT_stat_read = document.getElementById("statReadButton");
const situation_alert = document.getElementById("situation");

let isBusy = false;
let hidDevice = null;

// 更新ボタンは、このページでユーザーが接続した実機へBOOTコマンドを送る。
window.HIDtaikoFirmwareFlasher?.setActiveHidDeviceProvider(() => hidDevice);

// ステータス文言はHTML側のi18n spanを使えないのでJS側に辞書を持つ
const MSG = {
  idle:        ["未接続", "Not connected"],
  needConnect: ["未接続です。先に「接続」を押してください", "Not connected. Click \"Connect\" first"],
  selecting:   ["選択中...", "Selecting..."],
  connectFail: ["接続失敗", "Connection failed"],
  disconnected:["切断", "Disconnected"],
  sending:     ["送信中...", "Sending..."],
  sent:        ["送信・保存完了", "Sent and saved"],
  sendFail:    ["送信失敗", "Send failed"],
  receiving:   ["受信中...", "Receiving..."],
  received:    ["受信完了", "Received"],
  receiveFail: ["受信失敗", "Receive failed"],
  bootOk:      ["ファーム更新モードに入りました。UF2ドライブへ .uf2 をドラッグしてください",
                "Firmware update mode. Drag the .uf2 onto the UF2 drive"],
  bootFail:    ["ファーム更新モードへの切替に失敗", "Could not enter firmware update mode"],
  bootNoReply: ["接続器が応答しません。ファームが古いか、BOOTに対応していない可能性があります",
                "The device did not respond. Its firmware may be too old to support this."],
  statStart:   ["計測開始。叩いたあと「結果を読む」を押してください",
                "Measuring. Hit the drum, then click \"Read\""],
  statStartFail:["計測開始に失敗", "Could not start measuring"],
  statRead:    ["計測結果を表示しました（統計はリセット済み）", "Results shown (counters reset)"],
  statReadFail:["計測に失敗", "Measurement failed"],
  keySending:  ["キー配置送信中...", "Sending key layout..."],
  keySent:     ["キー配置送信・保存完了", "Key layout sent and saved"],
  keySendFail: ["キー配置送信失敗", "Key layout send failed"],
  keyReceiving:["キー配置受信中...", "Receiving key layout..."],
  keyReceived: ["キー配置受信完了", "Key layout received"],
  keyReceiveFail:["キー配置受信失敗", "Key layout receive failed"],
  hidConnected:["接続済み", "Connected"],
  hidUnsupported:["このブラウザでは接続できません。パソコン版の Chrome または Edge で開いてください（スマートフォン・Safari・Firefox は非対応です）",
                  "This browser cannot connect. Open this page in desktop Chrome or Edge (phones, Safari and Firefox are not supported)"],
  hidNotFound: ["デバイスが見つかりません。USBケーブルを挿し直してもう一度お試しください（充電専用ケーブルでは認識しません）",
                "No device found. Re-plug the USB cable and try again (charge-only cables will not work)"],
  modeSwitched:["保存・切替完了", "Saved and switched"],
};

// 起動モード(PC/Switch)のスイッチ。checked=Switch。表示名も一緒に面倒を見る
const usbModeSwitch = document.getElementById("usbModeSwitch");
const usbModeName = document.getElementById("usbModeName");
function currentUsbMode() {
  return hidDevice ? (hidDevice.vendorId === 0x0F0D ? 1 : 0) : modeValue();
}
function modeValue() { return usbModeSwitch && usbModeSwitch.checked ? 1 : 0; }
function setModeValue(v) {
  if (usbModeSwitch) usbModeSwitch.checked = !!v;
  if (usbModeName) usbModeName.textContent = v ? "Switch" : "PC";
}
if (usbModeSwitch) usbModeSwitch.addEventListener("change", () => setModeValue(modeValue()));

let currentMsg = null;
function setStatus(key, detail) {
  currentMsg = [key, detail];
  const m = MSG[key];
  const base = m ? m[document.body.classList.contains("is-en") ? 1 : 0] : key;
  situation_alert.textContent = detail ? base + ": " + detail : base;
}

// 言語を切り替えたら表示中の文言も差し替える
(function () {
  const cb = document.getElementById("langToggle");
  if (cb) cb.addEventListener("change", () => { if (currentMsg) setStatus(currentMsg[0], currentMsg[1]); });
})();

// 未接続のまま操作されたときに黙って赤くなるのを防ぐ
function requireConnection() {
  if (hidDevice) return true;
  setDot("error");
  setStatus("needConnect");
  return false;
}

setStatus("idle")

const status_dot = document.getElementById("status-dot");
function setDot(state) {
  status_dot.className = "status-dot" + (state ? " is-" + state : "");
}

//--------------- WebHIDでの接続（PC/Switch両モード共通）---------------
// PCモードの接続器(VID 0xCAFE)は設定用のベンダー定義HIDインターフェース(Usage Page 0xFF00)、
// Switchモード(HID 0x0F0D)はHORI互換レポート記述子に元からある8バイトOUTPUTを使う。
// どちらも同じバイト列プロトコルで、ファーム側の実装は src/switch_cfg.h
// （コマンド番号はそこと合わせること）
const CFG_MAGIC = 0x55;
const CFG_SYNC  = 0xA5;
const CFG_N     = 16;             // kandoの個数
const CFG_N_OLD = 15;             // kando[15](SW1設定)が無い版のファーム
const CFG_VENDOR_BYTE = 7;        // 入力レポート内のvendorバイト位置
const CFG_SYNC_STAT = 0x5A;       // 計測値のバーストの先頭（設定の読み戻しと区別する）
const CFG_SYNC_KEY = 0x6A;        // キーマップのバーストの先頭（設定・計測値と区別する）
const CFG_STAT_N = 10;            // 計測値の個数
const CFG_KEY_N = 4;              // キーマップの個数

function hidWrite(bytes, dev = hidDevice) {
  const buf = new Uint8Array(8);
  buf.set(bytes);
  return dev.sendReport(0, buf);
}

// コマンドを投げて、入力レポートのvendorバイトを sync の次から need バイト並べ直す
function hidBurst(cmd, sync, need, timeoutMs = 3000, minNeed = need) {
  // 応答待ちの間に切断されると hidDevice が null になる。開始時のデバイスを掴んで解除まで通す
  const dev = hidDevice;
  return new Promise((resolve, reject) => {
    const bytes = [];
    let synced = false;

    const done = (fn, arg) => {
      clearTimeout(timer);
      clearTimeout(idle);
      dev.removeEventListener("inputreport", onReport);
      fn(arg);
    };
    const timer = setTimeout(() => done(reject, new Error("hidReadAll: timeout")), timeoutMs);

    // minNeed に届いた後、150ms新しいバイトが来なければそこで打ち切る。
    // kando[15]を持たない旧ファームは need より2バイト短いバーストしか返さない
    let idle = null;
    function onReport(e) {
      const b = e.data.getUint8(CFG_VENDOR_BYTE);
      // 打った直後のレポートが混ざるので、同期バイトが来るまでは読み捨てる
      if (!synced) { if (b === sync) synced = true; return; }
      bytes.push(b);
      if (bytes.length === need) { clearTimeout(idle); done(resolve, bytes); return; }
      if (bytes.length >= minNeed) {
        clearTimeout(idle);
        idle = setTimeout(() => done(resolve, bytes), 150);
      }
    }

    dev.addEventListener("inputreport", onReport);
    hidWrite([CFG_MAGIC, cmd], dev).catch(err => done(reject, err));
  });
}

// 設定の読み戻し: kando(LE16)×N + usb_mode_pref
const hidReadAll = () => hidBurst(0x03, CFG_SYNC, CFG_N * 2 + 1, 3000, CFG_N_OLD * 2 + 1);
// 計測値の読み戻し: LE16×CFG_STAT_N。読むと同時にファーム側の統計はリセットされる
const hidReadStat = () => hidBurst(0x05, CFG_SYNC_STAT, CFG_STAT_N * 2);

document.getElementById("hidConnection").addEventListener("click", async () => {
  if (!navigator.hid) { setDot("error"); setStatus("hidUnsupported"); return; }
  try {
    setDot("connecting");
    setStatus("selecting");
    // PCモードは設定用HID（VID 0xCAFE, Usage Page 0xFF00）、SwitchモードはHORI互換（0x0F0D）。
    // PIDはモードや「USB識別子」で複数あるので絞らない
    const [dev] = await navigator.hid.requestDevice({ filters: [
      { vendorId: 0xCAFE, usagePage: 0xFF00 },
      { vendorId: 0x0F0D },
    ] });
    if (!dev) { setDot("error"); setStatus("hidNotFound"); return; }
    if (!dev.opened) await dev.open();

    hidDevice = dev;
    setDot("connected");
    setStatus("hidConnected");
  } catch (error) {
    setDot("error");
    setStatus("connectFail", error.message);
    console.error("Error in hid connect:", error);
  }
});

// 参照を残すと requireConnection() が「接続済み」と誤判定する。
// デバイスが自分から居なくなる場面（ファーム更新モード・起動モードの切替）でも呼ぶ
async function dropConnection() {
  try { if (hidDevice) await hidDevice.close(); } catch (e) {}
  hidDevice = null;
  setDot(null);
}

// USBを抜かれた場合もここを通す。参照が残ると requireConnection() が「接続済み」と誤判定する
navigator.hid?.addEventListener("disconnect", event => {
  if (event.device !== hidDevice) return;
  hidDevice = null;
  setDot(null);
  setStatus("disconnected");
});

BUTTON_CLICK_EVENT_DISC.addEventListener("click", async () => {
  await dropConnection();
  setStatus("disconnected")
});

BUTTON_CLICK_EVENT_send.addEventListener("click", async () => {
  if (isBusy) return;
  if (!requireConnection()) return;
  isBusy = true;
  try {
    setDot("loading");
    setStatus("sending")
    for (let i = 0; i < CFG_N; i++) {
      const v = +kando[i] | 0;
      await hidWrite([CFG_MAGIC, 0x01, i, v & 0xFF, (v >> 8) & 0xFF]);
    }
    await hidWrite([CFG_MAGIC, 0x04, modeValue()]);
    await hidWrite([CFG_MAGIC, 0x02]);   // SAVE
    // 今のモードはVIDで分かる（0x0F0D＝Switch / 0xCAFE＝PC）。
    // 保存した起動モードと違えば保存直後に列挙し直されるので、一旦接続を切る
    if (modeValue() !== currentUsbMode()) { await dropConnection(); setStatus("modeSwitched"); return; }
    setDot("connected");
    setStatus("sent");
  } catch (error) {
    setDot("error");
    setStatus("sendFail", error.message);
    console.error("Error in send:", error);
  } finally {
    isBusy = false;
  }
});

// 受信した1つの設定値をUIへ反映する
function applyKando(idx, val) {
  if (!(idx >= 0 && idx < CFG_N)) return;
  kando[idx] = val;
  const textElement = document.getElementById(`text${idx}`);
  const volumeSlider = document.getElementById(`volumeSlider${idx}`);
  if (volumeSlider) volumeSlider.value = val;
  if (textElement) textElement.textContent = val;
  const spread = document.getElementById('spreadMode');
  const pid = document.getElementById('switchPid');
  const layout = document.getElementById('keyLayout');
  const sw1 = document.getElementById('sw1Use');
  if (idx === 11 && spread) spread.value = val;
  if (idx === 15 && sw1) sw1.value = val;
  if (idx === 12 && pid) pid.value = val;
  if (idx === 14 && layout) layout.value = val;
}

BUTTON_CLICK_EVENT_request.addEventListener("click", async () => {
  if (isBusy) return;
  if (!requireConnection()) return;
  isBusy = true;
  try {
    setDot("loading");
    setStatus("receiving")
    const b = await hidReadAll();
    const n = b.length >= CFG_N * 2 + 1 ? CFG_N : CFG_N_OLD;   // 旧ファームは2バイト短い
    for (let i = 0; i < n; i++) applyKando(i, b[i * 2] | (b[i * 2 + 1] << 8));
    if (n === CFG_N_OLD) applyKando(15, 1);   // SW1設定を持たない版はSW1ありの挙動
    setModeValue(b[n * 2]);
    const adv = document.getElementById('advancedToggle');
    if (!adv || !adv.checked) syncBeginnerDelay();
    setDot("connected");
    setStatus("received")
  } catch (error) {
    setDot("error");
    setStatus("receiveFail", error.message);
    console.error("Error in request:", error);
  } finally {
    isBusy = false;
  }
});

// 11,12 はスライダーではなくセレクト（入力分散 / USB識別子）なので飛ばす
const SLIDER_INDEXES = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 13];

for (const i of SLIDER_INDEXES) {
  const textElement = document.getElementById(`text${i}`);
  const volumeSlider = document.getElementById(`volumeSlider${i}`);
  if (!volumeSlider) continue;

  volumeSlider.addEventListener("input", () => {
    textElement.textContent = volumeSlider.value;
    kando[i] = +volumeSlider.value;
  });

  document.addEventListener("DOMContentLoaded", () => {
    textElement.textContent = volumeSlider.value;
    kando[i] = +volumeSlider.value;
  });
}

// キー配置セレクト初期化
document.addEventListener("DOMContentLoaded", () => {
  for (let i = 0; i < 4; i++) {
    const sel = document.getElementById(`keymap${i}`);
    KEY_OPTIONS.forEach(opt => {
      const o = document.createElement("option");
      o.value = opt.code;
      o.textContent = opt.label;
      sel.appendChild(o);
    });
    sel.value = key_map[i];
    sel.addEventListener("change", () => { key_map[i] = parseInt(sel.value); });
  }
});

// Switch設定セレクト初期化
document.addEventListener("DOMContentLoaded", () => {
  const spread = document.getElementById('spreadMode');
  const pid = document.getElementById('switchPid');
  if (spread) {
    spread.value = kando[11];
    spread.addEventListener("change", () => { kando[11] = parseInt(spread.value); });
  }
  if (pid) {
    pid.value = kando[12];
    pid.addEventListener("change", () => { kando[12] = parseInt(pid.value); });
  }
  const layout = document.getElementById('keyLayout');
  if (layout) {
    layout.value = kando[14];
    layout.addEventListener("change", () => { kando[14] = parseInt(layout.value); });
  }
  const sw1 = document.getElementById('sw1Use');
  if (sw1) {
    sw1.value = kando[15];
    sw1.addEventListener("change", () => { kando[15] = parseInt(sw1.value); });
  }
});

function applySensGradient(slider) {
  const min = +slider.min, max = +slider.max;
  const [rs, re] = slider.classList.contains('rim-sens') ? [60, 100] : [100, 150];
  const span = max - min;
  const s = ((rs - min) / span * 100).toFixed(2);
  const e = ((re - min) / span * 100).toFixed(2);
  slider.style.background =
    `linear-gradient(to right,#d8d8d8 ${s}%,rgba(255,170,0,0.75) ${s}%,rgba(255,170,0,0.75) ${e}%,#d8d8d8 ${e}%)`;
}

function setBeginnerSliders() {
  document.querySelectorAll('.rim-sens').forEach(s => {
    const idx = parseInt(s.id.replace('volumeSlider', ''));
    s.min = 40; s.max = 150; s.step = 5;
    s.value = Math.min(150, Math.max(40, Math.round(+s.value / 5) * 5));
    kando[idx] = +s.value;
    applySensGradient(s);
    const t = document.getElementById('text' + idx);
    if (t) t.textContent = s.value;
  });
  document.querySelectorAll('.don-sens').forEach(s => {
    const idx = parseInt(s.id.replace('volumeSlider', ''));
    s.min = 60; s.max = 170; s.step = 5;
    s.value = Math.min(170, Math.max(60, Math.round(+s.value / 5) * 5));
    kando[idx] = +s.value;
    applySensGradient(s);
    const t = document.getElementById('text' + idx);
    if (t) t.textContent = s.value;
  });
}

function setAdvancedSliders() {
  document.querySelectorAll('.rim-sens, .don-sens').forEach(s => {
    s.min = 0; s.max = 500; s.step = 1;
    s.style.background = '';
  });
}

function syncBeginnerDelay() {
  const rimSlider = document.getElementById('rimDelaySlider');
  const donSlider = document.getElementById('donDelaySlider');
  const crossSlider = document.getElementById('crossDelaySlider');
  const simToggle = document.getElementById('simulatorToggle');
  // つまみはstep幅に丸められるので、数値表示にはkandoの実値を出す。丸めた値を表示すると
  // 「15と表示して30を送る」ことになり、見えている値と送信値が食い違う
  if (rimSlider) {
    rimSlider.value = Math.max(+rimSlider.min, Math.min(+rimSlider.max, kando[5]));
    document.getElementById('rimDelayText').textContent = kando[5];
    kando[9] = Math.min(25, +kando[5] + 5);
    const s9 = document.getElementById('volumeSlider9');
    if (s9) { s9.value = kando[9]; document.getElementById('text9').textContent = kando[9]; }
  }
  if (donSlider) {
    donSlider.value = Math.max(+donSlider.min, Math.min(+donSlider.max, kando[4]));
    document.getElementById('donDelayText').textContent = kando[4];
  }
  if (crossSlider) {
    crossSlider.value = Math.max(+crossSlider.min, Math.min(+crossSlider.max, kando[6]));
    document.getElementById('crossDelayText').textContent = kando[6];
  }
  kando[8] = 0;
  const s8 = document.getElementById('volumeSlider8');
  if (s8) { s8.value = 0; document.getElementById('text8').textContent = 0; }
  if (simToggle) simToggle.checked = kando[7] > 0;
}

function setDelayMode(isAdvanced) {
  const beg = document.getElementById('beginnerDelaySection');
  const adv = document.getElementById('advancedDelaySection');
  if (beg) beg.style.display = isAdvanced ? 'none' : '';
  if (adv) adv.style.display = isAdvanced ? '' : 'none';
}

function setExplainMode(isAdvanced) {
  document.querySelectorAll('.beginner-explain').forEach(el =>
    el.style.display = isAdvanced ? 'none' : 'block');
  document.querySelectorAll('.advanced-explain').forEach(el =>
    el.style.display = isAdvanced ? 'block' : 'none');
}

(function () {
  const rimSlider = document.getElementById('rimDelaySlider');
  const donSlider = document.getElementById('donDelaySlider');
  const crossSlider = document.getElementById('crossDelaySlider');
  const simToggle = document.getElementById('simulatorToggle');

  document.addEventListener('DOMContentLoaded', () => {
    syncBeginnerDelay();
  });

  rimSlider.addEventListener('input', function () {
    const v = +this.value;
    document.getElementById('rimDelayText').textContent = v;
    kando[5] = v;
    kando[9] = Math.min(25, v + 5);
    const s5 = document.getElementById('volumeSlider5');
    const s9 = document.getElementById('volumeSlider9');
    if (s5) { s5.value = v; document.getElementById('text5').textContent = v; }
    if (s9) { s9.value = kando[9]; document.getElementById('text9').textContent = kando[9]; }
  });

  donSlider.addEventListener('input', function () {
    const v = +this.value;
    document.getElementById('donDelayText').textContent = v;
    kando[4] = v;
    const s4 = document.getElementById('volumeSlider4');
    if (s4) { s4.value = v; document.getElementById('text4').textContent = v; }
  });

  crossSlider.addEventListener('input', function () {
    const v = +this.value;
    document.getElementById('crossDelayText').textContent = v;
    kando[6] = v;
    const s6 = document.getElementById('volumeSlider6');
    if (s6) { s6.value = v; document.getElementById('text6').textContent = v; }
  });

  simToggle.addEventListener('change', function () {
    const v = this.checked ? 20 : 0;
    kando[7] = v;
    const s7 = document.getElementById('volumeSlider7');
    if (s7) { s7.value = v; document.getElementById('text7').textContent = v; }
  });
})();

(function () {
  const toggle = document.getElementById('advancedToggle');
  if (!toggle) return;
  const saved = localStorage.getItem('hidtaiko-advanced') === '1';
  toggle.checked = saved;
  document.querySelectorAll('.advanced-card').forEach(el =>
    el.classList.toggle('is-visible', saved));
  setDelayMode(saved);
  setExplainMode(saved);
  if (saved) setAdvancedSliders();
  else setBeginnerSliders();

  toggle.addEventListener('change', () => {
    const on = toggle.checked;
    document.querySelectorAll('.advanced-card').forEach(el =>
      el.classList.toggle('is-visible', on));
    setDelayMode(on);
    setExplainMode(on);
    if (on) setAdvancedSliders();
    else { setBeginnerSliders(); syncBeginnerDelay(); }
    localStorage.setItem('hidtaiko-advanced', on ? '1' : '0');
  });
})();

// 計測値はHIDのバーストで読む（cmd 0x05）。PCモードでも打→送信が同じ指標で測れる
async function readStatAny() {
  const b = await hidReadStat();
  const u16 = (i) => b[i * 2] | (b[i * 2 + 1] << 8);
  return {
    min: u16(0), avg: u16(1), max: u16(2),
    latmax: u16(3), latavg: u16(4), latn: u16(5),
    h0: u16(6), h1: u16(7), h2: u16(8), h3: u16(9),
    // デルタとLEDの実測はHIDのバーストに載せていない（20µs周期の話には要らない）
  };
}

function showStat(s) {
  const set = (id, v) => {
    const el = document.getElementById(id);
    if (el) el.textContent = (v === undefined ? "–" : v);
  };
  for (let i = 0; i < 4; i++) {
    const delta = parseInt(s["d" + i]);
    const thr = +kando[i];
    set(`statD${i}`, s["d" + i]);
    set(`statK${i}`, thr);
    set(`statH${i}`, s["h" + i]);
    // デルタが感度に迫っていれば誤検出の一歩手前。色で気付けるようにする。
    // ただし**そのパッドを叩いていないときだけ**判断できる。叩けばデルタは必ず感度を
    // 超える（超えたから検出されている）ので、打数>0で色を付けても全部赤くなるだけ
    const cell = document.getElementById(`statD${i}`);
    if (cell) {
      cell.classList.remove("is-near", "is-over");
      const hits = parseInt(s["h" + i]);
      if (Number.isFinite(delta) && thr > 0 && hits === 0) {
        if (delta > thr) cell.classList.add("is-over");
        else if (delta > thr * 0.6) cell.classList.add("is-near");
      }
    }
  }
  set("statMin", s.min);
  set("statAvg", s.avg);
  set("statMax", s.max);
  set("statCnt", s.cnt);

  // LED書き込みの実測。1回1µs未満なのでµsのままだと0か1しか出ない。
  // 合計÷回数をnsで出す（切り捨ての位相がばらつくので平均は意味を持つ）
  const n = parseInt(s.ledn), sum = parseInt(s.ledsum);
  set("statLedAvg", (Number.isFinite(n) && n > 0) ? Math.round(sum * 1000 / n) : undefined);
  set("statLedMax", s.ledmax);
  set("statLedN", s.ledn);

  // 打を検出してからレポートがUSBへ渡るまで。PC/Switchで同じ指標
  set("statLatAvg", s.latavg);
  set("statLatMax", s.latmax);
  set("statLatN", s.latn);
}

BUTTON_CLICK_EVENT_stat_reset.addEventListener("click", async () => {
  if (isBusy) return;
  if (!requireConnection()) return;
  isBusy = true;
  try {
    setDot("loading");
    await readStatAny();   // 読むとファーム側の統計がリセットされる＝これが「計測開始」
    showStat({});
    setDot("connected");
    setStatus("statStart");
  } catch (error) {
    setDot("error");
    setStatus("statStartFail", error.message);
    console.error("Error in stat reset:", error);
  } finally {
    isBusy = false;
  }
});

BUTTON_CLICK_EVENT_stat_read.addEventListener("click", async () => {
  if (isBusy) return;
  if (!requireConnection()) return;
  isBusy = true;
  try {
    setDot("loading");
    showStat(await readStatAny());
    setDot("connected");
    setStatus("statRead");
  } catch (error) {
    setDot("error");
    setStatus("statReadFail", error.message);
    console.error("Error in stat read:", error);
  } finally {
    isBusy = false;
  }
});

// 計測・Switch機能・キー配置の開閉見出し。buttonなのでEnter/Spaceもこのclickへ来る
for (const [toggleId, contentId] of [["statToggle", "statContent"], ["switchToggle", "switchContent"], ["keymapToggle", "keymapContent"]]) {
  const title = document.getElementById(toggleId);
  const content = document.getElementById(contentId);
  if (!title || !content) continue;
  title.setAttribute("aria-controls", contentId);
  title.setAttribute("aria-expanded", String(content.style.display !== "none"));
  title.addEventListener("click", () => {
    const open = content.style.display === "none";
    content.style.display = open ? "" : "none";
    title.classList.toggle("is-collapsed", !open);
    title.setAttribute("aria-expanded", String(open));
  });
}

BUTTON_CLICK_EVENT_keymap_send.addEventListener("click", async () => {
  if (isBusy) return;
  if (!requireConnection()) return;
  isBusy = true;
  try {
    setDot("loading");
    setStatus("keySending")
    // SETKEY×4 → SAVE（ファームの0x06/0x02。PC/Switchで同じプロトコル）
    for (let i = 0; i < CFG_KEY_N; i++) {
      await hidWrite([CFG_MAGIC, 0x06, i, key_map[i]]);
    }
    await hidWrite([CFG_MAGIC, 0x02]);   // SAVE
    setDot("connected");
    setStatus("keySent")
  } catch (error) {
    setDot("error");
    setStatus("keySendFail", error.message);
    console.error("Error in keymap send:", error);
  } finally {
    isBusy = false;
  }
});

BUTTON_CLICK_EVENT_keymap_request.addEventListener("click", async () => {
  if (isBusy) return;
  if (!requireConnection()) return;
  isBusy = true;
  try {
    setDot("loading");
    setStatus("keyReceiving")
    // GETKEY(0x07) → CFG_SYNC_KEY + 4バイトのバースト
    const b = await hidBurst(0x07, CFG_SYNC_KEY, CFG_KEY_N);
    for (let i = 0; i < CFG_KEY_N; i++) {
      const code = b[i];
      // ファームは4..0xFFしか書かない。範囲外は空きフラッシュ由来のゴミなので拾わない
      if (code >= 4 && code <= 0xFF) {
        key_map[i] = code;
        const sel = document.getElementById(`keymap${i}`);
        if (sel) sel.value = code;
      }
    }
    setDot("connected");
    setStatus("keyReceived")
  } catch (error) {
    setDot("error");
    setStatus("keyReceiveFail", error.message);
    console.error("Error in keymap request:", error);
  } finally {
    isBusy = false;
  }
});

// 押してから初めて非対応と分かるのでは遅い。ページを開いた時点で伝えてボタンを潰す
if (!navigator.hid) {
  setDot("error");
  setStatus("hidUnsupported");
  const connect = document.getElementById("hidConnection");
  if (connect) connect.disabled = true;
}

// スライダーとプルダウンには <label> が無く、行見出しは読み上げ名にならない。行から名前を組み立てて付ける
function labelControls() {
  const other = document.body.classList.contains("is-en") ? "ja" : "en";
  // 日英の両方がDOMに居るので、表示していない側を落としてから文字を取る
  const textOf = el => {
    if (!el) return "";
    const copy = el.cloneNode(true);
    for (const hidden of copy.querySelectorAll(".i18n-" + other)) hidden.remove();
    return (copy.textContent || "").replace(/\s+/g, " ").trim();
  };
  for (const row of document.querySelectorAll(".slider-row, .keymap-row")) {
    const name = textOf(row.querySelector(".pad-name") || row.querySelector(".label"));
    const card = row.closest(".card");
    const title = card ? textOf(card.querySelector(".card-title")) : "";
    for (const control of row.querySelectorAll("input[type=range], select")) {
      control.setAttribute("aria-label", (title + " " + name).trim() || control.id);
    }
  }
}
labelControls();
// 言語を切り替えたら読み上げ名も切り替える
document.getElementById("langToggle")?.addEventListener("change", labelControls);
