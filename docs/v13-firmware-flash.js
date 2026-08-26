// 各感度変更ページで同じDFU→UF2書き込み操作を提供する。
(function (global) {
  let activeHidDeviceProvider = null;

  function setStatus(target, ja, en) {
    target.replaceChildren();
    const jaText = document.createElement("span");
    jaText.className = "i18n-ja";
    jaText.textContent = ja;
    const enText = document.createElement("span");
    enText.className = "i18n-en";
    enText.textContent = en;
    target.append(jaText, enText);
  }

  // File System Accessのドライブ選択はクリック直後でないと開けないため、
  // DFUへ移した直後に必要になった場合だけ次のクリックへ引き継ぐ。
  function bindFlashButton(button, status, url, bootMode) {
    let readyToFlash = false;
    let busy = false;
    const report = (ja, en) => setStatus(status, ja, en);

    async function flash() {
      const how = await global.Picoboot.flashUf2(url, report);
      readyToFlash = false;
      report(
        how === "usb" ? "完了しました。書き込みを検証して接続器を再起動しました。" : "完了しました。接続器が再起動します。",
        how === "usb" ? "Done. The write was verified and the device is rebooting." : "Done. The device is restarting."
      );
    }

    button.addEventListener("click", async () => {
      if (busy) return;
      busy = true;
      button.disabled = true;
      try {
        if (readyToFlash) {
          await flash();
          return;
        }

        // V1.1/V1.2は本体からDFUへ移すコマンドを持たないので、物理BOOTSELを案内する。
        if (bootMode === "manual") {
          readyToFlash = true;
          report(
            "接続器を外し、BOOTを押したままUSBを挿してください。RPI-RP2が出たら、もう一度このボタンを押して書き込みます。",
            "Unplug the device, hold BOOT while reconnecting USB, then click this button again after RPI-RP2 appears."
          );
          return;
        }

        const entered = await global.Picoboot.enterBootsel(report, activeHidDeviceProvider?.() || null);
        readyToFlash = true;
        if (!entered) {
          report(
            "自動でDFUモードにできませんでした。接続器を外し、BOOTを押したままUSBを挿してください。RPI-RP2が出たら、もう一度このボタンを押して書き込みます。",
            "Could not enter DFU automatically. Unplug the device, hold BOOT while reconnecting USB, then click this button again after RPI-RP2 appears."
          );
          return;
        }

        try {
          await flash();
        } catch (error) {
          // DFU移行でユーザー操作が切れた初回だけ、ドライブ選択を次のクリックで開く。
          if (/もう一度ボタンを押してください|click the button once more/i.test(error.message)) {
            report(
              "ファーム更新モードに入りました。もう一度このボタンを押して、RPI-RP2ドライブを選んでください。",
              "Firmware update mode is ready. Click this button again and choose the RPI-RP2 drive."
            );
            return;
          }
          throw error;
        }
      } catch (error) {
        report("書き込みに失敗しました: " + error.message, "Flashing failed: " + error.message);
      } finally {
        busy = false;
        button.disabled = false;
      }
    });
  }

  // 固定版のページはdata属性で対象UF2とDFUへの入り方を宣言する。
  function bindStaticFlashers() {
    document.querySelectorAll("[data-firmware-flash]").forEach(root => {
      const button = root.querySelector("[data-firmware-flash-button]");
      const status = root.querySelector("[data-firmware-flash-status]");
      if (!button || !status || !root.dataset.firmwareFlash || !global.Picoboot) return;
      bindFlashButton(button, status, root.dataset.firmwareFlash, root.dataset.firmwareBoot || "auto");
    });
  }

  // V1.1の選択UIは選択値が変わるたびに作り直すため、呼び出し側へ生成関数を公開する。
  function appendFlashButton(target, url, bootMode) {
    if (!global.Picoboot || !global.isSecureContext) return;
    const button = document.createElement("button");
    button.type = "button";
    button.className = "fw-flash-button";
    button.dataset.firmwareFlashButton = "";
    const jaText = document.createElement("span");
    jaText.className = "i18n-ja";
    jaText.textContent = "このファームを書き込む";
    const enText = document.createElement("span");
    enText.className = "i18n-en";
    enText.textContent = "Flash this firmware";
    button.append(jaText, enText);

    const status = document.createElement("span");
    status.className = "fw-picker-note";
    status.dataset.firmwareFlashStatus = "";
    target.append(button, status);
    bindFlashButton(button, status, url, bootMode || "auto");
  }

  // V1.3は設定ページで明示的に選んだHID接続を渡し、同じVIDの別機器を避ける。
  function setActiveHidDeviceProvider(provider) {
    activeHidDeviceProvider = typeof provider === "function" ? provider : null;
  }

  global.HIDtaikoFirmwareFlasher = { appendFlashButton, setActiveHidDeviceProvider };
  document.addEventListener("DOMContentLoaded", bindStaticFlashers);
})(window);
