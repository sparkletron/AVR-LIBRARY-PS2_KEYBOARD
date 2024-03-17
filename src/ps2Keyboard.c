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

struct s_ps2 g_ps2;

enum callbackStates {waiting, no_cmd, resend_cmd, ack_cmd, ready_cmd, keyboard_id};
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

  uint8_t lastCMD;

  volatile enum keyReleaseStates keyReleaseState;
  volatile enum callbackStates callbackState;
} g_ps2keyboard;

//helper functions
//waits for callback to return, returns state of the callback for error
//sends commands
void sendCommand(uint8_t command);
//sends data, (does not update LAST_CMD);
void sendData(uint8_t data);
//sends commands that have no ack
void sendCommand_noack(uint8_t command);
//sends data that has no ack, (does not update LAST_CMD);
void sendData_noack(uint8_t data);
//handling (none implimented at this time).
enum callbackStates waitingForCallback();
//wait for idle state in rx/tx irq
void waitForDataIdle();
//wait for device ready AA, 00
void waitForDevReady();
//wait for command ack, eventually add timeouts to these methods
void waitForCMDack();
//wait for keyboard id state from callback
void waitForKeyboardID();
//convert raw PS2 data to a define via a lookup table. This will set the keybreak flag on or off.
uint8_t convertToDefine(uint8_t ps2data);
//Converts PS2 data to raw data, this performs checks on the data.
//If it returns 0 then the data is invalid.
uint8_t convertToRaw(uint16_t ps2data);
//Send the PS2 led command that allows PS2 LED states to be send to the keyboard.
void sendPS2ledsCMD();
//set internal LED tracking and send LED state to keyboard.
void setPS2leds(uint8_t caps, uint8_t num, uint8_t scroll);
//generate odd parity
uint8_t oddParityGen(uint16_t data);
//convert 8 bit data into a 11 bit packet
uint16_t dataToPacket(uint8_t data);
//copys data passed to it to the internal send buffer.
void copyPacketToBuffer(uint16_t packet);
//start trasmission of data to the keyboard.
void startTransmit();
//CALLBACK ROUTINES
//check the response to keyboard command sent and perform needed operations
void checkKeyboardResponse(uint16_t ps2Data);
//Default callback for recv that processes data and then hands it off to the user callback.
void extractData(uint16_t ps2data);

void initPS2keyboard(t_PS2userRecvCallback PS2recvCallback, void (*setPS2_PORT_Device)(struct s_ps2 *p_device), volatile uint8_t *p_port, uint8_t clkPin, uint8_t dataPin)
{
  uint8_t tmpSREG = 0;

  tmpSREG = SREG;
  cli();

  if(p_port == NULL) return;

  if(PS2recvCallback == NULL) return;

  if(setPS2_PORT_Device == NULL) return;

  setPS2_PORT_Device(&g_ps2);

  memset(&g_ps2keyboard, 0, sizeof(g_ps2keyboard));

  memset(&g_ps2, 0, sizeof(g_ps2));

  g_ps2.clkPin = clkPin;
  g_ps2.dataPin = dataPin;
  g_ps2.p_port = p_port;
  g_ps2.lastAckState = ack;
  g_ps2.dataState = idle;
  g_ps2.userRecvCallback = PS2recvCallback;
  g_ps2.recvCallback = &extractData;

  g_ps2keyboard.prevCapRelease    = release;
  g_ps2keyboard.prevNumRelease    = release;
  g_ps2keyboard.prevScrollRelease = release;

  *(g_ps2.p_port - 1) &= ~(1 << g_ps2.clkPin);
  *(g_ps2.p_port - 1) &= ~(1 << g_ps2.dataPin);

  if(g_ps2.p_port == &PORTB)
  {
    PCICR |= 1 << PCIE0;
    PCMSK0 |= 1 << g_ps2.clkPin;
  }
  else if(g_ps2.p_port == &PORTC)
  {
    PCICR |= 1 << PCIE1;
    PCMSK1 |= 1 << g_ps2.clkPin;
  }
  else
  {
    PCICR |= 1 << PCIE2;
    PCMSK2 |= 1 << g_ps2.clkPin;
  }

  SREG = tmpSREG;

  sei();

  //initialize keyboard using PC init method
  resetPS2keyboard();

  setPS2leds(0, 0, 0);
}

char PS2defineToChar(uint8_t ps2data)
{
  uint8_t tmpSREG = 0;

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

        return (getPS2capsLockState() ? e_set2scanCodes[index].ascii - 32 : e_set2scanCodes[index].ascii);
      }

      SREG = tmpSREG;

      return e_set2scanCodes[index].ascii;
    }
  }

  SREG = tmpSREG;
  return e_set2scanCodes[index].ascii;
}

