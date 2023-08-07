// pfs_dev_keymap_uk.c - Definition of UK keyboard mapping
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#include <pfs_dev_keymap.h>

#if EXTEND_ASCII
#define _X(x)   x
#define _NL(x)  (x + 0x70)
#else
#define _X(x)   0x00
#define _NL(x)  x
#endif

const PFS_DEV_KEYMAP pfs_dev_keymap_uk =
    {
    0x68,                           // Number of key mappings
        {
        { 0x00, 0x00 },             // 0x00 - No key pressed
        { 0x00, 0x00 },             // 0x01 - Keyboard Error Roll Over - used for all slots if too many keys are pressed ("Phantom key")
        { 0x00, 0x00 },             // 0x02 - Keyboard POST Fail
        { 0x00, 0x00 },             // 0x03 - Keyboard Error Undefined
        { 'a', 'A' },               // 0x04 - Keyboard a and A
        { 'b', 'B' },               // 0x05 - Keyboard b and B
        { 'c', 'C' },               // 0x06 - Keyboard c and C
        { 'd', 'D' },               // 0x07 - Keyboard d and D
        { 'e', 'E' },               // 0x08 - Keyboard e and E
        { 'f', 'F' },               // 0x09 - Keyboard f and F
        { 'g', 'G' },               // 0x0a - Keyboard g and G
        { 'h', 'H' },               // 0x0b - Keyboard h and H
        { 'i', 'I' },               // 0x0c - Keyboard i and I
        { 'j', 'J' },               // 0x0d - Keyboard j and J
        { 'k', 'K' },               // 0x0e - Keyboard k and K
        { 'l', 'L' },               // 0x0f - Keyboard l and L
        { 'm', 'M' },               // 0x10 - Keyboard m and M
        { 'n', 'N' },               // 0x11 - Keyboard n and N
        { 'o', 'O' },               // 0x12 - Keyboard o and O
        { 'p', 'P' },               // 0x13 - Keyboard p and P
        { 'q', 'Q' },               // 0x14 - Keyboard q and Q
        { 'r', 'R' },               // 0x15 - Keyboard r and R
        { 's', 'S' },               // 0x16 - Keyboard s and S
        { 't', 'T' },               // 0x17 - Keyboard t and T
        { 'u', 'U' },               // 0x18 - Keyboard u and U
        { 'v', 'V' },               // 0x19 - Keyboard v and V
        { 'w', 'W' },               // 0x1a - Keyboard w and W
        { 'x', 'X' },               // 0x1b - Keyboard x and X
        { 'y', 'Y' },               // 0x1c - Keyboard y and Y
        { 'z', 'Z' },               // 0x1d - Keyboard z and Z
        { '1', '!' },               // 0x1e - Keyboard 1 and !
        { '2', '"' },               // 0x1f - Keyboard 2 and @
        { '3', '#' },               // 0x20 - Keyboard 3 and #
        { '4', '$' },               // 0x21 - Keyboard 4 and $
        { '5', '%' },               // 0x22 - Keyboard 5 and %
        { '6', '^' },               // 0x23 - Keyboard 6 and ^
        { '7', '&' },               // 0x24 - Keyboard 7 and &
        { '8', '*' },               // 0x25 - Keyboard 8 and *
        { '9', '(' },               // 0x26 - Keyboard 9 and (
        { '0', ')' },               // 0x27 - Keyboard 0 and )
        { '\r', '\r' },             // 0x28 - Keyboard Return (ENTER)
        { 0x1B, 0x1B },             // 0x29 - Keyboard ESCAPE
        { 0x08, 0x08 },             // 0x2a - Keyboard DELETE (Backspace)
        { 0x09, 0x09 },             // 0x2b - Keyboard Tab
        { ' ', ' ' },               // 0x2c - Keyboard Spacebar
        { '-', '_' },               // 0x2d - Keyboard - and _
        { '=', '+' },               // 0x2e - Keyboard = and +
        { '[', '{' },               // 0x2f - Keyboard [ and {
        { ']', '}' },               // 0x30 - Keyboard ] and }
        { '\\', '|' },              // 0x31 - Keyboard \ and |
        { '#', '~' },               // 0x32 - Keyboard Non-US # and ~
        { ';', ':' },               // 0x33 - Keyboard ; and :
        { '\'', '@' },              // 0x34 - Keyboard ' and "
        { '`', 0x00 },              // 0x35 - Keyboard ` and ~
        { ',', '<' },               // 0x36 - Keyboard , and <
        { '.', '>' },               // 0x37 - Keyboard . and >
        { '/', '?' },               // 0x38 - Keyboard / and ?
        { _X(0xC1), _X(0xD1) },     // 0x39 - Keyboard Caps Lock
        { _X(0x81), _X(0x91) },     // 0x3a - Keyboard F1
        { _X(0x82), _X(0x92) },     // 0x3b - Keyboard F2
        { _X(0x83), _X(0x93) },     // 0x3c - Keyboard F3
        { _X(0x84), _X(0x94) },     // 0x3d - Keyboard F4
        { _X(0x85), _X(0x95) },     // 0x3e - Keyboard F5
        { _X(0x86), _X(0x96) },     // 0x3f - Keyboard F6
        { _X(0x87), _X(0x97) },     // 0x40 - Keyboard F7
        { _X(0x88), _X(0x98) },     // 0x41 - Keyboard F8
        { _X(0x89), _X(0x99) },     // 0x42 - Keyboard F9
        { _X(0x8A), _X(0x9A) },     // 0x43 - Keyboard F10
        { _X(0x8B), _X(0x9B) },     // 0x44 - Keyboard F11
        { _X(0x8C), _X(0x9C) },     // 0x45 - Keyboard F12
        { _X(0xC3), _X(0xD3) },     // 0x46 - Keyboard Print Screen
        { _X(0xC2), _X(0xD2) },     // 0x47 - Keyboard Scroll Lock
        { _X(0xC4), _X(0xD4) },     // 0x48 - Keyboard Pause
        { _X(0xC5), _X(0xD5) },     // 0x49 - Keyboard Insert
        { _X(0xE7), _X(0xF7) },     // 0x4a - Keyboard Home
        { _X(0xE9), _X(0xF9) },     // 0x4b - Keyboard Page Up
        { 0x7F, 0x7F },             // 0x4c - Keyboard Delete Forward
        { _X(0xE1), _X(0xF1) },     // 0x4d - Keyboard End
        { _X(0xE3), _X(0xF3) },     // 0x4e - Keyboard Page Down
        { _X(0xE6), _X(0xF6) },     // 0x4f - Keyboard Right Arrow
        { _X(0xE4), _X(0xF4) },     // 0x50 - Keyboard Left Arrow
        { _X(0xE2), _X(0xF2) },     // 0x51 - Keyboard Down Arrow
        { _X(0xE8), _X(0xF8) },     // 0x52 - Keyboard Up Arrow
        { _X(0xC0), _X(0xD0) },     // 0x53 - Keyboard Num Lock and Clear
        { '/', '/' },               // 0x54 - Keypad /
        { '*', '*' },               // 0x55 - Keypad *
        { '-', '-' },               // 0x56 - Keypad -
        { '+', '+' },               // 0x57 - Keypad +
        { '\r', '\r' },             // 0x58 - Keypad ENTER
        { _NL('1'), '1' },          // 0x59 - Keypad 1 and End
        { _NL('2'), '2' },          // 0x5a - Keypad 2 and Down Arrow
        { _NL('3'), '3' },          // 0x5b - Keypad 3 and PageDn
        { _NL('4'), '4' },          // 0x5c - Keypad 4 and Left Arrow
        { _NL('5'), '5' },          // 0x5d - Keypad 5
        { _NL('6'), '6' },          // 0x5e - Keypad 6 and Right Arrow
        { _NL('7'), '7' },          // 0x5f - Keypad 7 and Home
        { _NL('8'), '8' },          // 0x60 - Keypad 8 and Up Arrow
        { _NL('9'), '9' },          // 0x61 - Keypad 9 and Page Up
        { _NL('0'), '0' },          // 0x62 - Keypad 0 and Insert
        { _NL('.'), '.' },          // 0x63 - Keypad . and Delete
        { '\\', '|' },              // 0x64 - Keyboard Non-US \ and |
        { _X(0xC6), _X(0xD6) },     // 0x65 - Keyboard Application
        { _X(0xC7), _X(0xD7) },     // 0x66 - Keyboard Power
        { '=', '=' }                // 0x67 - Keypad =
        }
    };
