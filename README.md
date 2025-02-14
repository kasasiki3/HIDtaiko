# HIDtaiko 
低価格で高性能なおうち太鼓

## ・基盤
　画像の寸法でベニヤ板をカットし、各ねじを用い組み立てます。組み立ての詳細はyoutubeを参照してください。また、販売も行っています。
#### 部品
| 部品 | 数量 | 品番,リンク |
| ---- | ---- | ---- |
| 防振ゴム | 14 |VD2-2015M6|
| M6ネジ | 14 | M6L16 |
| M6ナット |14 |適当(ネジピッチに注意)|
| M6ワッシャー | 14 |適当|
|ラワンベニヤ | 12mm | ホームセンター |  
|圧電素子| 4 |LF-W31E17B or 純正|
|工作用ケーブル|8|0.8～1.5m|
|熱収縮チューブ(任意)|1 |ケーブルに合わせたサイズ|

## ・接続器
#### HIDtaikoの接続器は大きく分けて2種類に分かれています。以下に概要を示します。
 #### 1.Arduino IDEを使用するタイプ
※改造目的、switchでの使用、arduinoIDEに関する知識がある人におすすめです。
※ver1.2接続器,ver1.3接続器,ver2.0接続器
・改造や機能の追加が簡単
・pc,switchに対応
・感度変更のたびにスケッチの書き込みが必要
・制作にはライブラリの導入が必要
・制作にはarduinoに関する知識が必要
#### 2.rp2040を使用するタイプ
※簡単に接続器を作りたい人。とりあえずこっちがおすすめです。
※HIDtaiko_rp2040
・Raspberry Pi Pico SDKを使用
・WEBブラウザで簡単に感度変更が可能
・制作が簡単
・小型
・制作費用800円以下
・改造が比較的難しい

## 性能(参考)
##### どちらの接続器も性能は同じくらいです。
・大体のロール処理は入ります。
・ロール連打(一振り):13
・旧ダブル連打(ドキムネ):1100
・
## Arduino IDEを使用した接続器の詳細
　ハードウェアはマイコンボードと抵抗を並列につないだ回路です。これにスケッチ(プログラム)を書き込むことで接続器とし動作します。(非常に簡単)
 ・1.2接続器,1.3接続器は基板とキースイッチを用い、デザインと生産性を向上させたものです。
 ・2.0接続器は基板、キースイッチ、ディスプレイ、オペアンプを用い太鼓性能の向上を目指し、ハードウェアでの感度変更機能を搭載した物です。(重大なバグがありまともに使用できません。)
