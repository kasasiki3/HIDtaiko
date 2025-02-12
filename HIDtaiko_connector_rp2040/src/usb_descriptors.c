#include "tusb.h"
#include "usb_descriptors.h"

/* 複数のインターフェースの組み合わせは一意のプロダクトIDを持つ必要があります。これは、PCが最初の接続後にデバイスドライバーを保存するためです。
 * 同じVID/PIDで異なるインターフェースを持つ場合（例: 最初にMSC、後にCDC）は、PCでシステムエラーが発生する可能性があります。
 *
 * 自動プロダクトIDレイアウトのビットマップ:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USB_PID (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                 _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))

#define USB_VID 0xcafe
#define USB_BCD 0x0200

//--------------------------------------------------------------------+
// デバイスディスクリプタ
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
    {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = USB_BCD,
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = USB_VID,
        .idProduct = USB_PID,
        .bcdDevice = 0x0100,

        .iManufacturer = 0x01,
        .iProduct = 0x02,
        .iSerialNumber = 0x03,

        .bNumConfigurations = 0x01};

// GET DEVICE DESCRIPTOR 受信時に呼び出される
// アプリケーションはディスクリプタへのポインタを返す
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// HIDレポートディスクリプタ
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] = {TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD))};

// GET HID REPORT DESCRIPTOR 受信時に呼び出される
// アプリケーションはディスクリプタへのポインタを返す
// ディスクリプタの内容は転送が完了するまで存在する必要がある
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return desc_hid_report;
}

//--------------------------------------------------------------------+
// コンフィギュレーションディスクリプタ
//--------------------------------------------------------------------+

enum
{
	ITF_NUM_HID,
	ITF_NUM_CDC,
	ITF_NUM_CDC_DATA,
	ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_CDC_DESC_LEN)

#define EPNUM_HID 0x84

#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT   0x02
#define EPNUM_CDC_IN    0x82

uint8_t const desc_configuration[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 1),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64)
};

#if TUD_OPT_HIGH_SPEED
// USB仕様によると、高速対応デバイスはdevice_qualifierおよびother_speed_configurationを報告する必要がある

// 他のスピード設定
uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

// device_qualifierは、主にdeviceディスクリプタと類似している。スピードに基づいて設定を変更しないため。
tusb_desc_device_qualifier_t const desc_device_qualifier =
    {
        .bLength = sizeof(tusb_desc_device_qualifier_t),
        .bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER,
        .bcdUSB = USB_BCD,

        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,

        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
        .bNumConfigurations = 0x01,
        .bReserved = 0x00};

// GET DEVICE QUALIFIER DESCRIPTOR リクエスト受信時に呼び出される
// アプリケーションはディスクリプタへのポインタを返す。内容は転送が完了するまで存在する必要がある。
uint8_t const *tud_descriptor_device_qualifier_cb(void)
{
    return (uint8_t const *)&desc_device_qualifier;
}

// GET OTHER SPEED CONFIGURATION DESCRIPTOR リクエスト受信時に呼び出される
// アプリケーションはディスクリプタへのポインタを返す。内容は転送が完了するまで存在する必要がある。
// 他のスピード設定。例: 高速の場合、これはフルスピード用
uint8_t const *tud_descriptor_other_speed_configuration_cb(uint8_t index)
{
    (void)index; // 複数の設定用

    // 他のスピード設定は基本的にタイプ=OTHER_SPEED_CONFIGの設定
    memcpy(desc_other_speed_config, desc_configuration, CONFIG_TOTAL_LEN);
    desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

    // この例では、高速モードとフルスピードモードの両方で同じ設定を使用
    return desc_other_speed_config;
}

#endif // 高速

// GET CONFIGURATION DESCRIPTOR 受信時に呼び出される
// アプリケーションはディスクリプタへのポインタを返す
// ディスクリプタの内容は転送が完了するまで存在する必要がある
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index; // 複数の設定用

    // この例では、高速モードとフルスピードモードの両方で同じ設定を使用
    return desc_configuration;
}

//--------------------------------------------------------------------+
// 文字列ディスクリプタ
//--------------------------------------------------------------------+

// 文字列ディスクリプタへのポインタの配列
char const *string_desc_arr[] =
    {
        (const char[]){0x09, 0x04}, // 0: サポートされている言語は英語 (0x0409)
        "@kasashiki",                  // 1: メーカー
        "rp2040 connector",         // 2: 製品
        "2025 01 25",                   // 3: シリアル番号（チップIDを使用するべき）
        "HIDtaiko_rp2040",          // 4: CDC Interface
};

static uint16_t _desc_str[32];

// GET STRING DESCRIPTOR リクエスト受信時に呼び出される
// アプリケーションはディスクリプタへのポインタを返す。内容は転送が完了するまで存在する必要がある。
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;

    uint8_t chr_count;

    if (index == 0)
    {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        // 注意: 0xEEインデックスの文字列はMicrosoft OS 1.0ディスクリプタ
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
            return NULL;

        const char *str = string_desc_arr[index];

        // 最大文字数に制限
        chr_count = strlen(str);
        if (chr_count > 31)
            chr_count = 31;

        // ASCII文字列をUTF-16に変換
        for (uint8_t i = 0; i < chr_count; i++)
        {
            _desc_str[1 + i] = str[i];
        }
    }

    // 最初のバイトは長さ（ヘッダーを含む）、2番目のバイトは文字列タイプ
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}


