/*
 * AVR328P-LIBRARY-PS2_DRIVER
 *
 *  Created on: September 28, 2017
 *      Author: John Convertino
 *			email: electrobs@gmail.com
 *
    Copyright (C) 2017 John Convertino

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *		Version: v1.0
 *		 September 28, 2017 v1.0 first release version
 */


#ifndef PS2_DEFINES
#define PS2_DEFINES

typedef void (*t_PS2recvCallback)(uint16_t recvBuffer);
typedef void (*t_PS2userRecvCallback)(uint8_t recvBuffer);
enum callbackStates {done, waiting, error, resend, noack};

//bit defines
#define MAX_BIT_COUNT    64
#define BIT_COUNT        8
#define MAX_REPEAT_RATE  0x1F
#define MAX_DELAY        0x03
#define DEFAULT_RATE     0x0B
#define DEFAULT_DELAY    0x01

//bool values
#define RECV_MODE   0
#define SEND_MODE   1
#define NKEYBREAK   0
#define KEYBREAK    1
#define ID_NRECV    0
#define ID_RECV     1

//bit stuff
#define DELAY_BITS_POS  5
#define UINT8_T_SIZE    8
#define PARITY_BIT_POS  9
#define DATA_BIT0_POS   1
#define STOP_BIT_POS    10
#define STOP_BIT_VALUE  1
#define SEND_BUFF_SIZE  12
#define RECV_BUFF_SIZE  11

//keyboard commands
#define CMD_RESET       0xFF
#define CMD_RESEND      0xFE
#define CMD_DEFAULT     0xF6
#define CMD_DISABLE     0xF5
#define CMD_ENABLE      0xF4
#define CMD_SET_RATE    0xF3
#define CMD_READ_ID     0xF2
#define CMD_SET_LED     0xED
#define CMD_ACK         0xFA
#define CMD_KEY_RDY     0xAA

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
