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


#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <avr/common.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ps2Keyboard.h"
#include "ps2DataType.h"
#include "ps2scanCodes.h"

struct s_ps2keyboard e_ps2keyboard;

//helper functions
//waits for callback to return, returns state of the callback for error
//handling (none implimented at this time).
enum callbackStates waitingForCallback();
//convert raw PS2 data to a define via a lookup table. This will set the
//keybreak flag on or off.
uint8_t convertToDefine(uint8_t ps2data);
//Converts PS2 data to raw data, this performs checks on the data.
//If it returns 0 then the data is invalid.
uint8_t convertToRaw(uint16_t ps2data);
//Send the PS2 led command that allows PS2 LED states to be send to the
//keyboard.
void sendPS2ledsCMD();
//set internal LED tracking and send LED state to keyboard.
void setPS2leds(uint8_t caps, uint8_t num, uint8_t scroll);
//generate odd parity
uint8_t oddParityGen(uint8_t data);
//convert 8 bit data into a 11 bit packet
uint16_t dataToPacket(uint8_t data);
//copys data passed to it to the internal send buffer.
void copyPacketToSendBuf(uint16_t packet);
//start trasmission of data to the keyboard.
void startTransmit();
//CALLBACK ROUTINES
//generic routine for keyboard commands that don't send a data byte after
void checkForAck(uint16_t ps2Data);
//On keyboard reset this checks for keyboard ready byte.
void checkForKeyRdy(uint16_t ps2Data);
//Gets the ID of keyboard and stores it for later use.
void getID(uint16_t ps2Data);
//Default callback for recv that processes data and then hands it off to
//the user callback.
void extractData(uint16_t ps2data);

void initPS2keyboard(t_PS2userRecvCallback PS2recvCallback, volatile uint8_t *p_port, uint8_t clkPin, uint8_t dataPin)
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	if(p_port == NULL) return;

	if(PS2recvCallback == NULL) return;

	memset(&e_ps2keyboard, 0, sizeof(e_ps2keyboard));

	e_ps2keyboard.clkPin = clkPin;
	e_ps2keyboard.dataPin = dataPin;
	e_ps2keyboard.p_port = p_port;
	e_ps2keyboard.callbackState = done;
	e_ps2keyboard.userRecvCallback = PS2recvCallback;
	e_ps2keyboard.recvCallback = &checkForAck;

	e_ps2keyboard.modeFlag = RECV_MODE;

	*(e_ps2keyboard.p_port - 1) &= ~(1 << e_ps2keyboard.clkPin);
	*(e_ps2keyboard.p_port - 1) &= ~(1 << e_ps2keyboard.dataPin);

	if(e_ps2keyboard.p_port == &PORTB)
	{
		PCICR |= 1 << PCIE0;
		PCMSK0 |= 1 << e_ps2keyboard.clkPin;
	}
	else if(e_ps2keyboard.p_port == &PORTC)
	{
		PCICR |= 1 << PCIE1;
		PCMSK1 |= 1 << e_ps2keyboard.clkPin;
	}
	else
	{
		PCICR |= 1 << PCIE2;
		PCMSK2 |= 1 << e_ps2keyboard.clkPin;
	}

	SREG = tmpSREG;

	sei();

	//initialize keyboard using PC init method
	resetPS2keyboard();

	sendPS2ledsCMD();

	setPS2leds(0, 0 ,0);
}

char defineToChar(uint8_t ps2data)
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

				return (getCapsLockState() ? e_set2scanCodes[index].ascii - 32 : e_set2scanCodes[index].ascii);
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
	static uint8_t prevLEDS = 0;
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	if(prevLEDS != e_ps2keyboard.leds.packet)
	{
		SREG = tmpSREG;

		sendPS2ledsCMD();

		setPS2leds(getCapsLockState(), getNumLockState(), getScrollLockState());

		cli();
	}

	prevLEDS = e_ps2keyboard.leds.packet;

	SREG = tmpSREG;
}

uint8_t getKeybreakState()
{
	return e_ps2keyboard.keybreak;
}

uint16_t getKeyboardID()
{
	return e_ps2keyboard.id;
}

uint8_t getCapsLockState()
{
	return e_ps2keyboard.leds.bit.cap;
}

uint8_t getNumLockState()
{
	return e_ps2keyboard.leds.bit.num;
}

uint8_t getScrollLockState()
{
	return e_ps2keyboard.leds.bit.scroll;
}

void resendPS2lastByte()
{
	uint16_t tempConv = 0;
	uint8_t tmpSREG = 0;

	//wait till we are done sending last command.
	while(e_ps2keyboard.modeFlag == SEND_MODE);

	tempConv = dataToPacket(CMD_RESEND);

	copyPacketToSendBuf(tempConv);

	startTransmit();
}

void resetPS2keyboard()
{
	uint16_t tempConv = 0;

	//wait till we are done sending last command.
	while(e_ps2keyboard.modeFlag == SEND_MODE);

	e_ps2keyboard.recvCallback = &checkForKeyRdy;

	e_ps2keyboard.callbackState = waiting;

	tempConv = dataToPacket(CMD_RESET);

	copyPacketToSendBuf(tempConv);

	startTransmit();

	waitingForCallback();
}

