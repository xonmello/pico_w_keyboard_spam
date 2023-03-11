#include "tusb_config.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <string.h>
#include "bsp/board.h"

#define REPORT_ID_KEYBOARD 0

void led_blinking_task(void);
void hid_task(void);
void send_key_task(void);

enum  {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

int main() {
    board_init();
    cyw43_arch_init();
    tud_init(BOARD_TUD_RHPORT);
    while (true) {
        printf("loop\n");
        led_blinking_task();
        tud_task();
        send_key_task();
    }
    return 0;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    (void) instance;
    (void) report_id;
    (void) report_type;

    // echo back anything we received from host
    tud_hid_report(0, buffer, bufsize);
}

//--------------------------------------------------------------------+
// TUD TASK
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// Blinking Task
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // blink is disabled
    if (!blink_interval_ms) return;

    // Blink every interval ms
    if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
    start_ms += blink_interval_ms;

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
    led_state = 1 - led_state; // toggle
}

//--------------------------------------------------------------------+
// Key Send Task
//--------------------------------------------------------------------+
void send_key_task(void) {
    static uint32_t start_ms = 0;
    static uint32_t send_key_interval = 750;
    static bool has_keyboard_key = false;

    // Blink every interval ms
    if ( board_millis() - start_ms < send_key_interval) return; // not enough time

    start_ms += send_key_interval;
    if (tud_suspended()) tud_remote_wakeup();
    else if (!has_keyboard_key)
    {
        if (!tud_hid_ready()) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            return;
        };
        
        // Send key press 'A'
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
    } else has_keyboard_key = false;
}

