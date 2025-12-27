#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>
#include <unistd.h>
#include <cstdlib>

#include "hardware/flash.h"//EEPROM(フラッシュメモリを使用)
#include "hardware/sync.h"
#include "hardware/irq.h"

#include "bsp/board.h" //HID関係
#include "tusb.h"
#include "usb_descriptors.h"
#include "keyboard.h"
#include <hardware/flash.h>


static void save_setting_to_flash(void){
    const uint32_t FLASH_TARGET_OFFSET = 0x1F0000;
    
    uint16_t kando_uint16_t[9];  // uint16_t型の配列に変更
    for(int i = 0; i < 9; i++){
        kando_uint16_t[i] = static_cast<uint16_t>(kando[i]);
    }

    uint8_t buffer[FLASH_PAGE_SIZE] = {0}; // 256バイトのバッファを用意
    memcpy(buffer, kando_uint16_t, sizeof(kando_uint16_t)); // バッファにデータをコピー

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
}




uint8_t g_read_data[9];
int kando3[9];

void load_setting_from_flash(void){
    const uint32_t FLASH_TARGET_OFFSET = 0x1F0000;
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

    uint16_t kando_uint16_t[9];
    memcpy(kando_uint16_t, flash_data, sizeof(kando_uint16_t)); // フラッシュからデータをコピー

    for(int i = 0; i < 9; i++){
        kando[i] = static_cast<int>(kando_uint16_t[i]);
        printf("%d\n", kando[i]); 
    }
}


void hid_task(void);

KeyBoard keyboard;
void serial_task(void);

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) 
{
	(void)itf;
	(void)rts;

	if (dtr) {
		//tud_cdc_write_str("Connected\n");
	}
}
volatile bool data_received = false;
// データが届いたときに呼び出されるコールバック
void tud_cdc_rx_cb(uint8_t itf) {
    data_received = true;  // データが届いたことを示すフラグを立てる
}

 int usb_hid_poll_interval = 1; // Set polling rate to 1ms
 
/*------------- メイン関数 -------------1*/
int main(void){
    board_init();
    tusb_init();
    adc_init();
    adc_gpio_init(26);
    stdio_init_all();
    load_setting_from_flash();

    while (1){
        tud_task(); // tinyusbデバイスタスク
        hid_task(); // キーボードの処理
    if(data_received){
        data_received = false;
        serial_task();//シリアル通信
    }
    }
    return 0;
}

//-----------------------USB-----------------------
int ii = 0;
bool serial_Mode[3];//何を実行するか決めるフラグ
int count = 0;
void serial_task(void){

    char buf[32];
    int a = tud_cdc_read(buf, sizeof(buf)); // USBからの読み取り
    if (a <= 0) {
    return;
}
    buf[a] = '\0'; // 終端文字を追加
    int received_value = std::atoi((char *)buf); // 文字列を整数に変換
  //  printf("Received number: %d\n", received_value); // デバッグ出力

  if( serial_Mode[2]){
                //splitしてから文字列を整数に変換する
        char *token = strtok(buf, "/n");
        
    while (token != NULL) {
        // ":"でキーと値を分割
        char *colon = strchr(token, ':');
        if (colon) {
            *colon = '\0';                 // ":"を区切りとして終端
            int key = atoi(token);         // キーを整数に変換
            int value = atoi(colon + 1);   // 値を整数に変換
            printf("Key: %d, Value: %d\n", key, value);
            kando[key] = value;
            count ++;
            if(count == 9){
                count = 0;
                serial_Mode[2] = false;
            }
            
        }
        token = strtok(NULL, " "); // 次のトークンに進む
    }
  }
    
    if(received_value == 1000){//ヘッダー数値により、モード変更
    //printf("1000");

        for(int i = 0; i < 9; i++) {
            printf("%d:%d\n", i, kando[i]);
            sleep_us(5000);
        }
    }
    else if(received_value == 1001){
         //printf("1001");
        save_setting_to_flash();
        //printf("1001");

    }
    else if(received_value == 1002){
       // printf("1002");
        serial_Mode[2] = true;
    }
    else if(received_value == 1003){
        load_setting_from_flash();
        //printf("1003");

    }

}
/*
static void send_hid_report(bool keys_pressed)
{
    // HIDがまだ準備できていない場合はスキップ
    if (!tud_hid_ready())
    {
        return;
    }

    // 複数のゼロレポート送信を回避
    static bool send_empty = false;


 if (keys_pressed)
    {
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keyboard.key_codes);
        send_empty = true;
        sleep_ms(kando[7]);
    }
    else
    {
        // 前回キーが押されていた場合は空のキー報告を送信
        if (send_empty)
        {
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        }
        send_empty = false;
    }
}
    */
   static void send_hid_report(bool keys_pressed)
   {
       static bool send_empty = false;
       static uint32_t last_send_time = 0;
       static bool waiting = false;
   
       uint32_t now = keyboard.millis();
   
       if (!tud_hid_ready()) return;
   
       if (keys_pressed && !waiting)
       {
           tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keyboard.key_codes);
           send_empty = true;
           waiting = true;
           last_send_time = now;
       }
       else if (waiting)
       {
           // kando[7]ミリ秒経過したら空レポートを送信
           if (now - last_send_time >= kando[7]) {
               tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
               waiting = false;
           }
       }
       else
       {
           // キーが離されていて、直前に何か送っていたら空レポートを送信
           if (send_empty) {
               tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
               send_empty = false;
           }
       }
   }
   

// 10msごとにピンをポーリングし、レポートを送信
void hid_task(void)
{

    // キーが押されているか確認
    bool const keys_pressed = keyboard.update();

    // リモートウェイクアップ
    if (tud_suspended() && keys_pressed)
    {
        // サスペンドモードでホストがリモートウェイクアップを許可している場合、ホストをウェイクアップ
        tud_remote_wakeup();
    }
    else
    {
        // レポートを送信
        send_hid_report(keys_pressed);
    }
}

// レポート送信が成功したときに呼び出される
// アプリケーションはこれを使用して次のレポートを送信できる
// 注意: コンポジットレポートの場合、report[0]はレポートID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
{
    // 未実装: REPORT_ID_KEYBOARDのみを送信
    (void)instance;
    (void)len;
}

// GET_REPORTコントロールリクエストを受信したときに呼び出される
// アプリケーションはバッファにレポートの内容を入力し、その長さを返す必要がある
// ゼロを返すとリクエストはスタックでSTALLする
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    // TODO 未実装
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// SET_REPORTコントロールリクエストを受信、またはOUTエンドポイントでデータを受信したときに呼び出される

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize){
    (void)instance;
}

//-----------------デバイスコールバック-----------------
// デバイスがマウントされたときに呼び出される
//void tud_mount_cb(void){}

// デバイスがアンマウントされたときに呼び出される
//void tud_umount_cb(void){}

// USBバスがサスペンドされたときに呼び出される
// remote_wakeup_en: ホストがリモートウェイクアップを許可している場合
// 7ms以内に、デバイスは平均2.5mA以下の電流をバスから消費する必要がある
void tud_suspend_cb(bool remote_wakeup_en){
    (void)remote_wakeup_en;
}
