/* picokbd.c - Local USB keyboard handling */
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#include <pico.h>
#include <pico/stdlib.h>
#include <bsp/board.h>
#include <tusb.h>
#include <class/hid/hid.h>
#include <stdio.h>
#include <pfs_private.h>
#include <pfs_dev_gio.h>
#include <pfs_dev_kbd.h>
#include <pfs_dev_keymap.h>

#ifndef KBD_VERSION
#if TUSB_VERSION_MAJOR == 0
#if TUSB_VERSION_MINOR == 12
#define KBD_VERSION     3
#elif (TUSB_VERSION_MINOR == 14) | (TUSB_VERSION_MINOR == 15)
#define KBD_VERSION     4
#endif  // TUSB_VERSION_MINOR
#endif  // TUSB_VERSION_MAJOR
#endif  // KBD_VERSION
#ifndef KBD_VERSION
#error Unknown USB Version for keyboard
#endif  // KBD_VERSION

#ifndef STATIC
#define STATIC  static
#endif

STATIC bool bRepeat;
STATIC uint8_t led_flags = 0;
STATIC struct pfs_device *gio_kbd = NULL;
STATIC PFS_DEV_KEYMAP *keymap = NULL;
STATIC struct pfs_file *(*kbd_gio_open) (const struct pfs_device *dev, const char *name, int oflags) = NULL;
STATIC int (*kbd_gio_ioctl) (struct pfs_file *fd, unsigned long request, void *argp) = NULL;
STATIC struct pfs_v_file kbd_entry;

#ifndef USE_ASYNC_CONTEXT
#define USE_ASYNC_CONTEXT   1
#endif

#ifndef DEBUG
#define DEBUG               0
#endif

#if USE_ASYNC_CONTEXT
#include <pico/async_context_threadsafe_background.h>
STATIC void kbd_work (async_context_t *context, struct async_work_on_timeout *timeout);
STATIC async_context_threadsafe_background_t    asyc;
STATIC async_at_time_worker_t                   asyw = { .do_work = kbd_work };
#endif

STATIC void set_leds (char leds)
    {
    uint8_t const addr = 1;
    led_flags = (uint8_t) leds;
#if DEBUG > 0
    printf ("set_leds (%02X)\n", leds);
#endif
    static tusb_control_request_t ledreq = {
        .bmRequestType_bit.recipient = TUSB_REQ_RCPT_INTERFACE,
        .bmRequestType_bit.type = TUSB_REQ_TYPE_CLASS,
        .bmRequestType_bit.direction = TUSB_DIR_OUT,
        .bRequest = HID_REQ_CONTROL_SET_REPORT,
        .wValue = HID_REPORT_TYPE_OUTPUT << 8,
        .wIndex = 0,    // Interface number
        .wLength = sizeof (led_flags)
        };

#if KBD_VERSION == 3
    bool bRes = tuh_control_xfer (addr, &ledreq, &led_flags, NULL);
#elif KBD_VERSION == 4
    tuh_xfer_t ledxfer = {
        .daddr = addr,
        .setup = &ledreq,
        .buffer = &led_flags,
        .complete_cb = NULL
        };
    bool bRes = tuh_control_xfer (&ledxfer);
#endif
    }

// Encode key codes and press / release status
// The encoding is designed to express common events as a single byte
// Less common keys are encoded as two distinct bytes.
// Key codes less than 0x58 are sent as-is
// The eight modifier keys are sent as 0x58 to 0x5F
// Remaining keys are sent as 0x70 + 4msb followed by 0x60 + 4 lsb
// Key release is indicated by high bit set (on both bytes if two)
STATIC void key_encode (uint8_t key, bool bPress)
    {
    if ( key < 0x58 )
        {
        if ( ! bPress ) key |= 0x80;
        pfs_dev_gio_input (gio_kbd, key);
        }
    else if (( key >= HID_KEY_CONTROL_LEFT ) && ( key <= HID_KEY_GUI_RIGHT ))
        {
        key -= HID_KEY_CONTROL_LEFT - 0x58;
        if ( ! bPress ) key |= 0x80;
        pfs_dev_gio_input (gio_kbd, key);
        }
    else
        {
        uint8_t keyh = 0x70 | ( key >> 4 );
        key = ( key & 0x0F ) | 0x60;
        if ( ! bPress )
            {
            keyh |= 0x80;
            key |= 0x80;
            }
        pfs_dev_gio_input (gio_kbd, keyh);
        pfs_dev_gio_input (gio_kbd, key);
        }
    }

