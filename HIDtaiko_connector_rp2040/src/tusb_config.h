#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

//--------------------------------------------------------------------
// 共通の設定
//--------------------------------------------------------------------

// board.mk によって定義されます
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU は定義されている必要があります
#endif

// デバイスに使用される RHPort 番号は board.mk によって定義可能で、デフォルトはポート 0
#ifndef BOARD_DEVICE_RHPORT_NUM
#define BOARD_DEVICE_RHPORT_NUM 0
#endif

// RHPort の最大動作速度は board.mk によって定義可能です。
// デフォルトは内部 HighSpeed PHY を持つ MCU では HighSpeed で、
// それ以外では FullSpeed
#ifndef BOARD_DEVICE_RHPORT_SPEED
#if (CFG_TUSB_MCU == OPT_MCU_LPC18XX || CFG_TUSB_MCU == OPT_MCU_LPC43XX || CFG_TUSB_MCU == OPT_MCU_MIMXRT10XX || \
     CFG_TUSB_MCU == OPT_MCU_NUC505 || CFG_TUSB_MCU == OPT_MCU_CXD56 || CFG_TUSB_MCU == OPT_MCU_SAMX7X)
#define BOARD_DEVICE_RHPORT_SPEED OPT_MODE_HIGH_SPEED
#else
#define BOARD_DEVICE_RHPORT_SPEED OPT_MODE_FULL_SPEED
#endif
#endif

// board.mk によって定義された rhport と速度でデバイスモードを設定
#if BOARD_DEVICE_RHPORT_NUM == 0
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)
#elif BOARD_DEVICE_RHPORT_NUM == 1
#define CFG_TUSB_RHPORT1_MODE (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)
#else
#error "RHPort の設定が不正です"
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_NONE
#endif

// CFG_TUSB_DEBUG は DEBUG ビルドでコンパイラによって定義されます
// #define CFG_TUSB_DEBUG           0

/* 一部の MCU では USB DMA が特定の SRAM 領域にしかアクセスできない場合があり、アライメントに制限があります。
 * TinyUSB は以下のマクロを使用して転送メモリを宣言し、それらが特定のセクションに配置されるようにします。
 * 例:
 * - CFG_TUSB_MEM_SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

//--------------------------------------------------------------------
// デバイス設定
//--------------------------------------------------------------------

// エンドポイント0のサイズ
#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE 64
#endif

//------------- クラス -------------//
#define CFG_TUD_HID 1
#define CFG_TUD_CDC 1
#define CFG_TUD_MSC 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 0

// HID buffer size Should be sufficient to hold ID (if any) + Data
#define CFG_TUD_HID_EP_BUFSIZE    64

// CDC FIFO size of TX and RX
#define CFG_TUD_CDC_RX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_CDC_TX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)

// CDC Endpoint transfer buffer size, more is faster
#define CFG_TUD_CDC_EP_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
