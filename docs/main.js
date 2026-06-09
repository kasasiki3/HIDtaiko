let port;
let writer;

let kando = [200, 200, 200, 200, 200, 200, 200, 200, 200]; // 感度保存用

const BUTTON_CLICK_EVENT = document.getElementById("connection");
const BUTTON_CLICK_EVENT_DISC= document.getElementById("disconnection");
const BUTTON_CLICK_EVENT_send = document.getElementById("sendButton");
const BUTTON_CLICK_EVENT_request = document.getElementById("requestButton");
const BUTTON_CLICK_EVENT_save = document.getElementById("saveButton");
const button = document.getElementById("updateValue");
const volumeSlider = document.getElementById("volumeSlider");
const situation_alert = document.getElementById("situation");
//const BUTTON_CLICK_EVENT_read = document.getElementById("readButton");a

let reader; // readerが必要な場合に備えて
situation_alert.textContent = "未接続"

const status_dot = document.getElementById("status-dot");
function setDot(state) {
  status_dot.className = "status-dot" + (state ? " is-" + state : "");
}

// シリアルポートの接続処理
BUTTON_CLICK_EVENT.addEventListener("click", async () => {
  try {
    // ポートがすでに開いている場合、閉じて再接続する
    if (port && port.readable) {
      console.log("Closing existing port...");
      if (writer) writer.releaseLock(); // writerのロックを解放
      if (reader) reader.releaseLock(); // readerのロックを解放
      await port.close(); // ポートを閉じる
    }

    setDot("connecting");
    situation_alert.textContent = "選択中..."

    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });

    // writerとreaderを初期化
    writer = port.writable.getWriter();
    reader = port.readable.getReader();

    setDot("connected");
    situation_alert.textContent = "RP2040接続済み"
    console.log("Port connected successfully!");

    // 初期メッセージを送信
    const data = new TextEncoder().encode("Hello, Serial!\n");
    await writer.write(data);
    console.log("Data sent: Hello, Serial!");
  } catch (error) {
    setDot("error");
    situation_alert.textContent = "接続失敗"
    console.error("Error:", error);
  }
});


// ウィンドウを閉じる前にポートを閉じる
window.addEventListener("beforeunload", async () => {
  if (writer) writer.releaseLock();
  if (reader) reader.releaseLock(); // readerのロックも解放
  if (port) await port.close();
});
BUTTON_CLICK_EVENT_DISC.addEventListener("click", async () => {
  if (writer) writer.releaseLock(); // writerのロックを解放
  if (reader) reader.releaseLock(); // readerのロックを解放
  if (port) await port.close();
  setDot(null);
  situation_alert.textContent = "切断"
});

// データ送信処理
BUTTON_CLICK_EVENT_send.addEventListener("click", async () => {
  try {
    // "1002"の送信
    await writer.write(new TextEncoder().encode("1002\n"));
    setDot("loading");
    situation_alert.textContent = "送信中..."
    for (let i = 0; i < 9; i++){//
      await new Promise(resolve => setTimeout(resolve, 100)); // 10ms待機
      const data = `${i}:${kando[i]}`; // iとkando[i]を「:」で結合
      const data2 =  new TextEncoder().encode(data)
      await writer.write(data2); // UTF-8エンコードして送信
    }
    setDot("connected");
    situation_alert.textContent = "送信完了"
  } catch (error) {
    setDot("error");
    console.error("Error in send:", error);
  }
});

// データ要求処理
BUTTON_CLICK_EVENT_request.addEventListener("click", async () => {
  try {
    // "1000"を送信して感度データを要求
    await writer.write(new TextEncoder().encode("1000\n"));
    setDot("loading");
    situation_alert.textContent = "受信中..."

    for (let i = 0; i <= 8; i++) {
      let isMatched = false;

      while (!isMatched) {
        const data = await reader.read();
        const decodedData = new TextDecoder().decode(data.value); // データをデコード

        // データが予期した形式であるかを確認
        const parts = decodedData.trim().split(":");
        if (parts.length === 2) {
          const [index, value] = parts;

          // データが正しい範囲であることを確認
        //  if (parseInt(index) === i && !isNaN(value)) {
            const textElement = document.getElementById(`text${i}`);
            const volumeSlider = document.getElementById(`volumeSlider${i}`);

            kando[i] = value; // データを配列に格納
            volumeSlider.value = kando[i]; // スライダーを更新
            textElement.textContent = kando[i]; // スライダー横のテキストを更新

            isMatched = true; // 正しいデータを処理したらループを抜ける
          //}
        } else {
          // データが無効な形式の場合はログに記録
         // alert("error");
          console.warn(`Invalid data format received: ${decodedData}`);
        }
      }
    }
    setDot("connected");
    situation_alert.textContent = "受信完了"
  } catch (error) {
    setDot("error");
    console.error("Error in request:", error);
  }
});

BUTTON_CLICK_EVENT_save.addEventListener("click", async () => {
    await writer.write(new TextEncoder().encode("1001\n"));
    console.log("1001_saved");
    setDot("connected");
    situation_alert.textContent = "保存完了"
});
/*
BUTTON_CLICK_EVENT_read.addEventListener("click", async () => {
  await writer.write(new TextEncoder().encode("1003\n"));
  console.log("1003_read");
  alert("read");
});

*/

// スライダーの入力処理
for (let i = 0; i <= 8; i++) {
  const textElement = document.getElementById(`text${i}`);
  const volumeSlider = document.getElementById(`volumeSlider${i}`);

  volumeSlider.addEventListener("input", () => {
    const volume = volumeSlider.value; // スライダーの値を取得
    textElement.textContent = volume; // スライダー横のテキストを更新
    kando[i] = volumeSlider.value;
  
  });
}
for (let i = 0; i <= 8; i++) {
  const textElement = document.getElementById(`text${i}`);
  const volumeSlider = document.getElementById(`volumeSlider${i}`);

    document.addEventListener("DOMContentLoaded", () => {
    const volume = volumeSlider.value; // スライダーの初期値を取得
    textElement.textContent = volume; // スライダー横のテキストを初期化
    kando[i] = volumeSlider.value; // kando 配列に初期値を設定
  });
}

