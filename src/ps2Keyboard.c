/*******************************************************************************
 * @file    psKeyboard.c
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

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <avr/common.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ps2Keyboard.h"
#include "ps2scanCodes.h"

volatile int toggle = 0;

enum keyReleaseStates {no_release, release};

struct s_ps2keyboard
{
  union
  {
    struct
    {
      uint8_t scroll:1;
      uint8_t num:1;
      uint8_t cap:1;
      uint8_t nothing:5;
    } bit;

    uint8_t packet;
  } leds, prevLEDS;

  union
  {
    struct
    {
      uint8_t rate:4;
      uint8_t delay:4;
    } param;

    uint8_t packet;
  } typematic;

  volatile enum keyReleaseStates prevCapRelease;
  volatile enum keyReleaseStates prevNumRelease;
  volatile enum keyReleaseStates prevScrollRelease;

  volatile uint8_t keybreak:1;
  volatile uint8_t idRecv:1;

  uint16_t id;

  volatile enum keyReleaseStates keyReleaseState;
};

//helper functions
//convert scancode to define from scancodes header.
uint8_t convertToDefine(struct s_ps2 *p_ps2keyboard, uint8_t ps2data);
//set internal LED tracking and send LED state to keyboard.
void setPS2leds(struct s_ps2 *p_ps2keyboard, uint8_t caps, uint8_t num, uint8_t scroll);
//callbacks
//get the two byte id
void getID(void *p_data, uint16_t ps2Data);
//check the response to keyboard command sent and perform needed operations
void checkKeyboardResponse(void *p_data, uint16_t ps2Data);
//Default callback for recv that processes data and then hands it off to the user callback.
void extractData(void *p_data, uint16_t ps2data);

void initPS2keyboard(struct s_ps2 *p_ps2keyboard, t_PS2userRecvCallback PS2recvCallback, void (*setPS2_PORT_Device)(struct s_ps2 *p_device), volatile uint8_t *p_port, uint8_t clkPin, uint8_t dataPin)
{
  uint8_t tmpSREG = 0;

  tmpSREG = SREG;
  cli();

  if(p_ps2keyboard == NULL) return;

  if(p_port == NULL) return;

  if(PS2recvCallback == NULL) return;

  if(setPS2_PORT_Device == NULL) return;

  memset(p_ps2keyboard, 0, sizeof(struct s_ps2));

  p_ps2keyboard->p_device = malloc(sizeof(struct s_ps2keyboard));

  memset(p_ps2keyboard->p_device, 0, sizeof(struct s_ps2keyboard));

  p_ps2keyboard->clkPin = clkPin;
  p_ps2keyboard->dataPin = dataPin;
  p_ps2keyboard->p_port = p_port;
  p_ps2keyboard->lastAckState = ack;
  p_ps2keyboard->dataState = idle;
  p_ps2keyboard->userRecvCallback = PS2recvCallback;
  p_ps2keyboard->recvCallback = NULL;
  p_ps2keyboard->responseCallback = &checkKeyboardResponse;
  p_ps2keyboard->callUserCallback = &extractData;

  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->prevCapRelease    = release;
  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->prevNumRelease    = release;
  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->prevScrollRelease = release;

  *(p_ps2keyboard->p_port - 1) &= ~(1 << p_ps2keyboard->clkPin);
  *(p_ps2keyboard->p_port - 1) &= ~(1 << p_ps2keyboard->dataPin);

  if(p_ps2keyboard->p_port == &PORTB)
  {
    PCICR |= 1 << PCIE0;
    PCMSK0 |= 1 << p_ps2keyboard->clkPin;
  }
  else if(p_ps2keyboard->p_port == &PORTC)
  {
    PCICR |= 1 << PCIE1;
    PCMSK1 |= 1 << p_ps2keyboard->clkPin;
  }
  else
  {
    PCICR |= 1 << PCIE2;
    PCMSK2 |= 1 << p_ps2keyboard->clkPin;
  }

  setPS2_PORT_Device(p_ps2keyboard);

  SREG = tmpSREG;

  sei();

  //initialize keyboard using PC init method
  resetPS2keyboard(p_ps2keyboard);

  setPS2leds(p_ps2keyboard, 0, 0, 0);
}

char PS2defineToChar(struct s_ps2 *p_ps2keyboard, uint8_t ps2data)
{
  uint8_t tmpSREG = 0;

  if(p_ps2keyboard == NULL) return '\0';

  tmpSREG = SREG;
  cli();

  int index = 0;

  for(index = 0; e_set2scanCodes[index].defineCode != 0; index++)
  {
    if(e_set2scanCodes[index].defineCode == ps2data)
    {
      if((e_set2scanCodes[index].ascii >= 'a') && (e_set2scanCodes[index].ascii <= 'z'))
      {
        SREG = tmpSREG;

        return (getPS2capsLockState(p_ps2keyboard) ? e_set2scanCodes[index].ascii - 32 : e_set2scanCodes[index].ascii);
      }

      SREG = tmpSREG;

      return e_set2scanCodes[index].ascii;
    }
  }

  SREG = tmpSREG;
  return e_set2scanCodes[index].ascii;
}

void updatePS2leds(struct s_ps2 *p_ps2keyboard)
{
  waitForDataIdle(p_ps2keyboard);

  if(((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->prevLEDS.packet != ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->leds.packet)
  {
    setPS2leds(p_ps2keyboard, getPS2capsLockState(p_ps2keyboard), getPS2numLockState(p_ps2keyboard), getPS2scrollLockState(p_ps2keyboard));
  }

  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->prevLEDS.packet = ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->leds.packet;
}

uint8_t getPS2keyReleased(struct s_ps2 *p_ps2keyboard)
{
  return (((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->keyReleaseState == release);
}

uint16_t getPS2keyboardID(struct s_ps2 *p_ps2keyboard)
{
  return ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->id;
}

uint8_t getPS2capsLockState(struct s_ps2 *p_ps2keyboard)
{
  return ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->leds.bit.cap;
}

uint8_t getPS2numLockState(struct s_ps2 *p_ps2keyboard)
{
  return ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->leds.bit.num;
}

uint8_t getPS2scrollLockState(struct s_ps2 *p_ps2keyboard)
{
  return ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->leds.bit.scroll;
}

void resendPS2lastByte(struct s_ps2 *p_ps2keyboard)
{
  sendCommand(p_ps2keyboard, CMD_RESEND);
}

void resetPS2keyboard(struct s_ps2 *p_ps2keyboard)
{
  sendCommand(p_ps2keyboard, CMD_RESET);

  waitForDevReady(p_ps2keyboard);
}

void disablePS2keyboard(struct s_ps2 *p_ps2keyboard)
{
  sendCommand(p_ps2keyboard, CMD_DISABLE);
}

void enablePS2keyaboard(struct s_ps2 *p_ps2keyboard)
{
  sendCommand(p_ps2keyboard, CMD_ENABLE);
}

void setPS2default(struct s_ps2 *p_ps2keyboard)
{
  sendCommand(p_ps2keyboard, CMD_DEFAULT);
}

//broken? Need to check for ACKs
void setPS2typmaticRateDelay(struct s_ps2 *p_ps2keyboard, uint8_t delay, uint8_t rate)
{
  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->typematic.param.rate = (rate <= MAX_REPEAT_RATE ? rate : DEFAULT_RATE);

  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->typematic.param.delay = (delay <= MAX_DELAY ? delay : DEFAULT_DELAY);

  sendCommand(p_ps2keyboard, CMD_SET_RATE);

  sendData(p_ps2keyboard, ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->typematic.packet);
}

void sendPS2readIDcmd(struct s_ps2 *p_ps2keyboard)
{
  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->id = 0;

  sendCommand(p_ps2keyboard, CMD_READ_ID);

  waitForDevID(p_ps2keyboard);
}

//helper functions
uint8_t convertToDefine(struct s_ps2 *p_ps2keyboard, uint8_t ps2data)
{
  int index = 0;
  static int shift = 0;
  static uint64_t buffer = 0;

  buffer |= (uint64_t)(ps2data << shift);

  for(index = 0; e_set2scanCodes[index].defineCode != 0; index++)
  {
    if(e_set2scanCodes[index].keyCode == buffer)
    {
      buffer = 0;
      shift = 0;
      ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->keyReleaseState = no_release;
      return e_set2scanCodes[index].defineCode;
    }

    if(e_set2scanCodes[index].breakCode == buffer)
    {
      buffer = 0;
      shift = 0;
      ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->keyReleaseState = release;
      return e_set2scanCodes[index].defineCode;
    }
  }

  //shift by a byte for each miss, up to the total number of bytes in a 64 bit int
  shift += sizeof(uint8_t)*8;
  shift %= sizeof(uint64_t)*8;

  //we have wrapped around, reset buffer to 0. this is an error.
  if(!shift)
  {
    buffer = 0;
  }

  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->keyReleaseState = no_release;

  return 0;
}


void setPS2leds(struct s_ps2 *p_ps2keyboard, uint8_t caps, uint8_t num, uint8_t scroll)
{
  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->leds.bit.cap = caps & 0x01;

  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->leds.bit.num = num & 0x01;

  ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->leds.bit.scroll = scroll & 0x01;

  sendCommand_noack(p_ps2keyboard, CMD_SET_LED);

  sendData(p_ps2keyboard, ((struct s_ps2keyboard *)(p_ps2keyboard->p_device))->leds.packet);
}


void getID(void *p_data, uint16_t ps2Data)
{
  static int shift = 0;
  uint8_t convData = 0;

  struct s_ps2 *p_ps2 = NULL;

  if(p_data == NULL) return;

  p_ps2 = (struct s_ps2 *)p_data;

  convData = convertToRaw(ps2Data);

  ((struct s_ps2keyboard *)(p_ps2->p_device))->id |= convData << (shift * 8);

  shift++;

  p_ps2->callbackState = (shift > 1 ? dev_id : waiting);

  shift %= 2;
}

void checkKeyboardResponse(void *p_data, uint16_t ps2Data)
{
  uint8_t convData = 0;

  struct s_ps2 *p_ps2 = NULL;

  if(p_data == NULL) return;

  p_ps2 = (struct s_ps2 *)p_data;

  convData = convertToRaw(ps2Data);

  p_ps2->recvCallback = &extractData;

  switch(convData)
  {
    case CMD_DEV_RDY:
      p_ps2->callbackState = ready_cmd;
      break;
    case CMD_RESEND:
      p_ps2->callbackState = resend_cmd;
      break;
    case CMD_ACK:
      if(p_ps2->lastCMD == CMD_READ_ID)
        p_ps2->recvCallback = &getID;
      if(p_ps2->lastCMD == CMD_RESET)
        p_ps2->recvCallback = &checkKeyboardResponse;
      p_ps2->callbackState = ack_cmd;
      break;
    default:
      p_ps2->callbackState = no_cmd;
      break;
  }
}

void extractData(void *p_data, uint16_t ps2data)
{
  uint8_t rawPS2data = 0;
  uint8_t definePS2data = 0;

  struct s_ps2 *p_ps2 = NULL;

  if(p_data == NULL) return;

  p_ps2 = (struct s_ps2 *)p_data;

  rawPS2data = convertToRaw(ps2data);

  definePS2data = convertToDefine(p_ps2, rawPS2data);

  switch(definePS2data)
  {
    case KEYCODE_CAPS:
      if(!getPS2keyReleased(p_ps2) && (((struct s_ps2keyboard *)(p_ps2->p_device))->prevCapRelease == release))
      {
        ((struct s_ps2keyboard *)(p_ps2->p_device))->leds.bit.cap = ~((struct s_ps2keyboard *)(p_ps2->p_device))->leds.bit.cap;
      }

      ((struct s_ps2keyboard *)(p_ps2->p_device))->prevCapRelease = ((struct s_ps2keyboard *)(p_ps2->p_device))->keyReleaseState;
      break;
    case KEYCODE_NUM:
      if(!getPS2keyReleased(p_ps2) && (((struct s_ps2keyboard *)(p_ps2->p_device))->prevNumRelease == release))
      {
        ((struct s_ps2keyboard *)(p_ps2->p_device))->leds.bit.num = ~((struct s_ps2keyboard *)(p_ps2->p_device))->leds.bit.num;
      }

      ((struct s_ps2keyboard *)(p_ps2->p_device))->prevNumRelease = ((struct s_ps2keyboard *)(p_ps2->p_device))->keyReleaseState;
      break;
    case KEYCODE_SCROLL:
      if(!getPS2keyReleased(p_ps2) && (((struct s_ps2keyboard *)(p_ps2->p_device))->prevScrollRelease == release))
      {
        ((struct s_ps2keyboard *)(p_ps2->p_device))->leds.bit.scroll = ~((struct s_ps2keyboard *)(p_ps2->p_device))->leds.bit.scroll;
      }

      ((struct s_ps2keyboard *)(p_ps2->p_device))->prevScrollRelease = ((struct s_ps2keyboard *)(p_ps2->p_device))->keyReleaseState;
      break;
    default:
      p_ps2->userRecvCallback(definePS2data);
      break;
  }
}