void disablePS2keyboard()
{
	uint16_t tempConv = 0;

	//wait till we are done sending last command.
	while(e_ps2keyboard.modeFlag == SEND_MODE);

	e_ps2keyboard.recvCallback = &checkForAck;

	e_ps2keyboard.callbackState = waiting;

	tempConv = dataToPacket(CMD_DISABLE);

	copyPacketToSendBuf(tempConv);

	startTransmit();

	waitingForCallback();
}

void enablePS2keyaboard()
{
	uint16_t tempConv = 0;

	//wait till we are done sending last command.
	while(e_ps2keyboard.modeFlag == SEND_MODE);

	e_ps2keyboard.recvCallback = &checkForAck;

	e_ps2keyboard.callbackState = waiting;

	tempConv = dataToPacket(CMD_ENABLE);

	copyPacketToSendBuf(tempConv);

	startTransmit();

	waitingForCallback();
}

void setPS2default()
{
	uint16_t tempConv = 0;

	//wait till we are done sending last command.
	while(e_ps2keyboard.modeFlag == SEND_MODE);

	e_ps2keyboard.recvCallback = &checkForAck;

	e_ps2keyboard.callbackState = waiting;

	tempConv = dataToPacket(CMD_DEFAULT);

	copyPacketToSendBuf(tempConv);

	startTransmit();

	waitingForCallback();
}

void sendPS2typmaticRateDelayCMD()
{
	uint16_t tempConv = 0;

	//wait till we are done sending last command.
	while(e_ps2keyboard.modeFlag == SEND_MODE);

	e_ps2keyboard.recvCallback = &checkForAck;

	e_ps2keyboard.callbackState = waiting;

	tempConv = dataToPacket(CMD_SET_RATE);

	copyPacketToSendBuf(tempConv);

	startTransmit();

	waitingForCallback();
}

void setPS2typmaticRateDelay(uint8_t delay, uint8_t rate)
{
	uint16_t tempConv = 0;

	//wait till we are done sending last command.
	while(e_ps2keyboard.modeFlag == SEND_MODE);

	e_ps2keyboard.recvCallback = &checkForAck;

	e_ps2keyboard.callbackState = waiting;

	e_ps2keyboard.typematic.param.rate = rate;

	e_ps2keyboard.typematic.param.delay = delay;

	tempConv = dataToPacket(e_ps2keyboard.typematic.packet);

	copyPacketToSendBuf(tempConv);

	startTransmit();

	waitingForCallback();
}

void sendPS2readIDcmd()
{
	uint16_t tempConv = 0;

	//wait till we are done sending last command.
	while(e_ps2keyboard.modeFlag == SEND_MODE);

	e_ps2keyboard.id = 0;

	e_ps2keyboard.recvCallback = &getID;

	e_ps2keyboard.idRecv = ID_NRECV;

	e_ps2keyboard.callbackState = waiting;

	tempConv = dataToPacket(CMD_READ_ID);

	copyPacketToSendBuf(tempConv);

	startTransmit();

	waitingForCallback();
}

//helper functions
enum callbackStates waitingForCallback()
{
	while(e_ps2keyboard.callbackState == waiting);

	return e_ps2keyboard.callbackState;
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
			e_ps2keyboard.keybreak = NKEYBREAK;
			return e_set2scanCodes[index].defineCode;
		}

		if(e_set2scanCodes[index].breakCode == buffer)
		{
			buffer = 0;
			shift = 0;
			e_ps2keyboard.keybreak = KEYBREAK;
			return e_set2scanCodes[index].defineCode;
		}
	}

	shift += BIT_COUNT;
	shift %= MAX_BIT_COUNT;

	if(!shift)
	{
		buffer = 0;
	}

	e_ps2keyboard.keybreak = NKEYBREAK;

	return 0;
}

uint8_t convertToRaw(uint16_t ps2data)
{
	uint8_t tmpParity = 0;

	tmpParity = oddParityGen((uint8_t)(ps2data >> DATA_BIT0_POS) & 0xFF);

	if(tmpParity != (uint8_t)((ps2data >> PARITY_BIT_POS) & 0x0001)) return 0;

	return (uint8_t)((ps2data >> DATA_BIT0_POS) & 0x00FF);
}

void sendPS2ledsCMD()
{
	uint16_t tempConv = 0;

	//wait till we are done sending last command.
	while(e_ps2keyboard.modeFlag == SEND_MODE);

	e_ps2keyboard.recvCallback = &checkForAck;

	e_ps2keyboard.callbackState = waiting;

	tempConv = dataToPacket(CMD_SET_LED);

	copyPacketToSendBuf(tempConv);

	startTransmit();

	waitingForCallback();
}

