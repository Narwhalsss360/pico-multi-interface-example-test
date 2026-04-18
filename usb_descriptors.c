#include <stdint.h>
#include <bsp/board_api.h>
#include <tusb.h>
#include "tusb_config.h"
#include <common/tusb_types.h>
#include <device/usbd.h>
#include <class/hid/hid_device.h>

#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define THIS_VID (0x2E8A) // Raspberry Pi
#define THIS_PID (0x4000 | (CFG_TUD_HID << 1) | (CFG_TUD_CDC << 2))
#define DEVICE_VERSOIN_BCD (0x0100)
#define USB_VERSION_BCD (0x0200)
#define HID_INTERFACE_POLLING_INTERVAL (2)

const tusb_desc_device_t device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = USB_VERSION_BCD,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = THIS_VID,
    .idProduct = THIS_PID,
    .bcdDevice = DEVICE_VERSOIN_BCD,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1
};

enum {
    INTERFACE_NUMBER_CDC_0,
    INTERFACE_NUMBER_CDC_0_DATA,
    INTERFACE_NUMBER_HID,
    INTERFACE_COUNT
};

enum {
    ENDPOINT_NUMBER_CDC_0_NOTIFICATION = 0x81,
    ENDPOINT_NUMBER_CDC_0_IN = 0x82,
    ENDPOINT_NUMBER_CDC_0_OUT = 0x02,
    ENDPOINT_NUMBER_HID = 0x83
};

enum {
    STRING_INDEX_LANGID = 0,
    STRING_INDEX_MANUFACTURER,
    STRING_INDEX_PRODUCT,
    STRING_INDEX_SERIAL,
    STRING_INDEX_CDC_0,
    STRING_INDEX_HID,
};

const char* const descriptor_strings[] = {
    (const char[]) { 0x09, 0x04 },
    "Raspberry Pi",
    "Pico (CDC and HID)",
    NULL, // Will get during runtime...
    "Pico stdio",
    "Pico HID",
    "RPiReset"
};

const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(
        HID_REPORT_ID_KEYBOARD
    )),
    TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(
        HID_REPORT_ID_GAMEPAD
    ))
};

#define CONFIGURATION_DESCRIPTOR_LENGTH (TUD_CONFIG_DESC_LEN + (CFG_TUD_CDC * TUD_CDC_DESC_LEN) + (CFG_TUD_HID * TUD_HID_DESC_LEN))

const uint8_t configuration_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(
        1,
        INTERFACE_COUNT,
        0,
        CONFIGURATION_DESCRIPTOR_LENGTH,
        0x80,
        100
    ),
    TUD_CDC_DESCRIPTOR(
        INTERFACE_NUMBER_CDC_0,
        STRING_INDEX_CDC_0,
        ENDPOINT_NUMBER_CDC_0_NOTIFICATION,
        8,
        ENDPOINT_NUMBER_CDC_0_OUT,
        ENDPOINT_NUMBER_CDC_0_IN,
        CDC_0_IN_BUFFER_LENGTH
    ),
    TUD_HID_DESCRIPTOR(
        INTERFACE_NUMBER_HID,
        STRING_INDEX_HID,
        HID_ITF_PROTOCOL_NONE,
        sizeof(hid_report_descriptor),
        ENDPOINT_NUMBER_HID,
        CFG_TUD_HID_EP_BUFSIZE,
        HID_INTERFACE_POLLING_INTERVAL
    )
};

const uint8_t* tud_descriptor_device_cb() {
    return (const uint8_t*)&device_descriptor;
}

uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
  (void)instance;
  return hid_report_descriptor;
}

const uint8_t* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return configuration_descriptor;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t utf16_buffer[32 + 1];

    size_t char_count;

    switch (index) {
        case STRING_INDEX_LANGID: {
            memcpy(&utf16_buffer[1], descriptor_strings[STRING_INDEX_LANGID], 2);
            char_count = 1;
            break;
        }
        case STRING_INDEX_SERIAL: {
            // try to read the serial from the board
            char_count = board_usb_get_serial(utf16_buffer + 1, 32);
            break;
        }
        default: {
            // COPYRIGHT NOTE: Based on TinyUSB example
            // Windows wants utf16le

            // Determine which string descriptor to return
            if ( !(index < sizeof(descriptor_strings) / sizeof(descriptor_strings[0])) ) {
                return NULL;
            }

            // Copy string descriptor into _desc_str
            const char *str = descriptor_strings[index];

            char_count = strlen(str);
            size_t const max_count = sizeof(utf16_buffer) / sizeof(utf16_buffer[0]) - 1; // -1 for string type
            // Cap at max char
            if (char_count > max_count) {
                char_count = max_count;
            }

            // Convert ASCII string into UTF-16
            for (size_t i = 0; i < char_count; i++) {
                utf16_buffer[1 + i] = str[i];
            }
            break;
        }
    }

    // First byte is the length (including header), second byte is string type
    utf16_buffer[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (char_count * 2 + 2));

    return utf16_buffer;
}