### 部品
|部品|数量|品番,リンク|
|----|---|----|
|pro micro|1|[マイコンボード](https://www.amazon.co.jp/%E9%81%8A%E8%88%8E%E5%B7%A5%E6%88%BF-Pro-Micro-%E9%9D%92%E5%9F%BA%E6%9D%BF-2%E5%80%8B%E3%82%BB%E3%83%83%E3%83%88/dp/B0C2K6LYB4?__mk_ja_JP=%E3%82%AB%E3%82%BF%E3%82%AB%E3%83%8A&crid=27ALDBX8UE89Z&dib=eyJ2IjoiMSJ9.m__bSztH0BgMLF4mlpfWx46lJGAAx0u8LyPqb5RbXQIONCniD06gNeqzbCrvzciTHv70bczz8PmWnYu3eGPmxuZqG-YqUVxF3gxCCLVRuVkqm5hyj-KjGRMEjYwcsRN9UD68SsIvP6Z3Bic6-MRDB1IMgdzrPafN1NCz_OmDFZIaQT21UDPk7pCxoQ5LYbbNxmhnAz30EfdWCqq6-Ev4ITj76OYF1JMMAHBSq78_29e5pW67F9RsKXut_Y2_Es9N5mF1vQ0K-w0V4fLtBNKtF37G9WK-b0nd6F7D0gd5UHT_x9Yuk07w3DH6dXhB8WnyiMj1bWwXPPfGRAUMCRHj5t8jZX5_m-gb8FRYqo_xdboQcK91N9Y-FnY-CcZXLs4uWED4_ULS9bTYfUhEyBtnoA8dKT1DtR8BemhPgVAAc42AlwDs8gy6m00_TCO0yolg.rhHQPvdRTxe4hx-lp_ZJ6uXop3bHxDHXbhYJplzmLyU&dib_tag=se&keywords=pro+micro&qid=1739381254&sprefix=pro+micro%2Caps%2C180&sr=8-6)|
|1/4W1M1Ω抵抗|4|[抵抗セット](https://www.amazon.co.jp/%E9%87%91%E5%B1%9E%E7%9A%AE%E8%86%9C%E6%8A%B5%E6%8A%97%E5%99%A8-%E9%9B%BB%E5%AD%90%E5%B7%A5%E4%BD%9C%E3%81%AE%E5%9F%BA%E6%9C%AC%E9%83%A8%E5%93%81-10R-1M%E6%8A%B5%E6%8A%97-%E3%80%90600pc%E3%80%91%E9%AB%98%E7%B2%BE%E5%BA%A6-%E9%9B%BB%E5%AD%90%E5%AE%9F%E9%A8%93%E7%94%A8%E5%9F%BA%E6%9C%AC%E9%83%A8%E5%93%81%E3%82%BB%E3%83%83%E3%83%88/dp/B0CT8PJ97X?__mk_ja_JP=%E3%82%AB%E3%82%BF%E3%82%AB%E3%83%8A&crid=3DTPG422SB58P&dib=eyJ2IjoiMSJ9.HPo6cirzJac7uI1tfT_N85IQ6L3fRrjcaKYir7R0MYyLHjzlB0WQ2nP15J2VRcfInKSY-z4aOE_0S4SVv5hnFZX7KhfJJB0xWaBEEzyA-A62DeLw6c5Zzc3VMS1hlS1wRc_4Y3Cm-NgH0zO0PD87op1PUAImKNN6t4B8R_QtP7WI-jQRaUm8eXPwkrzHnxbXmz363M6mCCQSVjn67nErAPcugBZugG7UlLTXLAtfGtJ3tw0fDtHKCuCt448jF5VZmz-f3Ff10Mb9EhULwxeTbgM0MmbGyJrsr4OkvB6RQIW5ab4GeLep38zuOsxZWA1syMZNUzY2DmW10o_oLSxgfkix0jSiZ3QEfTQFl9zwdugECZ6Xsh2in50vSO6SU-uScKws66la3rdCb8QNuy2TuX7vIjx319BRIC_1jJiaOo8fUfmyLMAxE9nLcJQT3nta.UQD11BkzrWHRtdP8rksHAw5IunN_wv8r4x1YsBNv9r8&dib_tag=se&keywords=1%2F4%E6%8A%B5%E6%8A%97&qid=1739381336&sprefix=1%2F4%E6%8A%B5%E6%8A%97%2Caps%2C178&sr=8-1&th=1)|
|ブレットボード,基板|1|[ブレットボード基板](https://www.amazon.co.jp/%E3%82%A4%E3%83%81%E3%82%B1%E3%83%B3-1%E5%88%97%E5%A4%9A%E3%81%84%E3%83%96%E3%83%AC%E3%83%83%E3%83%89%E3%83%9C%E3%83%BC%E3%83%89%E3%83%97%E3%83%AA%E3%83%B3%E3%83%88%E5%9F%BA%E6%9D%BF-%E3%81%AF%E3%82%93%E3%81%A0%E4%BB%98%E3%81%91%E5%8F%AF%E8%83%BD-%E9%87%91%E3%83%A1%E3%83%83%E3%82%AD%E4%BB%95%E4%B8%8A%E3%81%92%E3%81%AE%E4%B8%8A%E7%B4%9A%E5%93%81-%E3%83%8F%E3%83%BC%E3%83%95%E3%82%B5%E3%82%A4%E3%82%BA/dp/B0CMHD34G6?__mk_ja_JP=%E3%82%AB%E3%82%BF%E3%82%AB%E3%83%8A&crid=2AGZU6PXL321E&dib=eyJ2IjoiMSJ9.wVtwTgZhb9ysKD6uEqIX780UJUFfp8svxAUybcynxIoUBe82WqKeXeaE9ECm0sQ2iyRU3kvZMNz523VUIIYMKtbEQgeMZip9m4zydpBzVBPd7-0DbsaNqx_btTvpp_itK6NDSD6aI10dg8qUBkfpGYiGOwiAZ6f3nnFbecyeetcxkvOId8cMcNBY7EcN_TxiOMxpwxxmBzPOeHQaHOAQ5_P9Kf11aYzuOZtBYblSCr2H2DaHxXWa4cVuh3msl3JVEDzF2I1aO2GsNnsqaCd4a6bEXe1A5X_YQnlap24PyBXeWaRx7nIuZdgSTbE754HVYI6sM1xHYdAnBLJ6Zql4dotsuK4Ah6KSsG1sVMU2FgCn6xK9AWMZr5BGYeia5X50GvV-RfWZsZ7xFQO6jn8sZ2X9zYFrIXJJ6jaAgEHZ4qXMeSwW37KOdCW8RfWpcJp5.uz8NXzEyEV7tRfYqtuBsoBLc-NLwDiDgjJn2IQjpa6M&dib_tag=se&keywords=%E3%83%96%E3%83%AC%E3%83%83%E3%83%89%E3%83%9C%E3%83%BC%E3%83%89%E5%9F%BA%E6%9D%BF&qid=1739381379&sprefix=%E3%83%96%E3%83%AC%E3%83%83%E3%83%88%E3%83%9C%E3%83%BC%E3%83%89%E5%9F%BA%E6%9D%BF%2Caps%2C173&sr=8-1)|
### ハードウェア(配線)
以下の画像の様に配線してください。
※adc入力が4つあり勝つHID機能が使えるマイコンボードでのみ動作します。
<img src=images\images\wiring.png width= "700px" >

### マイコンボードへの書き込み
 1.[arduinoIDE](https://www.arduino.cc/en/software)をインストール

 2.switchを使用する場合は[こちらの記事](https://zenn.dev/kasashiki/articles/5c34ef0c962846)を参考にライブラリのインクルード

 3.[こちら](arduino_minimum)から用途に応じたスケッチを選択、ArduinoIDEにコピペし、マイコンボードに書き込む
### ・switchでの使用
※現在メンテナンスをしていないので動作に関して保障できません。
　ArduinoIDEの設定に関しては[こちらの記事](https://zenn.dev/kasashiki/articles/5c34ef0c962846)を参照してください。

## rp2040を使用した接続器の詳細
 回路はArduinoIDEを用いた接続器と同じです。プログラムの書き込みが非常に簡単で、webサイトで感度変更が可能なのが特徴です。

 ### 部品
 ケースとネジが付いている物はオプションです。無くても動作します。この接続器はキット、完成品を販売予定です。
 | 部品 | 数量 | 品番,リンク |
| ---- | ---- | ---- |
| rp2040搭載ボード | 1 |[adcpinが4つある物](https://ja.aliexpress.com/item/1005005407839815.html?spm=a2g0o.order_list.order_list_main.10.54b5585aH5Qkf0&gatewayAdapt=glo2jpn)|
|  1/4W1M1Ω抵抗|4|[抵抗セット](https://www.amazon.co.jp/%E9%87%91%E5%B1%9E%E7%9A%AE%E8%86%9C%E6%8A%B5%E6%8A%97%E5%99%A8-%E9%9B%BB%E5%AD%90%E5%B7%A5%E4%BD%9C%E3%81%AE%E5%9F%BA%E6%9C%AC%E9%83%A8%E5%93%81-10R-1M%E6%8A%B5%E6%8A%97-%E3%80%90600pc%E3%80%91%E9%AB%98%E7%B2%BE%E5%BA%A6-%E9%9B%BB%E5%AD%90%E5%AE%9F%E9%A8%93%E7%94%A8%E5%9F%BA%E6%9C%AC%E9%83%A8%E5%93%81%E3%82%BB%E3%83%83%E3%83%88/dp/B0CT8PJ97X?__mk_ja_JP=%E3%82%AB%E3%82%BF%E3%82%AB%E3%83%8A&crid=3DTPG422SB58P&dib=eyJ2IjoiMSJ9.HPo6cirzJac7uI1tfT_N85IQ6L3fRrjcaKYir7R0MYyLHjzlB0WQ2nP15J2VRcfInKSY-z4aOE_0S4SVv5hnFZX7KhfJJB0xWaBEEzyA-A62DeLw6c5Zzc3VMS1hlS1wRc_4Y3Cm-NgH0zO0PD87op1PUAImKNN6t4B8R_QtP7WI-jQRaUm8eXPwkrzHnxbXmz363M6mCCQSVjn67nErAPcugBZugG7UlLTXLAtfGtJ3tw0fDtHKCuCt448jF5VZmz-f3Ff10Mb9EhULwxeTbgM0MmbGyJrsr4OkvB6RQIW5ab4GeLep38zuOsxZWA1syMZNUzY2DmW10o_oLSxgfkix0jSiZ3QEfTQFl9zwdugECZ6Xsh2in50vSO6SU-uScKws66la3rdCb8QNuy2TuX7vIjx319BRIC_1jJiaOo8fUfmyLMAxE9nLcJQT3nta.UQD11BkzrWHRtdP8rksHAw5IunN_wv8r4x1YsBNv9r8&dib_tag=se&keywords=1%2F4%E6%8A%B5%E6%8A%97&qid=1739381336&sprefix=1%2F4%E6%8A%B5%E6%8A%97%2Caps%2C178&sr=8-1&th=1)|
|4×2のピンヘッダ|1|[L字ピンヘッダ](https://www.amazon.co.jp/dp/B00TRTIB7O?ref=ppx_yo2ov_dt_b_fed_asin_title)|
|基板| 1 |[プリント基板](C:\Users\kasashiki\Documents\HIDtaiko\HIDtaiko\HIDtaiko_connector_rp2040\kicad_pcb)or[ブレットボード基板](https://www.amazon.co.jp/%E3%82%A4%E3%83%81%E3%82%B1%E3%83%B3-1%E5%88%97%E5%A4%9A%E3%81%84%E3%83%96%E3%83%AC%E3%83%83%E3%83%89%E3%83%9C%E3%83%BC%E3%83%89%E3%83%97%E3%83%AA%E3%83%B3%E3%83%88%E5%9F%BA%E6%9D%BF-%E3%81%AF%E3%82%93%E3%81%A0%E4%BB%98%E3%81%91%E5%8F%AF%E8%83%BD-%E9%87%91%E3%83%A1%E3%83%83%E3%82%AD%E4%BB%95%E4%B8%8A%E3%81%92%E3%81%AE%E4%B8%8A%E7%B4%9A%E5%93%81-%E3%83%8F%E3%83%BC%E3%83%95%E3%82%B5%E3%82%A4%E3%82%BA/dp/B0CMHD34G6?__mk_ja_JP=%E3%82%AB%E3%82%BF%E3%82%AB%E3%83%8A&crid=2AGZU6PXL321E&dib=eyJ2IjoiMSJ9.wVtwTgZhb9ysKD6uEqIX780UJUFfp8svxAUybcynxIoUBe82WqKeXeaE9ECm0sQ2iyRU3kvZMNz523VUIIYMKtbEQgeMZip9m4zydpBzVBPd7-0DbsaNqx_btTvpp_itK6NDSD6aI10dg8qUBkfpGYiGOwiAZ6f3nnFbecyeetcxkvOId8cMcNBY7EcN_TxiOMxpwxxmBzPOeHQaHOAQ5_P9Kf11aYzuOZtBYblSCr2H2DaHxXWa4cVuh3msl3JVEDzF2I1aO2GsNnsqaCd4a6bEXe1A5X_YQnlap24PyBXeWaRx7nIuZdgSTbE754HVYI6sM1xHYdAnBLJ6Zql4dotsuK4Ah6KSsG1sVMU2FgCn6xK9AWMZr5BGYeia5X50GvV-RfWZsZ7xFQO6jn8sZ2X9zYFrIXJJ6jaAgEHZ4qXMeSwW37KOdCW8RfWpcJp5.uz8NXzEyEV7tRfYqtuBsoBLc-NLwDiDgjJn2IQjpa6M&dib_tag=se&keywords=%E3%83%96%E3%83%AC%E3%83%83%E3%83%89%E3%83%9C%E3%83%BC%E3%83%89%E5%9F%BA%E6%9D%BF&qid=1739381379&sprefix=%E3%83%96%E3%83%AC%E3%83%83%E3%83%88%E3%83%9C%E3%83%BC%E3%83%89%E5%9F%BA%E6%9D%BF%2Caps%2C173&sr=8-1)|
|M3×5mmネジ|4|[ねじセット](https://www.amazon.co.jp/%E3%82%B5%E3%83%A0%E3%82%B3%E3%82%B9-%E7%9A%BF%E9%A0%AD%E5%B0%8F%E3%83%8D%E3%82%B8-M3%E3%82%B9%E3%83%86%E3%83%B3%E3%83%AC%E3%82%B9%E3%81%AD%E3%81%98-180%E5%80%8B%E5%85%A5%E3%82%8A-340%E5%80%8B%E3%82%BB%E3%83%83%E3%83%88%EF%BC%88%E5%8F%8E%E7%B4%8D%E3%82%B1%E3%83%BC%E3%82%B9%E4%BB%98%E3%81%8D%EF%BC%89/dp/B09SCV6HL5?__mk_ja_JP=%E3%82%AB%E3%82%BF%E3%82%AB%E3%83%8A&crid=3PYVLE7ZXF27C&dib=eyJ2IjoiMSJ9.WYM2WdbStyCmseHVPbiZ8lk7UqkA-c21DRwH4ztqT7FeTU5QovEO6Ac5lUm_6mxhfMfxAbElsQJkQyuBRebRoOahu5gNG8cHpRGK8qJpjGggg9uPre-Ll_nQ8nXp0y4nGDPOMbeYwEbPb-vfAaWopfwpZl07e3A52yDfoKJ-njJTUOGKz8v85jdomX85ZCIQtE8doItuHpj-QegWI8nHd84WigrADfaPw3uVgxpqIOOI_RUF44BuJYamR_w5SuY3DxXKh3PwSPO8LmM14tN31o_3uYEeCQhK5M6bB0U7cSX45wMHH1xmfP8rhYbIS9mhDUG6YD4jhFO1n0fBeG9Yly1THMTPrDMkztsAxeVnED3tkZc7qDrdW73Apd2vcoKiA17fyHl4aUHOSzlVv6ZlPWtGORgvqkc-OhN9LgAPt38ezBEjSyMHl-JBhapz4HU9.-M_J3a8lueYuVcQMOfoN6w_5m5cI7szOaETCfwOMZV8&dib_tag=se&keywords=m3+%E3%81%AD%E3%81%98&qid=1739424693&sprefix=m3%E3%81%AD%E3%81%98%2Caps%2C184&sr=8-5)|  
|3Dプリントケース| 1 |[stlファイル](HIDtaiko_connector_rp2040\case)|

### 接続器の組み立て
　プリント基板、ブレットボード基板を用いた接続器の組み立て方法は、[こちらの記事]()で詳細に解説しています。

## License
*** by kasashiki is licensed under the Apache License, Version2.0

#自作おうち太鼓#HIDtaiko#接続器