void updatePS2leds()
{
  waitForDataIdle();

  if(g_ps2keyboard.prevLEDS.packet != g_ps2keyboard.leds.packet)
  {
    setPS2leds(getPS2capsLockState(), getPS2numLockState(), getPS2scrollLockState());
  }

  g_ps2keyboard.prevLEDS.packet = g_ps2keyboard.leds.packet;
}

uint8_t getPS2keyReleased()
{
  return (g_ps2keyboard.keyReleaseState == release);
}

uint16_t getPS2keyboardID()
{
  return g_ps2keyboard.id;
}

uint8_t getPS2capsLockState()
{
  return g_ps2keyboard.leds.bit.cap;
}

uint8_t getPS2numLockState()
{
  return g_ps2keyboard.leds.bit.num;
}

uint8_t getPS2scrollLockState()
{
  return g_ps2keyboard.leds.bit.scroll;
}

void resendPS2lastByte()
{
  sendCommand(CMD_RESEND);
}

void resetPS2keyboard()
{
  sendCommand(CMD_RESET);

  waitForDevReady();
}

void disablePS2keyboard()
{
  sendCommand(CMD_DISABLE);
}

void enablePS2keyaboard()
{
  sendCommand(CMD_ENABLE);
}

void setPS2default()
{
  sendCommand(CMD_DEFAULT);
}

//broken? Need to check for ACKs
void setPS2typmaticRateDelay(uint8_t delay, uint8_t rate)
{
  g_ps2keyboard.typematic.param.rate = (rate <= MAX_REPEAT_RATE ? rate : DEFAULT_RATE);

  g_ps2keyboard.typematic.param.delay = (delay <= MAX_DELAY ? delay : DEFAULT_DELAY);

  sendCommand(CMD_SET_RATE);

  sendData(g_ps2keyboard.typematic.packet);
}

void sendPS2readIDcmd()
{
  g_ps2keyboard.id = 0;

  sendCommand(CMD_READ_ID);

  waitForKeyboardID();
}

//helper functions
void sendCommand(uint8_t command)
{
  g_ps2keyboard.lastCMD = command;

  sendData(command);
}

void sendCommand_noack(uint8_t command)
{
  g_ps2keyboard.lastCMD = command;

  sendData_noack(command);
}

void sendData(uint8_t data)
{
  uint8_t tmpSREG = 0;
  uint16_t tempConv = 0;

  tmpSREG = SREG;

  waitForDataIdle();

  cli();

  tempConv = dataToPacket(data);

  copyPacketToBuffer(tempConv);

  startTransmit();

  SREG = tmpSREG;

  waitingForCallback();
}

void sendData_noack(uint8_t data)
{
  uint8_t tmpSREG = 0;
  uint16_t tempConv = 0;

  tmpSREG = SREG;

  waitForDataIdle();

  cli();

  tempConv = dataToPacket(data);

  copyPacketToBuffer(tempConv);

  startTransmit();

  //no ack
  g_ps2.recvCallback = &extractData;
  g_ps2keyboard.callbackState = no_cmd;

  SREG = tmpSREG;
}

enum callbackStates waitingForCallback()
{
  while(g_ps2keyboard.callbackState == waiting);

  return g_ps2keyboard.callbackState;
}

void waitForDataIdle()
{
  while(g_ps2.dataState != idle);
}

void waitForDevReady()
{
  while(g_ps2keyboard.callbackState != ready_cmd);
}

void waitForKeyboardID()
{
  while(g_ps2keyboard.callbackState != keyboard_id);
}

//FA is ACK
void waitForCMDack()
{
  while(g_ps2keyboard.callbackState != ack_cmd);
}

uint8_t convertToDefine(uint8_t ps2data)
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
      g_ps2keyboard.keyReleaseState = no_release;
      return e_set2scanCodes[index].defineCode;
    }

    if(e_set2scanCodes[index].breakCode == buffer)
    {
      buffer = 0;
      shift = 0;
      g_ps2keyboard.keyReleaseState = release;
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

  g_ps2keyboard.keyReleaseState = no_release;

  return 0;
}

uint8_t convertToRaw(uint16_t ps2data)
{
  uint8_t tmpParity = 0;

  tmpParity = oddParityGen((uint8_t)((ps2data >> DATA_BIT0_POS) & 0x00FF));

  if(tmpParity != (uint8_t)((ps2data >> PARITY_BIT_POS) & 0x0001)) return 0;

  return (uint8_t)((ps2data >> DATA_BIT0_POS) & 0x00FF);
}

