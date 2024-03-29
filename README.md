# PS2 Keyboard driver for AVR based microcontrollers.

Interface a atmel microcontroller with a PS/2 keyboard.

author: Jay Convertino

data: 2024.03.11

license: MIT

## Release Versions
### Current
  - release_v0.1.0

### Past
  - none

## Requirements
  - avr-gcc
  - avrlibc
  - PS2_BASE (submodule)

## Building
  - make : builds all

## Documentation
  - See doxygen generated document
  - Method for ready check is universal, NOT efficent. Optimize send data for your application!

### Example Code
```c
#include <inttypes.h>
#include <avr/common.h>
#include <avr/io.h>

#include "ps2PORTBirq.h"
#include "ps2Keyboard.h"

void recvCallback(uint8_t recvBuffer);

int main(void)
{
  struct s_ps2 ps2;

  DDRD = ~0;

  PORTD = 0;

  initPS2keyboard(&ps2, &recvCallback, &setPS2_PORTB_Device, &PORTB, PORTB0, PORTB1);

  for(;;)
  {
    updatePS2leds(&ps2);
  }
}

void recvCallback(uint8_t recvBuffer)
{
  PORTD = recvBuffer;
}
```