void setPS2leds(uint8_t caps, uint8_t num, uint8_t scroll)
{
	uint16_t tempConv = 0;

	//wait till we are done sending last command.
	while(e_ps2keyboard.modeFlag == SEND_MODE);

	e_ps2keyboard.recvCallback = &checkForAck;

	e_ps2keyboard.callbackState = waiting;

	e_ps2keyboard.leds.bit.cap = (caps != 0 ? 1 : 0);

	e_ps2keyboard.leds.bit.num = (num != 0 ? 1 : 0);

	e_ps2keyboard.leds.bit.scroll = (scroll != 0 ? 1 : 0);

	tempConv = dataToPacket(e_ps2keyboard.leds.packet);

	copyPacketToSendBuf(tempConv);

	startTransmit();

	waitingForCallback();
}

uint8_t oddParityGen(uint8_t data)
{
	//setting to 1 generates odd parity. 0 for even.
	uint8_t tempParity = 1;
	uint8_t index = 0;

	for(index = 0; index < UINT8_T_SIZE; index++)
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

	tempConv = (uint16_t)data << DATA_BIT0_POS;

	parity = oddParityGen(data);

	tempConv |= (uint16_t)parity << PARITY_BIT_POS;

	tempConv |= (uint16_t)STOP_BIT_VALUE << STOP_BIT_POS;

	return tempConv;
}

//copy to send buffer
void copyPacketToSendBuf(uint16_t packet)
{
	e_ps2keyboard.sendBuffer = packet;
}

//start transmit routine.
void startTransmit()
{
	uint8_t tmpSREG = SREG;
	cli();

	e_ps2keyboard.modeFlag = SEND_MODE;

	e_ps2keyboard.sendIndex = 0;

	*(e_ps2keyboard.p_port - 1) |= 1 << e_ps2keyboard.clkPin;

	*e_ps2keyboard.p_port &= ~(1 << e_ps2keyboard.clkPin);

	_delay_ms(100);

	*(e_ps2keyboard.p_port - 1) |= 1 << e_ps2keyboard.dataPin;

	*e_ps2keyboard.p_port &= (e_ps2keyboard.sendBuffer << e_ps2keyboard.dataPin) | ~(1 << e_ps2keyboard.dataPin);

	*(e_ps2keyboard.p_port - 1) &= ~(1 << e_ps2keyboard.clkPin);

	e_ps2keyboard.sendIndex++;

	SREG = tmpSREG;
}

void checkForAck(uint16_t ps2Data)
{
	uint8_t convData = 0;

	convData = convertToRaw(ps2Data);

	switch(convData)
	{
		case CMD_ACK:
			e_ps2keyboard.callbackState = done;
			e_ps2keyboard.recvCallback = &extractData;
			break;
		case CMD_RESEND:
			e_ps2keyboard.callbackState = resend;
			break;
		default:
			e_ps2keyboard.callbackState = error;
			e_ps2keyboard.recvCallback = &extractData;
			break;
	}
}

void checkForKeyRdy(uint16_t ps2Data)
{
	uint8_t convData = 0;

	convData = convertToRaw(ps2Data);

	switch(convData)
	{
		case CMD_KEY_RDY:
			e_ps2keyboard.callbackState = done;
			e_ps2keyboard.recvCallback = &extractData;
			break;
		case CMD_RESEND:
			e_ps2keyboard.callbackState = resend;
			break;
		case CMD_ACK:
			e_ps2keyboard.callbackState = waiting;
			break;
		default:
			e_ps2keyboard.callbackState = error;
			e_ps2keyboard.recvCallback = &extractData;
			break;
	}
}

void getID(uint16_t ps2Data)
{
	uint8_t convData = 0;

	convData = convertToRaw(ps2Data);

	switch(convData)
	{
		case KEYBOARD_ID1:
			e_ps2keyboard.id |= convData;
			e_ps2keyboard.callbackState = waiting;
			break;
		case KEYBOARD_ID2:
			e_ps2keyboard.id |= convData << UINT8_T_SIZE;
			e_ps2keyboard.idRecv = ID_RECV;
			e_ps2keyboard.callbackState = done;
			e_ps2keyboard.recvCallback = &extractData;
			break;
		case CMD_RESEND:
			e_ps2keyboard.callbackState = resend;
			break;
		case CMD_ACK:
			e_ps2keyboard.callbackState = waiting;
			break;
		default:
			e_ps2keyboard.callbackState = error;
			e_ps2keyboard.recvCallback = &extractData;
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
			if(!getKeybreakState())
			{
				e_ps2keyboard.leds.bit.cap = ~(e_ps2keyboard.leds.bit.cap & 0x01);
			}
			break;
		case KEYCODE_NUM:
			if(!getKeybreakState())
			{
				e_ps2keyboard.leds.bit.num = ~(e_ps2keyboard.leds.bit.num & 0x01);
			}
			break;
		case KEYCODE_SCROLL:
			if(!getKeybreakState())
			{
				e_ps2keyboard.leds.bit.scroll = ~(e_ps2keyboard.leds.bit.scroll & 0x01);
			}
			break;
		default:
			e_ps2keyboard.userRecvCallback(definePS2data);
			break;
	}
}