void setPS2leds(uint8_t caps, uint8_t num, uint8_t scroll)
{
  g_ps2keyboard.leds.bit.cap = caps & 0x01;

  g_ps2keyboard.leds.bit.num = num & 0x01;

  g_ps2keyboard.leds.bit.scroll = scroll & 0x01;

  sendCommand_noack(CMD_SET_LED);

  sendData(g_ps2keyboard.leds.packet);
}

uint8_t oddParityGen(uint16_t data)
{
  //setting to 1 generates odd parity. 0 for even.
  uint8_t tempParity = 1;
  uint8_t index = 0;

  for(index = 0; index < sizeof(uint16_t)*8; index++)
  {
    tempParity ^= (data >> index) & 0x01;
  }

  return tempParity;
}

//convert data to packet
uint16_t dataToPacket(uint8_t data)
{
  uint8_t parity = 0;
  uint16_t tempConv = 0;

  //start value is 0, tempConv set to 0, no need to do again.
  tempConv |= ((uint16_t)data) << DATA_BIT0_POS;

  parity = oddParityGen(data);

  tempConv |= ((uint16_t)parity) << PARITY_BIT_POS;

  tempConv |= ((uint16_t)STOP_BIT_VALUE) << STOP_BIT_POS;

  return tempConv;
}

//copy to send buffer
void copyPacketToBuffer(uint16_t packet)
{
  g_ps2.buffer = packet;
}

//start transmit routine.
void startTransmit()
{
  g_ps2.dataState = send;

  g_ps2keyboard.callbackState = waiting;

  g_ps2.recvCallback = &checkKeyboardResponse;

  //set clock pin low for at least 100ms
  *(g_ps2.p_port - 1) |= 1 << g_ps2.clkPin;

  *g_ps2.p_port &= ~(1 << g_ps2.clkPin);

  _delay_ms(110);

  //set data pin to output and set low
  *(g_ps2.p_port - 1) |= 1 << g_ps2.dataPin;

  *g_ps2.p_port &= ~(1 << g_ps2.dataPin);

  //release clock pin by setting to input
  *(g_ps2.p_port - 1) &= ~(1 << g_ps2.clkPin);

  g_ps2.index++;
}

void getID(uint16_t ps2Data)
{
  static int shift = 0;
  uint8_t convData = 0;

  convData = convertToRaw(ps2Data);

  g_ps2keyboard.id |= convData << (shift * 8);
  g_ps2keyboard.callbackState = waiting;

  shift++;

  g_ps2keyboard.callbackState = (shift > 1 ? keyboard_id : waiting);

  shift %= 2;
}

void checkKeyboardResponse(uint16_t ps2Data)
{
  uint8_t convData = 0;

  convData = convertToRaw(ps2Data);

  g_ps2.recvCallback = &extractData;

  switch(convData)
  {
    case CMD_DEV_RDY:
      g_ps2keyboard.callbackState = ready_cmd;
      break;
    case CMD_RESEND:
      g_ps2keyboard.callbackState = resend_cmd;
      break;
    case CMD_ACK:
      if(g_ps2keyboard.lastCMD == CMD_READ_ID)
        g_ps2.recvCallback = &getID;
      if(g_ps2keyboard.lastCMD == CMD_RESET)
        g_ps2.recvCallback = &checkKeyboardResponse;
      g_ps2keyboard.callbackState = ack_cmd;
      break;
    default:
      g_ps2keyboard.callbackState = no_cmd;
      break;
  }
}

void extractData(uint16_t ps2data)
{
  uint8_t rawPS2data = 0;
  uint8_t definePS2data = 0;

  rawPS2data = convertToRaw(ps2data);

  definePS2data = convertToDefine(rawPS2data);

  switch(definePS2data)
  {
    case KEYCODE_CAPS:
      if(!getPS2keyReleased() && (g_ps2keyboard.prevCapRelease == release))
      {
        g_ps2keyboard.leds.bit.cap = ~g_ps2keyboard.leds.bit.cap;
      }

      g_ps2keyboard.prevCapRelease = g_ps2keyboard.keyReleaseState;
      break;
    case KEYCODE_NUM:
      if(!getPS2keyReleased() && (g_ps2keyboard.prevNumRelease == release))
      {
        g_ps2keyboard.leds.bit.num = ~g_ps2keyboard.leds.bit.num;
      }

      g_ps2keyboard.prevNumRelease = g_ps2keyboard.keyReleaseState;
      break;
    case KEYCODE_SCROLL:
      if(!getPS2keyReleased() && (g_ps2keyboard.prevScrollRelease == release))
      {
        g_ps2keyboard.leds.bit.scroll = ~g_ps2keyboard.leds.bit.scroll;
      }

      g_ps2keyboard.prevScrollRelease = g_ps2keyboard.keyReleaseState;
      break;
    default:
      g_ps2.userRecvCallback(definePS2data);
      break;
  }
}
