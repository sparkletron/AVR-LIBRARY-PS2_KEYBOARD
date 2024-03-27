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
 * \param p_ps2keyboard struct containing keyboard instance information
 * \param PS2recvCallback Callback to a user supplied function to parse keyboard data.
 * \param setPS2_PORT_device A function pointer to a IRQ port data setter.
 * \param p_port Gets the address of a port to be used for the clk and data pin.
 * \param clkPin Define which pin used for the clock
 * \param dataPin Define the pin used for data.
 */
void initPS2keyboard(struct s_ps2 *p_ps2keyboard, t_PS2userRecvCallback PS2recvCallback, void (*setPS2_PORT_Device)(struct s_ps2 *p_device), volatile uint8_t *p_port, uint8_t clkPin, uint8_t dataPin);

/**
 * \brief Convert PS2 keyboard define representation
 * to a character.
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 * \param ps2data PS2 keyboard data in a the form of a define from the scan code lookup table.
 *
 * \return Return a ASCII character
 */
char PS2defineToChar(struct s_ps2 *p_ps2keyboard, uint8_t ps2data);

/**
 * \brief Updates LEDS on keyboard, must be called in for loop, can NOT
 * be called by the user callback, program will hang.
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 */
void updatePS2leds(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Is the current code a character with keybreak?
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 *
 * \return 1 the current key is released, 0 is no release
 */
uint8_t getPS2keyReleased(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Get ID from keyboard
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 *
 * \return 16 bit id of keyboard.
 */
uint16_t getPS2keyboardID(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Get Caps lock state.
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 *
 * \return 1 for on, 0 for off.
 */
uint8_t getPS2capsLockState(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Get Num lock state.
 *
 * \return 1 for on, 0 for off.
 */
uint8_t getPS2numLockState(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Get Scroll Lock state.
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 *
 * \return 1 for on, 0 for off.
 */
uint8_t getPS2scrollLockState(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Resend PS2 keyboards last byte to the host.
 */
void resendPS2lastByte(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Reset PS2 keyboard.
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 */
void resetPS2keyboard(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Disable PS2 keyboard.
 */
void disablePS2keyboard(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Enable PS2 keyboard.
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 */
void enablePS2keyaboard(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Set default rates for keyboard.
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 */
void setPS2default(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Send Typmatic command before sendind typmatic data
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 */
void sendPS2typmaticRateDelayCMD(struct s_ps2 *p_ps2keyboard);

/**
 * \brief Set PS2 typmatic parameters, if the number is invalid defaults are used
 *
 * \param p_ps2keyboard struct containing keyboard instance information
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
void setPS2typmaticRateDelay(struct s_ps2 *p_ps2keyboard, uint8_t delay, uint8_t rate);

/**
 * \brief Send PS2 command to keyboard to read the ID.
 *
 * \param p_ps2keyboard struct containing keyboard instance information
 */
void sendPS2readIDcmd(struct s_ps2 *p_ps2keyboard);

#endif
