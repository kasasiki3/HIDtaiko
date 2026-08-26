#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

// HID レポートの ID を定義
enum
{
    REPORT_ID_KEYBOARD = 1 // キーボードのレポート ID
};

extern uint8_t usb_mode;   // 0=PCキーボード 1=Switchタタコン
extern int kando[15];      // 感度・遅延設定（フラッシュから読込）

// Switch用HIDレポート。buttons(2)+hat(1)+4軸(4)+vendor(1)の8バイト
typedef struct __attribute__((packed)) {
    uint16_t buttons;
    uint8_t  hat;
    uint8_t  lx, ly, rx, ry;
    uint8_t  vendor;
} switch_report_t;

#endif /* USB_DESCRIPTORS_H_ */
