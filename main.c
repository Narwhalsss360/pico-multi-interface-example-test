#include <pico/assert.h>
#include <pico/stdio.h>
#include <stdio.h>
#include <tusb.h>
#include <bsp/board_api.h>

int main() {
    board_init();
    tusb_rhport_init_t dev_init = {.role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO};
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    if (board_init_after_tusb) {
        board_init_after_tusb();
    }
    stdio_init_all();

    while (true) {
        void loop(void);
        tud_task();
        loop();
    }
    hard_assert(false);
    return 0;
}

#define ALIVE_INTERVAL (4000)

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

