# docs Changelog

## 2026-06-06

### 初期統合
- `ver1.3_webapp_rp2040version/docs/` を HIDtaiko リポジトリの `docs/` として統合
- GitHub Pages 配信元として使用予定

### バージョンナビ追加
- ヘッダー左上に V1.1 / V1.2 切り替えリンクを追加（`main.css` に `.version-nav` スタイル追加）
- 元の「RP2040」表記を「V1.1」に修正

### V1.2 ページ追加（`v12.html` / `v12.js`）
- `index.html` をベースに V1.2 用ページを新規作成
- 主な差分：
  - ボーレート 115200 → 9600（`v12.js`）
  - 感度スライダー max: 500、デフォルト: 右縁=80 / 右面=130 / 左面=130 / 左縁=80
  - Delay スライダー max: Adelay=25 / Bdelay=40 / Cdelay=40 / Ddelay=40 / Hdelay=40
  - Delay デフォルト: 8 / 6 / 13 / 10 / 0 ms
  - 接続時ステータス表示「V1.2接続済み」
  - ファームウェア更新手順を Arduino IDE 方式に変更

### EN/JP 切り替え追加（`i18n.js`）
- ヘッダーに EN/JP トグルボタンを追加
- `body.is-en` / `body.is-ja` クラスで `.i18n-ja` / `.i18n-en` 要素を CSS で切り替え
- `localStorage` に言語設定を保存（キー: `hidtaiko-lang`）、ページ跨ぎで引き継ぎ
- 翻訳対象：ボタン、カードタイトル、パッドラベル、説明文全文、ファームウェア手順

### 感度調整ガイド改訂（両ページ共通）
- 手順を2ステップに整理：
  1. 確実な反応を保ちながら感度を下げる（値を大きく）
  2. Ddelay → Bdelay → Adelay → Cdelay の順で各 Delay を最小化
- 感度と Delay のトレードオフを補足として追記
- 日英両対応
