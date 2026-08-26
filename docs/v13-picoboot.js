// RP2040へのUF2書き込み。BOOTSELのPICOBOOT(WebUSB)を第一手にし、
// WinUSBが当たっていない環境ではBOOTSELドライブ(File System Access)へ落とす。
// v135.js と index.html の両方から使うため、UIには触らず進捗はコールバックで返す。
(function (global) {
"use strict";

const PICOBOOT_VID = 0x2E8A;
const PICOBOOT_PID_RP2040 = 0x0003;
const RP2040_FAMILY_ID = 0xE48BFF56;
const PICOBOOT_MAGIC = 0x431FD10B;
const PICOBOOT_CMD_EXCLUSIVE = 0x01;
const PICOBOOT_CMD_REBOOT = 0x02;
const PICOBOOT_CMD_ERASE = 0x03;
const PICOBOOT_CMD_WRITE = 0x05;
const PICOBOOT_CMD_EXIT_XIP = 0x06;
const PICOBOOT_CMD_READ = 0x84;
const FLASH_BASE = 0x10000000;
const FLASH_END = 0x10200000;
const FLASH_SECTOR_SIZE = 4096;
const FLASH_PAGE_SIZE = 256;
const UF2_MAGIC = 0x0A324655;
const UF2_MAGIC2 = 0x9E5D5157;
const UF2_END_MAGIC = 0x0AB16F30;
const UF2_FLAG_NOFLASH = 0x00000001;
const UF2_FLAG_FAMILY_ID = 0x00002000;

function withTimeout(promise, timeoutMs, label) {
  let timer;
  const timeout = new Promise((_, reject) => {
    timer = setTimeout(() => reject(new Error(`${label} timed out`)), timeoutMs);
  });
  return Promise.race([promise, timeout]).finally(() => clearTimeout(timer));
}

function mergeFlashRanges(ranges) {
  const sorted = [...ranges].sort((a, b) => a.start - b.start);
  const merged = [];
  for (const range of sorted) {
    const last = merged[merged.length - 1];
    if (last && range.start <= last.end) last.end = Math.max(last.end, range.end);
    else merged.push({ start: range.start, end: range.end });
  }
  return merged;
}

async function parseUf2(buffer) {
  if (buffer.byteLength === 0 || buffer.byteLength % 512 !== 0) throw new Error("invalid UF2 size");
  const view = new DataView(buffer);
  const blocks = [];
  const ranges = [];
  let expectedBlocks = null;
  for (let offset = 0; offset < buffer.byteLength; offset += 512) {
    if (view.getUint32(offset, true) !== UF2_MAGIC ||
        view.getUint32(offset + 4, true) !== UF2_MAGIC2 ||
        view.getUint32(offset + 508, true) !== UF2_END_MAGIC) {
      throw new Error("invalid UF2 magic");
    }
    const flags = view.getUint32(offset + 8, true);
    const target = view.getUint32(offset + 12, true);
    const size = view.getUint32(offset + 16, true);
    const blockNo = view.getUint32(offset + 20, true);
    const blockCount = view.getUint32(offset + 24, true);
    if (expectedBlocks === null) expectedBlocks = blockCount;
    if (blockCount !== expectedBlocks || blockNo >= blockCount) throw new Error("invalid UF2 block numbering");
    if (flags & UF2_FLAG_NOFLASH) continue;
    if (flags & UF2_FLAG_FAMILY_ID && view.getUint32(offset + 28, true) !== RP2040_FAMILY_ID) {
      throw new Error("UF2 is not for RP2040");
    }
    if (target < FLASH_BASE || target >= FLASH_END || size === 0 || size > 476 ||
        target + size > FLASH_END || target % FLASH_PAGE_SIZE !== 0 || size % FLASH_PAGE_SIZE !== 0) {
      throw new Error("UF2 flash range is invalid");
    }
    blocks.push({ address: target, data: new Uint8Array(buffer.slice(offset + 32, offset + 32 + size)) });
    const start = target & ~(FLASH_SECTOR_SIZE - 1);
    const end = (target + size + FLASH_SECTOR_SIZE - 1) & ~(FLASH_SECTOR_SIZE - 1);
    ranges.push({ start, end });
  }
  if (!blocks.length) throw new Error("UF2 contains no flash blocks");
  blocks.sort((a, b) => a.address - b.address);
  return { blocks, ranges: mergeFlashRanges(ranges) };
}

function findPicobootInterface(configuration) {
  for (const iface of configuration.interfaces) {
    for (const alternate of iface.alternates) {
      if (alternate.interfaceClass !== 0xFF) continue;
      const out = alternate.endpoints.find(ep => ep.direction === "out" && ep.type === "bulk");
      const input = alternate.endpoints.find(ep => ep.direction === "in" && ep.type === "bulk");
      if (out && input) return { number: iface.interfaceNumber, out: out.endpointNumber, in: input.endpointNumber };
    }
  }
  throw new Error("PICOBOOT interface not found");
}

class PicobootClient {
  constructor(device, iface) {
    this.device = device;
    this.iface = iface;
    this.token = 1;
  }

  // WinUSBが当たっていないデバイスでは open() が返ってこないことがあるので、
  // 掴むところまでを丸ごと打ち切ってドライブ経路へ渡す
  async open() {
    await withTimeout((async () => {
      if (!this.device.opened) await this.device.open();
      if (!this.device.configuration) await this.device.selectConfiguration(1);
      this.iface = findPicobootInterface(this.device.configuration);
      await this.device.claimInterface(this.iface.number);
    })(), 3000, "PICOBOOT open");
  }

  async close() {
    try { await this.device.releaseInterface(this.iface.number); } catch (_) {}
    try { await this.device.close(); } catch (_) {}
  }

  async resetInterface() {
    await withTimeout(this.device.controlTransferOut({
      requestType: "vendor", recipient: "interface", request: 0x41,
      value: 0, index: this.iface.number,
    }), 3000, "PICOBOOT reset");
  }

  async command(commandId, commandData = null, transferOut = null, transferInLength = 0, timeoutMs = 10000) {
    const packet = new ArrayBuffer(32);
    const packetView = new DataView(packet);
    packetView.setUint32(0, PICOBOOT_MAGIC, true);
    const token = this.token++;
    packetView.setUint32(4, token, true);
    packetView.setUint8(8, commandId);
    packetView.setUint8(9, commandData ? commandData.byteLength : 0);
    packetView.setUint16(10, 0, true);
    packetView.setUint32(12, transferOut ? transferOut.byteLength : transferInLength, true);
    if (commandData) new Uint8Array(packet, 16, commandData.byteLength).set(new Uint8Array(commandData));
    await withTimeout(this.device.transferOut(this.iface.out, packet), timeoutMs, "PICOBOOT command");

    let input = null;
    if (transferOut) await withTimeout(this.device.transferOut(this.iface.out, transferOut), timeoutMs, "PICOBOOT data");
    if (transferInLength) input = await withTimeout(this.device.transferIn(this.iface.in, transferInLength), timeoutMs, "PICOBOOT read");
    if (commandId & 0x80) await withTimeout(this.device.transferOut(this.iface.out, new Uint8Array(0)), timeoutMs, "PICOBOOT ack");
    else await withTimeout(this.device.transferIn(this.iface.in, 64), timeoutMs, "PICOBOOT status");
    if (commandId !== PICOBOOT_CMD_REBOOT) {
      const status = await withTimeout(this.device.controlTransferIn({
        requestType: "vendor", recipient: "interface", request: 0x42,
        value: 0, index: this.iface.number,
      }, 16), timeoutMs, "PICOBOOT command status");
      if (!status.data || status.data.byteLength < 10) throw new Error("invalid PICOBOOT status");
      const statusView = new DataView(status.data.buffer, status.data.byteOffset, status.data.byteLength);
      const statusToken = statusView.getUint32(0, true);
      const statusCode = statusView.getUint32(4, true);
      const statusCommand = statusView.getUint8(8);
      const inProgress = statusView.getUint8(9);
      if (statusToken !== token || statusCommand !== commandId || inProgress || statusCode !== 0) {
        throw new Error(`PICOBOOT command failed (status ${statusCode})`);
      }
    }
    return input ? new Uint8Array(input.data.buffer, input.data.byteOffset, input.data.byteLength) : null;
  }

  async exclusive() { await this.command(PICOBOOT_CMD_EXCLUSIVE, new Uint8Array([1])); }
  async exitXip() { await this.command(PICOBOOT_CMD_EXIT_XIP); }

  async erase(address, size) {
    const data = new ArrayBuffer(8);
    const view = new DataView(data);
    view.setUint32(0, address, true);
    view.setUint32(4, size, true);
    await this.command(PICOBOOT_CMD_ERASE, data);
  }

  async write(address, bytes) {
    const data = new ArrayBuffer(8);
    const view = new DataView(data);
    view.setUint32(0, address, true);
    view.setUint32(4, bytes.byteLength, true);
    await this.command(PICOBOOT_CMD_WRITE, data, bytes);
  }

  async read(address, size) {
    const data = new ArrayBuffer(8);
    const view = new DataView(data);
    view.setUint32(0, address, true);
    view.setUint32(4, size, true);
    return this.command(PICOBOOT_CMD_READ, data, null, size);
  }

  async reboot() {
    const data = new ArrayBuffer(12);
    const view = new DataView(data);
    view.setUint32(0, 0, true);
    view.setUint32(4, 0x20042000, true);
    view.setUint32(8, 500, true);
    try { await this.command(PICOBOOT_CMD_REBOOT, data, null, 0, 3000); } catch (_) {}
  }
}

function sameBytes(a, b) {
  if (!a || a.length !== b.length) return false;
  for (let i = 0; i < a.length; i++) if (a[i] !== b[i]) return false;
  return true;
}

// 許可済みのものだけを返す。requestDevice は出さない:
// WinUSBが当たっていない環境では候補が0件のダイアログが出るだけで、閉じる手間が増える
async function findAllowedPicoboot() {
  if (!navigator.usb) return null;
  const allowed = await navigator.usb.getDevices();
  return allowed.find(dev => dev.vendorId === PICOBOOT_VID && dev.productId === PICOBOOT_PID_RP2040) || null;
}

// WebUSBを使いたい人が明示的に呼ぶ。ここでだけ選択ダイアログを出す
async function requestPicobootAccess() {
  if (!navigator.usb) throw new Error("WebUSB is not supported");
  return navigator.usb.requestDevice({ filters: [{ vendorId: PICOBOOT_VID, productId: PICOBOOT_PID_RP2040 }] });
}

// WebUSB経路。消去・書き込み・ベリファイまでやる。WinUSBが当たっていないと open() で落ちる
async function flashOverUsb(uf2, report, device) {
  const client = new PicobootClient(device);
  try {
    await client.open();
    await client.resetInterface();
    await client.exclusive();
    await client.exitXip();

    let erased = 0;
    const totalSectors = uf2.ranges.reduce((sum, r) => sum + (r.end - r.start) / FLASH_SECTOR_SIZE, 0);
    for (const range of uf2.ranges) {
      for (let address = range.start; address < range.end; address += FLASH_SECTOR_SIZE) {
        await client.erase(address, FLASH_SECTOR_SIZE);
        erased++;
        report(`消去中... ${erased}/${totalSectors}`, `Erasing... ${erased}/${totalSectors}`);
      }
    }
    for (let i = 0; i < uf2.blocks.length; i++) {
      await client.write(uf2.blocks[i].address, uf2.blocks[i].data);
      report(`書き込み中... ${i + 1}/${uf2.blocks.length}`, `Writing... ${i + 1}/${uf2.blocks.length}`);
    }
    for (let i = 0; i < uf2.blocks.length; i++) {
      const block = uf2.blocks[i];
      const actual = await client.read(block.address, block.data.byteLength);
      if (!sameBytes(actual, block.data)) throw new Error(`verify failed at 0x${block.address.toString(16)}`);
      report(`検証中... ${i + 1}/${uf2.blocks.length}`, `Verifying... ${i + 1}/${uf2.blocks.length}`);
    }
    await client.reboot();
  } finally {
    await client.close();
  }
}

// 一度選んでもらったUF2ドライブを覚えておく。ハンドルは構造化クローンでしか運べないので
// localStorageではなくIndexedDBに置く
const DIR_DB = "hidtaiko-flash";
const DIR_KEY = "rpi-rp2-dir";

function idbStore(mode) {
  return new Promise((resolve, reject) => {
    const open = indexedDB.open(DIR_DB, 1);
    open.onupgradeneeded = () => open.result.createObjectStore("handles");
    open.onerror = () => reject(open.error);
    open.onsuccess = () => resolve(open.result.transaction("handles", mode).objectStore("handles"));
  });
}

function idbRequest(request) {
  return new Promise((resolve, reject) => {
    request.onsuccess = () => resolve(request.result);
    request.onerror = () => reject(request.error);
  });
}

async function rememberDrive(dir) {
  try { await idbRequest((await idbStore("readwrite")).put(dir, DIR_KEY)); }
  catch (e) { console.warn("could not remember the UF2 drive:", e); }
}

// 覚えたハンドルがまだ使えるかを確かめる。BOOTSELドライブは入るたびに
// マウントし直されるので、権限が残っていても実体が死んでいることがある
async function recallDrive() {
  let dir;
  try { dir = await idbRequest((await idbStore("readonly")).get(DIR_KEY)); }
  catch (_) { return null; }
  if (!dir) return null;
  try {
    if (await dir.queryPermission({ mode: "readwrite" }) !== "granted") return null;
    await dir.getFileHandle("INFO_UF2.TXT");
    return dir;
  } catch (_) {
    return null;
  }
}

// ドライブ経路。ブートROMが書き込み完了と同時に再起動するので、
// close()やwrite()が中断として例外を投げることがある。それは失敗ではない
async function flashOverDrive(bytes, report) {
  if (!global.showDirectoryPicker) throw new Error("File System Access is not supported");
  let dir = await recallDrive();
  if (dir) {
    report("覚えているUF2ドライブに書き込みます...", "Writing to the remembered UF2 drive...");
  } else {
    report("UF2ドライブ（RPI-RP2）を選んでください...", "Pick the UF2 drive (RPI-RP2)...");
    dir = await global.showDirectoryPicker({ id: "rpi-rp2", mode: "readwrite" });
    const marker = await dir.getFileHandle("INFO_UF2.TXT").catch(() => null);
    if (!marker) throw new Error("選んだ場所はUF2ドライブではありません / not a UF2 drive");
    await rememberDrive(dir);
    report("書き込み中...", "Writing...");
  }
  const handle = await dir.getFileHandle("firmware.uf2", { create: true });
  const writable = await handle.createWritable();
  try {
    await writable.write(bytes);
    await writable.close();
  } catch (e) {
    try { await writable.close(); } catch (_) {}
  }
}

async function fetchUf2(url) {
  const response = await fetch(url, { cache: "no-store" });
  if (!response.ok) throw new Error(`UF2 download failed (${response.status})`);
  return response.arrayBuffer();
}

// WinUSBが当たっていればWebUSB、駄目ならドライブへ自動で切り替える。
// requestDevice のユーザーキャンセルだけは意思表示なので、ドライブへ落とさずそのまま投げる
// showDirectoryPicker はクリック直後にしか開けない。DFU移行のように時間のかかる処理を
// 挟むとジェスチャーが切れるので、呼び出し側が段を分けて渡してくる前提で書く
async function flashUf2(url, report) {
  const buffer = await fetchUf2(url);
  const uf2 = await parseUf2(buffer);
  const device = await findAllowedPicoboot();
  if (device) {
    try {
      await flashOverUsb(uf2, report, device);
      return "usb";
    } catch (e) {
      console.warn("PICOBOOT flash failed, falling back to the UF2 drive:", e);
      report("WebUSBで書けなかったのでUF2ドライブへ切り替えます...", "WebUSB failed, falling back to the UF2 drive...");
    }
  }
  try {
    await flashOverDrive(buffer, report);
  } catch (e) {
    // ジェスチャー切れだけは操作の問題なので、もう一度押せば済むと伝える
    if (/user gesture/i.test(e.message)) {
      throw new Error("もう一度ボタンを押してください / click the button once more");
    }
    throw e;
  }
  return "drive";
}

// ---- HIDtaikoをDFUへ落とす ----
// 設定チャネル(vendor HID)の 0x08。V1.3より前のファームには無いので、
// 応答が無ければ物理BOOTSELに頼るしかない
const CFG_MAGIC = 0x55;
const CFG_CMD_BOOT = 0x08;
const HIDTAIKO_VIDS = [0xCAFE, 0x0F0D];

function hidSend(dev, bytes) {
  const buf = new Uint8Array(8);
  buf.set(bytes);
  return dev.sendReport(0, buf);
}

// disconnectイベントに間に合わないことがあるので、送信が失敗するまで叩き続ける
function waitHidGone(dev, ms) {
  return new Promise(resolve => {
    let settled = false;
    const done = (v) => {
      if (settled) return;
      settled = true;
      clearTimeout(timer);
      navigator.hid.removeEventListener("disconnect", on);
      resolve(v);
    };
    const on = (e) => { if (!e.device || e.device === dev) done(true); };
    const timer = setTimeout(() => done(false), ms);
    navigator.hid.addEventListener("disconnect", on);
    (async () => {
      while (!settled) {
        try { await hidSend(dev, [0]); }
        catch (_) { done(true); return; }
        await new Promise(r => setTimeout(r, 100));
      }
    })();
  });
}

async function findHidtaiko(promptIfMissing) {
  if (!navigator.hid) return null;
  const known = await navigator.hid.getDevices();
  const found = known.find(d => HIDTAIKO_VIDS.includes(d.vendorId));
  if (found) return found;
  if (!promptIfMissing) return null;
  // 接続器が既にDFUに入っているとHIDには出てこない。選ばずに閉じられたら
  // 「もうDFUだ」とみなして書き込みへ進む
  try {
    const picked = await navigator.hid.requestDevice({
      filters: HIDTAIKO_VIDS.map(vendorId => ({ vendorId })),
    });
    return picked[0] || null;
  } catch (_) {
    return null;
  }
}

// 接続器が見つかればDFUへ落とす。見つからない/古いファームなら false を返し、
// 呼び出し側は「BOOTSELボタンを押しながら挿し直す」案内に回る
async function enterBootsel(report, preferredDevice) {
  // 設定ページで選択済みのデバイスを優先し、同じVIDの他社コントローラを誤って選ばない。
  const dev = preferredDevice || await findHidtaiko(true);
  if (!dev) return false;
  try {
    if (!dev.opened) await dev.open();
    await hidSend(dev, [CFG_MAGIC, CFG_CMD_BOOT]);
  } catch (_) {
    return false;
  }
  report("ファーム更新モードへ切り替えています...", "Entering firmware update mode...");
  const gone = await waitHidGone(dev, 3000);
  try { await dev.close(); } catch (_) {}
  // ブートROMがUSBを出し直すのを待つ
  if (gone) await new Promise(r => setTimeout(r, 1500));
  return gone;
}

global.Picoboot = { flashUf2, parseUf2, enterBootsel, requestPicobootAccess, recallDrive, PICOBOOT_VID, PICOBOOT_PID_RP2040 };
})(window);