STATIC void key_press (uint8_t modifier, uint8_t key)
    {
#if DEBUG > 0
    printf ("key_press (0x%02X), keymap = %p\n", key, keymap);
#endif
    if ( keymap != NULL )
        {
        if ( key == HID_KEY_CAPS_LOCK ) led_flags ^= KEYBOARD_LED_CAPSLOCK;
        if ( key == HID_KEY_SCROLL_LOCK ) led_flags ^= KEYBOARD_LED_SCROLLLOCK;
        if ( key == HID_KEY_NUM_LOCK ) led_flags ^= KEYBOARD_LED_NUMLOCK;
#if DEBUG > 0
        printf ("leds = %02X, led_flags = %02X\n", leds, led_flags);
#endif
        if ( key >= keymap->nkey ) return;
        if ( modifier & ( KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT ) )
            key = keymap->key[key].upper;
        else
            key = keymap->key[key].lower;
        if ( key == 0 ) return;
        if (( led_flags & KEYBOARD_LED_CAPSLOCK ) && ( key >= 'a' ) && ( key <= 'z' )) key -= 0x20;
#if EXTEND_ASCII
        if (( led_flags & KEYBOARD_LED_NUMLOCK ) && ( key >= 0x9E ) && ( key <= 0xA9 )) key -= 0x70;
#endif
        if ( modifier & ( KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_RIGHTCTRL ) ) key &= 0x1F;
        pfs_dev_gio_input (gio_kbd, key);
        }
    else
        {
        key_encode (key, true);
        }
    }

STATIC void key_release (uint8_t key)
    {
#if DEBUG > 0
    printf ("key_release (0x%02X)\n", key);
#endif
    if ( keymap == NULL ) key_encode (key, false);
    }

// look up new key in previous keys
STATIC inline int find_key_in_report(hid_keyboard_report_t const *p_report, uint8_t keycode)
    {
    for (int i = 0; i < 6; i++)
        {
        if (p_report->keycode[i] == keycode) return i;
        }
    return -1;
    }

STATIC inline void process_kbd_report(hid_keyboard_report_t const *p_new_report)
    {
    STATIC hid_keyboard_report_t prev_report =
        {
        .modifier = 0,
        .keycode = { 0, 0, 0, 0, 0, 0 }
        };
    for (int i = 0; i < 6; ++i)
        {
        uint8_t key = p_new_report->keycode[i];
        if ( key )
            {
#if DEBUG == 1
            printf ("Key %d reported.\n", key);
#endif
            int kp = find_key_in_report(&prev_report, key);
            if ( kp < 0 )
                {
                key_press (p_new_report->modifier, key);
                bRepeat = true;
                }
            else
                {
                prev_report.keycode[kp] = 0;
                }
            }
        }
    int new_mod = p_new_report->modifier;
    int old_mod = prev_report.modifier;
    int bit = 0x01;
    for (int i = 0; i < 8; ++i)
        {
        if ((new_mod & bit) && !(old_mod & bit))
            {
            key_press (p_new_report->modifier, HID_KEY_CONTROL_LEFT + i);
            bRepeat = true;
            }
        bit <<= 1;
        }
    bit = 0x01;
    for (int i = 0; i < 8; ++i)
        {
        if (!(new_mod & bit) && (old_mod & bit))
            {
            key_release (HID_KEY_CONTROL_LEFT + i);
            bRepeat = true;
            }
        bit <<= 1;
        }
    for (int i = 0; i < 6; ++i)
        {
        uint8_t key = prev_report.keycode[i];
        if ( key )
            {
            key_release (key);
            bRepeat = true;
            }
        }
    prev_report = *p_new_report;
    }

#if (KBD_VERSION == 3) || (KBD_VERSION == 4)

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Each HID instance can has multiple reports
#define MAX_REPORT  4
STATIC struct
    {
    uint8_t report_count;
    tuh_hid_report_info_t report_info[MAX_REPORT];
    } hid_info[CFG_TUH_HID];

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
    {
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
#if DEBUG > 0
    printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);

    // Interface protocol (hid_interface_protocol_enum_t)
    const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
    printf("HID Interface Protocol = %s\r\n", protocol_str[itf_protocol]);
