#include <pico.h>
#include <pico/assert.h>
#include <pico/stdio.h>
#include <stdio.h>
#include <hardware/gpio.h>
#include <tusb.h>
#include <class/hid/hid.h>
#include <bsp/board_api.h>
#include <bsp/board_api.h>

int main() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    board_init();
    tusb_rhport_init_t dev_init = {.role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO};
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    if (board_init_after_tusb) {
        board_init_after_tusb();
    }
    stdio_init_all();

    while (true) {
        void loop(void);
        void gamepad_task(void);
        void blink_task(void);

        tud_task();
        blink_task();
        gamepad_task();
        loop();
    }
    hard_assert(false);
    return 0;
}

#define ALIVE_INTERVAL (4000)
#define BLINK_INTERVAL (500)

void blink_task() {
    static uint32_t last_blink = 0;
    static bool on = false;
    if (board_millis() - last_blink < BLINK_INTERVAL) {
        return;
    }
    on = !on;
    gpio_put(PICO_DEFAULT_LED_PIN, on);
    last_blink = board_millis();
}

void gamepad_task() {
    static bool is_first = true;
    static hid_gamepad_report_t last_report = {.x = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0, .hat = 0, .buttons = 0};
    hid_gamepad_report_t report = {.x = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0, .hat = 0, .buttons = board_button_read() ? 0b00000000000000000000000000000001 : 0b00000000000000000000000000000000};
    if (!is_first) {
        if (memcmp(&last_report, &report, sizeof(report)) == 0) {
            return;
        }
    } else {
        is_first = false;
    }
    last_report = report;
    tud_hid_report(HID_REPORT_ID_GAMEPAD, &report, sizeof(report));
}

void loop() {
    static uint32_t last_send = 0;
    if (board_millis() - last_send < ALIVE_INTERVAL) {
        return;
    }
    printf("Hello, CDC!\n");
    last_send = board_millis();
}

void tud_hid_set_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t const *buffer,
    uint16_t bufsize
) {
    printf("tud_hid_set_report_cb:\n");
    printf("instance=%x\n", instance);
    printf("report_id=%u\n", report_id);
    printf("report_type=%u\n", report_type);
    printf("bufsize=" PRIu16 "\n", bufsize);
    printf("Buffer contents:\n");
    for (int i = 0; i < bufsize; ++i) {
        printf("    %d%s\n", buffer[i], i == bufsize - 1 ? "" : ",");
    }
    printf("\n");
}

uint16_t tud_hid_get_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t *buffer,
    uint16_t reqlen
) {
    printf("tud_hid_get_report_cb:\n");
    printf("instance=%u\n", instance);
    printf("report_id=%u\n", report_id);
    printf("report_type=%u\n", report_type);
    printf("reqlen=" PRIu16 "\n\n", reqlen);

    return 0;
}

