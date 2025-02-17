#ifndef KEYS_H
#define KEYS_H

#include "hardware/gpio.h" // gpio_* 関数
#include "class/hid/hid.h" // HID_KEY_* 定数
#include "../rp2_common\hardware_adc\include\hardware\adc.h"


long int timer_b[4] = {0,0,0,0}; // すべての時間,ドンの時間,カッの時間,
long int timer_n[4];

// GPIOピンとキーコードの対応を表す構造体
struct PinKey
{
	const uint8_t pin; // Picoのピン番号
	const uint8_t key; // HID_KEY_* に対応するキーコード
};

bool farst_connect = false;
int kando[9] = {80,160,160,80,30,12,35,0,0};
/*
0:左縁の感度
1:左面の感度
2:右面の感度
3:右縁の感度
4:面が入力されたときに次の入力を受け付けるまでの時間
5:4の縁版
6:面が入力されたときに縁の入力を受け付けない時間
7:一部のシュミレーターで使用する入力制限
*/

// キーボードクラスの定義
class KeyBoard
{
private:
	const static size_t num_pins = 4; // 使用するピンの数
	const PinKey pin_keys[num_pins] = { // GPIOピンとキーコードのマッピング
		{0, HID_KEY_D},             
		{1, HID_KEY_F},
		{2, HID_KEY_J},
		{3, HID_KEY_K},             
	};

public:
	uint8_t key_codes[6] = {0}; // HIDレポートで送信可能な最大6つのキーコード

	// コンストラクタ
	KeyBoard()
	{
		// 全ピンをプルアップ入力として初期化
		for (size_t i = 0; i < num_pins; i++)
		{
			gpio_init(pin_keys[i].pin);
			gpio_pull_up(pin_keys[i].pin);
			gpio_set_dir(pin_keys[i].pin, GPIO_IN);
		}
	}

	// デストラクタ
	virtual ~KeyBoard() {}
    long int result_ac[4];
    
    uint32_t millis() {
    return to_ms_since_boot(get_absolute_time());
}

	// キー状態を更新し、変更があればtrueを返す
	bool update()
	{
		long int main_timer = millis();
		// キーコードをクリア
		for (size_t i = 0; i < 4; i++)
		{
			key_codes[i] = 0;
		}
		// ピンを読み取り、最大6つのキーコードを設定
		uint8_t index = 0;
		bool changed = false;
		for (size_t i = 0; i < num_pins; i++)
		{
			adc_select_input(i);//このピンのキーを選択
			long int result = adc_read();//選択されたpinの情報を読み取る

			if (result - result_ac[0] > kando[0] && i == 0 && main_timer - timer_b[0] >= kando[4] && main_timer - timer_b[2] > kando[6] && main_timer - timer_n[0] > kando[8]) // 押されている場合
			{
				key_codes[index] = pin_keys[i].key; // キーコードを設定
				changed = true;
				index++;
				timer_b[0] = millis();
				timer_b[1] = millis();
				timer_n[0] = millis();
				if (index >= 3) // 最大6つのキー入力
				{
					break;
				}
			}
			
			else if (result - result_ac[1] > kando[1] && i == 1 && main_timer - timer_b[1] >= kando[5] && main_timer - timer_n[1] > kando[8]) // 押されている場合
			{
				key_codes[index] = pin_keys[i].key; // キーコードを設定
				changed = true;
				index++;
				timer_b[0] = millis();
				timer_b[2] = millis();
				timer_n[1] = millis();
				if (index >= 3) // 最大6つのキー入力
				{
					break;
				}
			}
			else if (result - result_ac[2] > kando[2] && i == 2 && main_timer - timer_b[2] >= kando[5] && main_timer - timer_n[2] > kando[8]) // 押されている場合
			{
				key_codes[index] = pin_keys[i].key; // キーコードを設定
				changed = true;
				index++;
				timer_b[0] = millis();
				timer_b[2] = millis();
				timer_n[2] = millis();
				if (index >= 3) // 最大6つのキー入力
				{
					break;
				}
			}
			else if (result - result_ac[3] > kando[3] && i == 3 && main_timer - timer_b[3] >= kando[4] && main_timer - timer_b[2] > kando[6] && main_timer - timer_n[3] > kando[8]) // 押されている場合
			{
				key_codes[index] = pin_keys[i].key; // キーコードを設定
				changed = true;
				index++;
				timer_b[0] = millis();
				timer_b[1] = millis();
				timer_n[3] = millis();
				if (index >= 3) // 最大6つのキー入力
				{
					break;
				}
			}
        result_ac[i] = result;
		}
		return changed;
	}
};

#endif 
