/*******************************************************************************
 * @file    ps2keyboardDefines.h
 * @author  Jay Convertino(electrobs@gmail.com)
 * @date    2024.03.12
 * @brief   defines for PS2
 * @version 0.0.0
 *
 * @TODO
 *  - Cleanup interface
 *
 * @license mit
 *
 * Copyright 2024 Johnathan Convertino
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 ******************************************************************************/


#ifndef _PS2_KEYBOARD_DEFINES
#define _PS2_KEYBOARD_DEFINES

//bit defines
#define MAX_REPEAT_RATE  0x1F
#define MAX_DELAY        0x03
#define DEFAULT_RATE     0x0B
#define DEFAULT_DELAY    0x01

//keyboard commands
#define CMD_SET_LED     0xED

//keyboard ID
#define KEYBOARD_ID1    0xAB
#define KEYBOARD_ID2    0x83

//keyboard codes as chars
#define KEYCODE_CAPS   1
#define KEYCODE_LSHIFT 2
#define KEYCODE_LCTRL  3
#define KEYCODE_LGUI   4
#define KEYCODE_LALT   5
#define KEYCODE_RSHIFT 6
#define KEYCODE_RCTRL  7
#define KEYCODE_RGUI   10
#define KEYCODE_RALT   11
#define KEYCODE_APPS   12
#define KEYCODE_F1     14
#define KEYCODE_F2     15
#define KEYCODE_F3     16
#define KEYCODE_F4     18
#define KEYCODE_F5     19
#define KEYCODE_F6     20
#define KEYCODE_F7     21
#define KEYCODE_F8     22
#define KEYCODE_F9     23
#define KEYCODE_F10    24
#define KEYCODE_F11    25
#define KEYCODE_F12    26
#define KEYCODE_ESC    27
#define KEYCODE_PRTSCR 28
#define KEYCODE_SCROLL 29
#define KEYCODE_PAUSE  30
#define KEYCODE_INSERT 31
#define KEYCODE_DEL    127
#define KEYCODE_HOME   128
#define KEYCODE_PGUP   129
#define KEYCODE_END    130
#define KEYCODE_PGDW   131
#define KEYCODE_UARROW 132
#define KEYCODE_LARROW 133
#define KEYCODE_DARROW 134
#define KEYCODE_RARROW 135
#define KEYCODE_NUM    136
#define KEYCODE_KPFWSL 137
#define KEYCODE_KPASTR 138
#define KEYCODE_KPMIN  139
#define KEYCODE_KPPLUS 140
#define KEYCODE_KPENT  150
#define KEYCODE_KPDEC  151
#define KEYCODE_KP0    152
#define KEYCODE_KP1    152
#define KEYCODE_KP2    153
#define KEYCODE_KP3    154
#define KEYCODE_KP4    155
#define KEYCODE_KP5    156
#define KEYCODE_KP6    157
#define KEYCODE_KP7    158
#define KEYCODE_KP8    159
#define KEYCODE_KP9    160

#endif
