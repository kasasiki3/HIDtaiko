# ◇RP2040接続器ver1.0
#### センサーに1MΩ抵抗を並列に接続した回路です。ver1.1には性能、安定感でやや劣りますが、回路がシンプルになります。
<img src="images\ver1.0_circuit.png" width= "500px" >
<img src="images\rp2040_ver1.0.jpg" width= "500px" >

### 部品表
#### [制作ガイド](https://zenn.dev/kasashiki/articles/28d31814b8bf86)
| 部品 | 数量 | リンク |
| ---- | ---- | ---- |
| rp2040zero | 1 | [adcpinが4つある物](https://ja.aliexpress.com/item/1005005407839815.html) |
| 1/4W1M1Ω抵抗 | 4 | [抵抗セット](https://www.amazon.co.jp/dp/B0CT8PJ97X) |
| 4×2のピンヘッダ | 1 | [L字ピンヘッダ](https://www.amazon.co.jp/dp/B00TRTIB7O) |
| 基板 | 1 | [ブレットボード基板](https://www.amazon.co.jp/dp/B0CMHD34G6) |
| M3×5mmネジ | 4 | [ねじセット](https://www.amazon.co.jp/dp/B09SCV6HL5) |
| 3Dプリントケース | 1 | [stlファイル](https://github.com/kasasiki3/HIDtaiko/tree/master/HIDtaiko_connector_rp2040/case) |

### 性能
※一部のシュミレーターでは入力制限を入れないとキー入力が認識されない可能性があるため、この性能が出ません。

・ドカ、カド処理含め、すべてのロール処理が入ります

・ロール連打(一振り):13

・旧ダブル連打(ドキムネ):1000

#### [プレイ動画](https://www.youtube.com/watch?v=6kISQiHiIU8&t=2s)


# ◇RP2040接続器ver1.1

#### vショットキーバリアダイオードを用いたクランプ回路を追加し、性能、安定感が向上しました。
#### ※制作ガイドは近日公開します。
<img src="images\ver1.1_circuit.png" width= "500px" >
<img src="images\rp2040_ver1.1.jpg" width= "500px" >

### 部品表
| 部品 | 数量 | リンク |
| ---- | ---- | ---- |
| rp2040zero | 1 | [adcpinが4つある物](https://ja.aliexpress.com/item/1005005407839815.html) |
| 1M1Ω抵抗 | 4 | [抵抗セット](https://www.amazon.co.jp/dp/B0CT8PJ97X) |
| ショットキーバリアダイオード | 8 | [動作確認済の物](https://ja.aliexpress.com/item/1005001552094086.html?spm=a2g0o.productlist.main.1.6e44uelOuelObK&algo_pvid=324fc66a-4f51-4271-9d8e-bdbd04cb3111&algo_exp_id=324fc66a-4f51-4271-9d8e-bdbd04cb3111-0&pdp_ext_f=%7B%22order%22%3A%223070%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21JPY%21243%21240%21%21%211.64%211.62%21%402140e67317474622478812536e3626%2112000016564939110%21sea%21JP%214091964399%21X&curPageLogUid=Vd5VeMGYBVVc&utparam-url=scene%3Asearch%7Cquery_from%3A) |
| 1KΩ抵抗 | 4 | [抵抗セット](https://www.amazon.co.jp/dp/B0CT8PJ97X) |
| 4×2のピンヘッダ | 1 | [L字ピンヘッダ](https://www.amazon.co.jp/dp/B00TRTIB7O) |
| 基板 | 1 | [ブレットボード基板](https://www.amazon.co.jp/dp/B0CMHD34G6) |
| M3×5mmネジ | 4 | [ねじセット](https://www.amazon.co.jp/dp/B09SCV6HL5) |
| 3Dプリントケース | 1 | [stlファイル](https://github.com/kasasiki3/HIDtaiko/tree/master/HIDtaiko_connector_rp2040/case) |


### 性能
※一部のシュミレーターでは入力制限を入れないとキー入力が認識されない可能性があるため、この性能が出ません。

・ドカ、カド処理含め、すべてのロール処理が入ります

・ロール連打(一振り):13

・旧ダブル連打(ドキムネ):1200

#### [プレイ動画](https://www.youtube.com/watch?v=wMSDLN9h2Co)

# rp2040zero接続器作り方

## ◇はじめに
#### ・この解説ははんだ付けの知識、簡単な回路図を読む力が必要です。もし不明な点がありましたらDMにて質問してください。
#### ・完成品はメルカリにて販売しています。[(メルカリ)](https://jp.mercari.com/user/profile/175469287)
#### ・はんだごて、ニッパーが必要です。
#### ・

## ◇使用する部品
<img src="images\rp2040build guide\parts.jpg" width= "500px" >

### 部品表
| 部品 | 数量 | リンク |
| ---- | ---- | ---- |
| rp2040zero | 1 | [adcpinが4つある物](https://ja.aliexpress.com/item/1005005407839815.html) |
| 1M1Ω抵抗 | 4 | [抵抗セット](https://www.amazon.co.jp/dp/B0CT8PJ97X) |
| ショットキーバリアダイオード | 8 | [動作確認済の物](https://ja.aliexpress.com/item/1005001552094086.html?spm=a2g0o.productlist.main.1.6e44uelOuelObK&algo_pvid=324fc66a-4f51-4271-9d8e-bdbd04cb3111&algo_exp_id=324fc66a-4f51-4271-9d8e-bdbd04cb3111-0&pdp_ext_f=%7B%22order%22%3A%223070%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21JPY%21243%21240%21%21%211.64%211.62%21%402140e67317474622478812536e3626%2112000016564939110%21sea%21JP%214091964399%21X&curPageLogUid=Vd5VeMGYBVVc&utparam-url=scene%3Asearch%7Cquery_from%3A) |
| 1KΩ抵抗 | 4 | [抵抗セット](https://www.amazon.co.jp/dp/B0CT8PJ97X) |
| コネクタ | 1 | [L字ピンヘッダ](https://www.amazon.co.jp/dp/B00TRTIB7O) |
| 基板 | 1 | [基板](https://ja.aliexpress.com/item/1005007024264426.html?spm=a2g0o.productlist.main.1.58f3t5bst5bs34&algo_pvid=35c5a8f6-3d56-44ee-9113-470bb06a20f4&algo_exp_id=35c5a8f6-3d56-44ee-9113-470bb06a20f4-0&pdp_ext_f=%7B%22order%22%3A%2214841%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21JPY%21725%21259%21%21%2135.78%2112.79%21%402102f0c917488537225005440e13b2%2112000039118829302%21sea%21JP%214091964399%21X&curPageLogUid=Qk7jOzGtDkLS&utparam-url=scene%3Asearch%7Cquery_from%3A) |
| ジャンパーワイヤー | (任意) | [セット](https://ja.aliexpress.com/item/1005006014728282.html?spm=a2g0o.productlist.main.37.12885206AgZpJh&algo_pvid=561f216c-3a62-4482-b9bc-fc6432844178&algo_exp_id=561f216c-3a62-4482-b9bc-fc6432844178-36&pdp_ext_f=%7B%22order%22%3A%221022%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21JPY%21327%21317%21%21%2116.14%2115.63%21%402102f0c917488538174972166e13be%2112000035329293181%21sea%21JP%214091964399%21X&curPageLogUid=mCZzehOhYLgt&utparam-url=scene%3Asearch%7Cquery_from%3A) |


## ◇回路図、組み立て例
#### 配線の形は自由です。これは例です。RP2040zeroのGPIOpinを確認しながら間違わないようにに配線してください。

<img src="images\rp2040build guide\front.jpg" width= "500px" >

回路全体

<img src="images\rp2040build guide\whole circuit.png" width= "500px" >

この回路を4つ用意します。

<img src="images\ver1.1_circuit.png" width= "500px" >


## ◇rp2040zeroをハンダ付け
#### まずはピンヘッダをrp2040zero固定します。

<img src="images\rp2040build guide\2.jpg" width= "500px" >

#### 次に基板に固定します。

<img src="images\rp2040build guide\4.jpg" width= "500px" >

## ◇コネクタの取付

#### 基板に対して並行になるように調整してください。

<img src="images\rp2040build guide\7.jpg" width= "500px" >

<img src="images\rp2040build guide\8.jpg" width= "500px" >

## ◇その他の電子部品の取付

<img src="images\rp2040build guide\front.jpg" width= "500px" >
<img src="images\rp2040build guide\11.jpg" width= "500px" >

#### 回路を理解し、部品を配置してください。
#### 部品を配置したら、まずは部品を基板にハンダ付けしてください。
#### その後、電子部品のピンを倒し、配線を行ってください。

<img src="images\rp2040build guide\back.jpg" width= "500px" >

## ◇ファームウェア書き込み

#### [ファームウェア](original_uf2/hidtaiko.uf2)をダウンロードし、エクスプローラーで開いてください。
#### 次に、このボタンを押しながらUSBケーブルでPCに接続します

<img src="images\rp2040build guide\3.jpg" width= "500px" >

#### ストレージとして認識されるのでファイルを入れます。

GIF

## ◇感度変更



[感度変更アプリ](https://kasasiki3.github.io/ver1.3_webapp_rp2040version/)こちらにアクセスし、基本の感度を接続器に送ってください。


#### 入力が出来たら完成です。