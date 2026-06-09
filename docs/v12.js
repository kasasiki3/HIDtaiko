let port;
let writer;

let kando = [200, 200, 200, 200, 200, 200, 200, 200, 200, 0];

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

const BUTTON_CLICK_EVENT = document.getElementById("connection");
const BUTTON_CLICK_EVENT_keymap_send = document.getElementById("keyMapSendButton");
const BUTTON_CLICK_EVENT_keymap_request = document.getElementById("keyMapRequestButton");
const BUTTON_CLICK_EVENT_DISC= document.getElementById("disconnection");
const BUTTON_CLICK_EVENT_send = document.getElementById("sendButton");
const BUTTON_CLICK_EVENT_request = document.getElementById("requestButton");
const situation_alert = document.getElementById("situation");

let reader;
let lineBuffer = '';
let isBusy = false;

async function readLine() {
  while (true) {
    const idx = lineBuffer.indexOf('\n');
    if (idx !== -1) {
      const line = lineBuffer.slice(0, idx).trim();
      lineBuffer = lineBuffer.slice(idx + 1);
      if (line) return line;
      continue;
    }
    const { value, done } = await reader.read();
    if (done) throw new Error('port closed');
    lineBuffer += new TextDecoder().decode(value);
  }
}

situation_alert.textContent = "未接続"

const status_dot = document.getElementById("status-dot");
function setDot(state) {
  status_dot.className = "status-dot" + (state ? " is-" + state : "");
}

BUTTON_CLICK_EVENT.addEventListener("click", async () => {
  try {
    if (port && port.readable) {
      if (writer) writer.releaseLock();
      if (reader) reader.releaseLock();
      await port.close();
    }

    setDot("connecting");
    situation_alert.textContent = "選択中..."

    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 9600 });
    await port.setSignals({ dataTerminalReady: true });

    writer = port.writable.getWriter();
    reader = port.readable.getReader();
    lineBuffer = '';

    setDot("connected");
    situation_alert.textContent = "V1.2接続済み"

    const data = new TextEncoder().encode("Hello, Serial!\n");
    await writer.write(data);
  } catch (error) {
    setDot("error");
    situation_alert.textContent = "接続失敗"
    console.error("Error:", error);
  }
});

window.addEventListener("beforeunload", async () => {
  if (writer) writer.releaseLock();
  if (reader) reader.releaseLock();
  if (port) await port.close();
});

BUTTON_CLICK_EVENT_DISC.addEventListener("click", async () => {
  if (writer) writer.releaseLock();
  if (reader) reader.releaseLock();
  if (port) await port.close();
  setDot(null);
  situation_alert.textContent = "切断"
});

BUTTON_CLICK_EVENT_send.addEventListener("click", async () => {
  if (isBusy) return;
  isBusy = true;
  try {
    setDot("loading");
    situation_alert.textContent = "送信中..."
    const payload = kando.map((v, i) => `${i}=${v}`).join(":");
    await writer.write(new TextEncoder().encode(`SENS:${payload}\n`));
    const ok = await readLine();
    if (ok !== "OK") throw new Error("send failed: " + ok);
    await writer.write(new TextEncoder().encode("SAVE\n"));
    const saveOk = await readLine();
    if (saveOk !== "OK") throw new Error("save failed: " + saveOk);
    setDot("connected");
    situation_alert.textContent = "送信・保存完了"
  } catch (error) {
    setDot("error");
    console.error("Error in send:", error);
  } finally {
    isBusy = false;
  }
});

BUTTON_CLICK_EVENT_request.addEventListener("click", async () => {
  if (isBusy) return;
  isBusy = true;
  try {
    setDot("loading");
    situation_alert.textContent = "受信中..."
    lineBuffer = '';
    await writer.write(new TextEncoder().encode("GETS\n"));
    while (true) {
      const line = await readLine();
      if (line === "OK") break;
      const parts = line.split(":");
      if (parts.length === 2) {
        const idx = parseInt(parts[0]);
        const val = parseInt(parts[1]);
        if (idx >= 0 && idx <= 9) {
          kando[idx] = val;
          const textElement = document.getElementById(`text${idx}`);
          const volumeSlider = document.getElementById(`volumeSlider${idx}`);
          if (volumeSlider) volumeSlider.value = val;
          if (textElement) textElement.textContent = val;
        }
      }
    }
    const advToggle = document.getElementById('advancedToggle');
    if (!advToggle || !advToggle.checked) syncBeginnerDelay();
    setDot("connected");
    situation_alert.textContent = "受信完了"
  } catch (error) {
    setDot("error");
    console.error("Error in request:", error);
  } finally {
    isBusy = false;
  }
});


