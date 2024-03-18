/*******************************************************************************
 * @file    psKeyboard.h
 * @author  Jay Convertino(electrobs@gmail.com)
 * @date    2024.03.12
 * @brief   ps2 keyboard driver
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

#ifndef _ps2Keyboard
#define _ps2Keyboard

#include <inttypes.h>
#include "ps2base.h"
#include "ps2keyboardDefines.h"

/**
 * \brief initialize PS2 keyboard
 *
 * \param PS2recvCallback Callback to a user supplied function to parse keyboard data.
 * \param setPS2_PORT_device A function pointer to a IRQ port data setter.
 * \param p_port Gets the address of a port to be used for the clk and data pin.
 * \param clkPin Define which pin used for the clock
 * \param dataPin Define the pin used for data.
 */
void initPS2keyboard(t_PS2userRecvCallback PS2recvCallback, void (*setPS2_PORT_Device)(struct s_ps2 *p_device), volatile uint8_t *p_port, uint8_t clkPin, uint8_t dataPin);

/**
 * \brief Convert PS2 keyboard define representation
 * to a character.
 *
 * \param ps2data PS2 keyboard data in a the form of a define from the scan code lookup table.
 *
 * \return Return a ASCII character
 */
char PS2defineToChar(uint8_t ps2data);

/**
 * \brief Updates LEDS on keyboard, must be called in for loop, can NOT
 * be called by the user callback, program will hang.
 */
void updatePS2leds();

/**
 * \brief Is the current code a character with keybreak?
 *
 * \return 1 the current key is released, 0 is no release
 */
uint8_t getPS2keyReleased();

/**
 * \brief Get ID from keyboard
 *
 * \return 16 bit id of keyboard.
 */
uint16_t getPS2keyboardID();

/**
 * \brief Get Caps lock state.
 *
 * \return 1 for on, 0 for off.
 */
uint8_t getPS2capsLockState();

/**
 * \brief Get Num lock state.
 *
 * \return 1 for on, 0 for off.
 */
uint8_t getPS2numLockState();

/**
 * \brief Get Scroll Lock state.
 *
 * \return 1 for on, 0 for off.
 */
uint8_t getPS2scrollLockState();

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
