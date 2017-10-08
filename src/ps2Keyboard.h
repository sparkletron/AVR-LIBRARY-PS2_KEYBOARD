/**
 * \brief Driver to interface with PS2 based keyboard
 *
 * \author John Convertino (electrobs@gmail.com)
 * \date   8/28/2017
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
 * \version 1.0
 */

#ifndef ps2Keyboard
#define ps2Keyboard

#include <inttypes.h>
#include "ps2defines.h"

#ifndef PS2PORTBIRQ_H_
	#ifndef PS2PORTCIRQ_H_
		#ifndef PS2PORTDIRQ_H_
			#warning "ps2PORT B, C, or D irq.h must be included to use this library, and before the ps2Keyboard header to remove this warning."
		#endif
	#endif
#endif

/**
 * \brief initialize PS2 keyboard
 *
 * \param p_port Gets the address of a port to be used for the clk and data pin.
 * \param clkPin Define which pin used for the clock
 * \param dataPin Define the pin used for data.
 */
void initPS2keyboard(t_PS2userRecvCallback PS2recvCallback, volatile uint8_t *p_port, uint8_t clkPin, uint8_t dataPin);

/**
 * \brief Convert PS2 keyboard define representation
 * to a character.
 *
 * \param ps2data PS2 keyboard data in a the form of a define from the scan code lookup table.
 *
 * \return Return a ASCII character
 */
char defineToChar(uint8_t ps2data);

/**
 * \brief Updates LEDS on keyboard, must be called in for loop, can NOT
 * be called by the user callback, program will hang.
 */
void updatePS2leds();

/**
 * \brief Get keybreak state.
 *
 * \return 0 for no keybreak, 1 for keybreak on last press.
 */
uint8_t getKeybreakState();

/**
 * \brief Get ID from keyboard
 *
 * \return 16 bit id of keyboard.
 */
uint16_t getKeyboardID();

/**
 * \brief Get Caps lock state.
 *
 * \return 1 for on, 0 for off.
 */
uint8_t getCapsLockState();

/**
 * \brief Get Num lock state.
 *
 * \return 1 for on, 0 for off.
 */
uint8_t getNumLockState();

/**
 * \brief Get Scroll Lock state.
 *
 * \return 1 for on, 0 for off.
 */
uint8_t getScrollLockState();

/**
 * \brief Resend PS2 keyboards last byte to the host.
 */
void resendPS2lastByte();

/**
 * \brief Reset PS2 keyboard.
 */
void resetPS2keyboard();

/**
 * \brief Disable PS2 keyboard.
 */
void disablePS2keyboard();

/**
 * \brief Enable PS2 keyboard.
 */
void enablePS2keyaboard();

/**
 * \brief Set default rates for keyboard.
 */
void setPS2default();

/**
 * \brief Send Typmatic command before sendind typmatic data
 */
void sendPS2typmaticRateDelayCMD();

/**
 * \brief Set PS2 typmatic parameters, if the number is invalid defaults are used
 *
 * \param delay set delay to a predefined amount, this can be 0 to 3.
 *      0 = 250ms
 *      1 = 500ms
 *      2 = 750ms
 *      3 = 1000ms
 * \param rate set the rate to predefined amount. Between 0 to 0x1F
 *      Theses are in cycles per second (hz) from 0x00 at 30 hz to
 *      0x1F at 2 hz. A table is available at:
 *      http://www.computer-engineering.org/ps2keyboard/
 */
void setPS2typmaticRateDelay(uint8_t delay, uint8_t rate);

/**
 * \brief Send PS2 command to keyboard to read the ID.
 */
void sendPS2readIDcmd();

#endif