for (let i = 0; i <= 9; i++) {
  const textElement = document.getElementById(`text${i}`);
  const volumeSlider = document.getElementById(`volumeSlider${i}`);

  volumeSlider.addEventListener("input", () => {
    textElement.textContent = volumeSlider.value;
    kando[i] = volumeSlider.value;
  });
}

for (let i = 0; i <= 9; i++) {
  const textElement = document.getElementById(`text${i}`);
  const volumeSlider = document.getElementById(`volumeSlider${i}`);

  document.addEventListener("DOMContentLoaded", () => {
    textElement.textContent = volumeSlider.value;
    kando[i] = volumeSlider.value;
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
  if (rimSlider) {
    rimSlider.value = Math.max(+rimSlider.min, Math.min(+rimSlider.max, kando[5]));
    document.getElementById('rimDelayText').textContent = rimSlider.value;
    kando[9] = Math.min(25, +rimSlider.value + 5);
    const s9 = document.getElementById('volumeSlider9');
    if (s9) { s9.value = kando[9]; document.getElementById('text9').textContent = kando[9]; }
  }
  if (donSlider) {
    donSlider.value = Math.max(+donSlider.min, Math.min(+donSlider.max, kando[4]));
    document.getElementById('donDelayText').textContent = donSlider.value;
  }
  if (crossSlider) {
    crossSlider.value = Math.max(+crossSlider.min, Math.min(+crossSlider.max, kando[6]));
    document.getElementById('crossDelayText').textContent = crossSlider.value;
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

document.getElementById("keymapToggle").addEventListener("click", () => {
  const content = document.getElementById("keymapContent");
  const title = document.getElementById("keymapToggle");
  const hidden = content.style.display === "none";
  content.style.display = hidden ? "" : "none";
  title.classList.toggle("is-collapsed", !hidden);
});

BUTTON_CLICK_EVENT_keymap_send.addEventListener("click", async () => {
  if (isBusy) return;
  isBusy = true;
  try {
    setDot("loading");
    situation_alert.textContent = "キー配置送信中..."
    const payload = key_map.map((v, i) => `${i}=${v}`).join(":");
    await writer.write(new TextEncoder().encode(`KEY:${payload}\n`));
    const ok = await readLine();
    if (ok !== "OK") throw new Error("keymap send failed: " + ok);
    await writer.write(new TextEncoder().encode("SAVE\n"));
    const saveOk = await readLine();
    if (saveOk !== "OK") throw new Error("save failed: " + saveOk);
    setDot("connected");
    situation_alert.textContent = "キー配置送信・保存完了"
  } catch (error) {
    setDot("error");
    console.error("Error in keymap send:", error);
  } finally {
    isBusy = false;
  }
});

BUTTON_CLICK_EVENT_keymap_request.addEventListener("click", async () => {
  if (isBusy) return;
  isBusy = true;
  try {
    setDot("loading");
    situation_alert.textContent = "キー配置受信中..."
    lineBuffer = '';
    await writer.write(new TextEncoder().encode("GETK\n"));
    while (true) {
      const line = await readLine();
      if (line === "OK") break;
      const parts = line.split(":");
      if (parts.length === 2) {
        const idx = parseInt(parts[0]);
        const code = parseInt(parts[1]);
        if (idx >= 0 && idx < 4) {
          key_map[idx] = code;
          const sel = document.getElementById(`keymap${idx}`);
          if (sel) sel.value = code;
        }
      }
    }
    setDot("connected");
    situation_alert.textContent = "キー配置受信完了"
  } catch (error) {
    setDot("error");
    console.error("Error in keymap request:", error);
  } finally {
    isBusy = false;
  }
});
