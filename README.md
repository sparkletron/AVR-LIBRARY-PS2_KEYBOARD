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

### TODO
  - code cleanup between this and PS2 mouse. Lots of duplicate code (move to PS2 base).

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
  initPS2keyboard(&recvCallback, &setPS2_PORTB_Device, &PORTB, PORTB0, PORTB1);

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