#endif

    // By default host stack will use activate boot protocol on supported interface.
    // Therefore for this simple example, we only need to parse generic report descriptor
    // (with built-in parser)
    if ( itf_protocol == HID_ITF_PROTOCOL_NONE )
        {
        hid_info[instance].report_count = tuh_hid_parse_report_descriptor(hid_info[instance].report_info,
            MAX_REPORT, desc_report, desc_len);
#if DEBUG > 0
        printf("HID has %u reports \r\n", hid_info[instance].report_count);
#endif
        }

    // request to receive report
    // tuh_hid_report_received_cb() will be invoked when report is available
    if ( !tuh_hid_receive_report(dev_addr, instance) )
        {
#if DEBUG > 0
        printf("Error: cannot request to receive report\r\n");
#endif
        }
    }

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
    {
#if DEBUG > 0
    printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
#endif
    }

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
    {
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD)
        {
#if DEBUG > 1
        printf("HID receive boot keyboard report\r\n");
#endif
        process_kbd_report( (hid_keyboard_report_t const*) report );
        }

    // continue to request to receive report
    if ( !tuh_hid_receive_report(dev_addr, instance) )
        {
#if DEBUG > 0
        printf("Error: cannot request to receive report\r\n");
#endif
        }
    }
#else
#error Unknown TinyUSB Version
#endif  // KBD_VERSION

#if USE_ASYNC_CONTEXT

STATIC void kbd_work (async_context_t *context, struct async_work_on_timeout *timeout)
    {
    uint8_t leds = led_flags;
#if DEBUG > 1
    printf ("-");
#endif
    bRepeat = true;
    while ( bRepeat )
        {
        bRepeat = false;
        tuh_task();
        }
    if ( ! async_context_add_at_time_worker_in_ms ((async_context_t *) &asyc, &asyw, 100) )
        {
#if DEBUG > 0
        printf ("Failed to add kbd_worker task\n");
#endif
        }
    if ( led_flags != leds) set_leds (led_flags);
    }

#else

STATIC struct repeating_timer s_kbd_timer;
STATIC bool keyboard_periodic (struct repeating_timer *prt)
    {
    uint8_t leds = led_flags;
#if DEBUG > 1
    printf ("-");
#endif
    bRepeat = true;
    while ( bRepeat )
        {
        bRepeat = false;
        tuh_task();
        }
    if ( led_flags != leds) set_leds (led_flags);
    return true;
    }
#endif

STATIC int kbd_ioctl (struct pfs_file *fd, unsigned long request, void *argp)
    {
    if ( request == IOC_RQ_KEYMAP )
        {
        keymap = (PFS_DEV_KEYMAP *) argp;
        return 0;
        }
    return kbd_gio_ioctl (fd, request, argp);
    }

STATIC struct pfs_file *kbd_open (const struct pfs_device *dev, const char *name, int oflags)
    {
    struct pfs_file *fd = kbd_gio_open (dev, name, oflags);
    if ( fd != NULL )
        {
        memcpy (&kbd_entry, fd->entry, sizeof (struct pfs_v_file));
        kbd_gio_ioctl = kbd_entry.ioctl;
        kbd_entry.ioctl = kbd_ioctl;
        fd->entry = &kbd_entry;
        }
    return fd;
    }

struct pfs_device *pfs_dev_kbd_fetch (PFS_DEV_KEYMAP *km)
    {
    if ( gio_kbd == NULL )
        {
        gio_kbd = pfs_dev_gio_create (set_leds, 128, IOC_MD_ANY);
        keymap = km;
        if ( gio_kbd != NULL )
            {
            kbd_gio_open = gio_kbd->open;
            gio_kbd->open = kbd_open;
#if DEBUG > 0
            printf ("setup_keyboard " __DATE__ " " __TIME__ "\n");
#endif
            tusb_init();
#if USE_ASYNC_CONTEXT
            if ( async_context_threadsafe_background_init_with_defaults (&asyc) )
                {
                if ( ! async_context_add_at_time_worker_in_ms ((async_context_t *) &asyc, &asyw, 100) )
                    {
#if DEBUG > 0
                    printf ("Failed to add kbd_worker task\n");
#endif
                    }
                }
            else
                {
                printf ("Failed to initialise async context\n");
                }
#else
            add_repeating_timer_ms (100, keyboard_periodic, NULL, &s_kbd_timer);
#endif
            }
        }
    return gio_kbd;
    }
