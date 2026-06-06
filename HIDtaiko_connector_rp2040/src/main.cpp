#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>
#include <unistd.h>
#include <cstdlib>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/irq.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "keyboard.h"


static void save_setting_to_flash(void) {
    const uint32_t FLASH_TARGET_OFFSET = 0x1F0000;

    uint16_t kando_uint16_t[9];
    for (int i = 0; i < 9; i++) {
        kando_uint16_t[i] = static_cast<uint16_t>(kando[i]);
    }

    uint8_t buffer[FLASH_PAGE_SIZE] = {0};
    memcpy(buffer, kando_uint16_t, sizeof(kando_uint16_t));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
}

void load_setting_from_flash(void) {
    const uint32_t FLASH_TARGET_OFFSET = 0x1F0000;
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

    uint16_t kando_uint16_t[9];
    memcpy(kando_uint16_t, flash_data, sizeof(kando_uint16_t));

    for (int i = 0; i < 9; i++) {
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
    (void)dtr;
    (void)rts;
}

volatile bool data_received = false;

void tud_cdc_rx_cb(uint8_t itf) {
    data_received = true;
}

/*------------- メイン関数 -------------*/
int main(void) {
    board_init();
    tusb_init();
    adc_init();
    adc_gpio_init(26);
    stdio_init_all();
    load_setting_from_flash();

    while (1) {
        tud_task();
        hid_task();
        if (data_received) {
            data_received = false;
            serial_task();
        }
    }
    return 0;
}

//-----------------------USB-----------------------
bool serial_Mode;
int count = 0;

void serial_task(void) {
    char buf[32];
    int len = tud_cdc_read(buf, sizeof(buf));
    if (len <= 0) {
        return;
    }
    buf[len] = '\0';
    int received_value = std::atoi((char *)buf);

    if (serial_Mode) {
        char *token = strtok(buf, "/n");
        while (token != NULL) {
            char *colon = strchr(token, ':');
            if (colon) {
                *colon = '\0';
                int key = atoi(token);
                int value = atoi(colon + 1);
                printf("Key: %d, Value: %d\n", key, value);
                kando[key] = value;
                count++;
                if (count == 9) {
                    count = 0;
                    serial_Mode = false;
                }
            }
            token = strtok(NULL, " ");
        }
    }

    if (received_value == 1000) {
        for (int i = 0; i < 9; i++) {
            printf("%d:%d\n", i, kando[i]);
            sleep_us(5000);
        }
    } else if (received_value == 1001) {
        save_setting_to_flash();
    } else if (received_value == 1002) {
        serial_Mode = true;
    } else if (received_value == 1003) {
        load_setting_from_flash();
    }
}

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
        if (now - last_send_time >= (uint32_t)kando[7]) {
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            waiting = false;
        }
    }
    else
    {
        if (send_empty) {
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            send_empty = false;
        }
    }
}

void hid_task(void)
{
    bool const keys_pressed = keyboard.update();

    if (tud_suspended() && keys_pressed)
    {
        tud_remote_wakeup();
    }
    else
    {
        send_hid_report(keys_pressed);
    }
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
{
    (void)instance;
    (void)report;
    (void)len;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}

void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
}
