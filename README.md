# PS2 Keyboard driver for AVR based microcontrollers.

## Sample Code

```c
#include <inttypes.h>
#include <avr/common.h>
#include <avr/io.h>

#include "ps2PORTBirq.h"
#include "ps2Keyboard.h"

void recvCallback(uint8_t recvBuffer);

int main(void)
{
	DDRD = ~0;

	PORTD = 0;

	initPS2keyboard(&recvCallback, &PORTB, PORTB0, PORTB1);

	for(;;)
	{
		updatePS2leds();
	}
}

void recvCallback(uint8_t recvBuffer)
{
	PORTD = recvBuffer;
}
```
