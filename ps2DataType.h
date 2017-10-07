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

#ifndef PS2DATATYPE_H_
#define PS2DATATYPE_H_

#include <inttypes.h>
#include "ps2defines.h"

extern struct s_ps2keyboard
{
	volatile uint8_t *p_port;
	uint8_t clkPin;
	uint8_t dataPin;
	
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
	} leds;
	
	union
	{
		struct
		{
			uint8_t rate:4;
			uint8_t delay:4;
		} param;
		
		uint8_t packet;
	} typematic;
	
	volatile enum callbackStates callbackState;
	volatile uint8_t modeFlag:1;
	volatile uint8_t keybreak:1;
	volatile uint8_t idRecv:1;
	
	volatile uint8_t sendIndex;
	volatile uint8_t recvIndex;
	
	uint16_t id;
	
	uint16_t sendBuffer;
	uint16_t recvBuffer;
	
	t_PS2userRecvCallback userRecvCallback;
	t_PS2recvCallback recvCallback;
	
} e_ps2keyboard;

#endif /* PS2DATATYPE_H_ */
