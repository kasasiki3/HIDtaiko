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

    uint16_t buf16[14] = {0};
    for (int i = 0; i < 10; i++) buf16[i] = static_cast<uint16_t>(kando[i]);
    // buf16[9]: reserved
    for (int i = 0; i < 4; i++) buf16[10 + i] = key_map[i];

    uint8_t buffer[FLASH_PAGE_SIZE] = {0};
    memcpy(buffer, buf16, sizeof(buf16));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
}

void load_setting_from_flash(void) {
    const uint32_t FLASH_TARGET_OFFSET = 0x1F0000;
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

    uint16_t buf16[14];
    memcpy(buf16, flash_data, sizeof(buf16));

    for (int i = 0; i < 10; i++) {
        kando[i] = static_cast<int>(buf16[i]);
        printf("%d\n", kando[i]);
    }
    for (int i = 0; i < 4; i++) {
        uint16_t v = buf16[10 + i];
        if (v >= 4 && v <= 0xFF) key_map[i] = static_cast<uint8_t>(v);
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
    adc_gpio_init(27);
    adc_gpio_init(28);
    adc_gpio_init(29);
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
static char serial_line_buf[128] = {0};
static int serial_line_pos = 0;

static void process_serial_line(const char *line) {
    if (strncmp(line, "SENS:", 5) == 0) {
        char tmp[128];
        strncpy(tmp, line + 5, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        char *tok = strtok(tmp, ":");
        while (tok) {
            char *eq = strchr(tok, '=');
            if (eq) {
                *eq = '\0';
                int k = atoi(tok), v = atoi(eq + 1);
                if (k >= 0 && k < 10) kando[k] = v;
            }
            tok = strtok(NULL, ":");
        }
        printf("OK\n");
    } else if (strncmp(line, "KEY:", 4) == 0) {
        char tmp[32];
        strncpy(tmp, line + 4, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        char *tok = strtok(tmp, ":");
        while (tok) {
            char *eq = strchr(tok, '=');
            if (eq) {
                *eq = '\0';
                int k = atoi(tok), v = atoi(eq + 1);
                if (k >= 0 && k < 4) key_map[k] = static_cast<uint8_t>(v);
            }
            tok = strtok(NULL, ":");
        }
        printf("OK\n");
    } else if (strcmp(line, "GETS") == 0) {
        for (int i = 0; i < 10; i++) printf("%d:%d\n", i, kando[i]);
        printf("OK\n");
    } else if (strcmp(line, "GETK") == 0) {
        for (int i = 0; i < 4; i++) printf("%d:%d\n", i, key_map[i]);
        printf("OK\n");
    } else if (strcmp(line, "SAVE") == 0) {
        save_setting_to_flash();
        printf("OK\n");
    } else if (strcmp(line, "LOAD") == 0) {
        load_setting_from_flash();
        printf("OK\n");
    }
}

void serial_task(void) {
    char buf[64];
    int len;
    while ((len = tud_cdc_read(buf, sizeof(buf))) > 0) {
    for (int i = 0; i < len; i++) {
        char c = buf[i];
        if (c == '\n' || c == '\r') {
            if (serial_line_pos > 0) {
                serial_line_buf[serial_line_pos] = '\0';
                process_serial_line(serial_line_buf);
                serial_line_pos = 0;
            }
        } else if (serial_line_pos < (int)sizeof(serial_line_buf) - 1) {
            serial_line_buf[serial_line_pos++] = c;
        }
    }
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
