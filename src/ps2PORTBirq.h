/*
 * AVR328P-LIBRARY-PS2_DRIVER
 *
 *  Created on: September 28, 2017
 *      Author: John Convertino
 *		email: electrobs@gmail.com
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

#ifndef PS2PORTBIRQ_H_
#define PS2PORTBIRQ_H_

#include <avr/interrupt.h>
#include <avr/common.h>
#include <avr/io.h>
#include <util/delay.h>
#include "ps2DataType.h"
#include "ps2defines.h"
#include "ps2Keyboard.h"
ISR(PCINT0_vect)
{
	uint8_t pinState = (*(e_ps2keyboard.p_port - 2) & (1 << e_ps2keyboard.clkPin)) >> e_ps2keyboard.clkPin;
	
	if((e_ps2keyboard.modeFlag == SEND_MODE) && !pinState)
	{
		//as long as we are not at the stop bit
		if(e_ps2keyboard.sendIndex <= STOP_BIT_POS)
		{
			uint16_t portValue = (e_ps2keyboard.sendBuffer >> e_ps2keyboard.sendIndex) & 0x0001;
			
			if(portValue != 0)
			{
				*(e_ps2keyboard.p_port - 1) &= ~(1 << e_ps2keyboard.dataPin);
			}
			else
			{
				*e_ps2keyboard.p_port &= ~(1 << e_ps2keyboard.dataPin);
				*(e_ps2keyboard.p_port - 1) |= 1 << e_ps2keyboard.dataPin;
			}
		}
		else
		{
			*(e_ps2keyboard.p_port - 1) &= ~(1 << e_ps2keyboard.dataPin);
			
			if((*(e_ps2keyboard.p_port - 2) >> e_ps2keyboard.dataPin) & 0x01)
			{
				e_ps2keyboard.callbackState = noack;
			}

			e_ps2keyboard.modeFlag = RECV_MODE;
		}
	
		e_ps2keyboard.sendIndex++;
		
		e_ps2keyboard.sendIndex %= SEND_BUFF_SIZE;
	}
	else if((e_ps2keyboard.modeFlag == RECV_MODE) && !pinState)
	{
		//build up recvBuffer
		e_ps2keyboard.recvBuffer |= ((*(e_ps2keyboard.p_port - 2) >> e_ps2keyboard.dataPin) & 0x01) << e_ps2keyboard.recvIndex;
		//on stop bit, callBack is used.
		
		if(e_ps2keyboard.recvIndex == STOP_BIT_POS)
		{
			e_ps2keyboard.recvCallback(e_ps2keyboard.recvBuffer);
			e_ps2keyboard.recvBuffer = 0;
		}
		
		e_ps2keyboard.recvIndex++;
		
		e_ps2keyboard.recvIndex %= RECV_BUFF_SIZE;
	}
}

#endif /* PS2PORTBIRQ_H_ */
