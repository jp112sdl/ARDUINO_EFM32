/*
  Copyright (c) 2018 huaweiwx@sina.com 2018.7.1

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

/* g_Pin2PortMapArray has NUM_DIGITAL_PINS entries. None of these functions
 * used to range-check, so an out-of-range pin read a port/pin pair from
 * whatever followed the table and then drove a random GPIO. */

extern void pinMode(uint8_t ucPin, uint32_t u32Mode )
{
  unsigned int out;

  if (ucPin >= NUM_DIGITAL_PINS) return;

  /* DOUT picks pull-up vs pull-down for gpioModeInputPull, and the idle level
   * for the wired-and/wired-or modes. */
  switch ( u32Mode )
  {
    case INPUT_PULLUP:
    case OUTPUT_OD:
      out = 1;
      break;
    default:
      out = 0;
      break;
  }

  /* Passed through rather than switched on, so every GPIO_Mode_TypeDef works.
   * The old switch silently ignored anything but INPUT/INPUT_PULLUP/OUTPUT/
   * gpioModeDisabled - OUTPUT_OD was accepted by the compiler and then did
   * nothing at all. */
  GPIO_PinModeSet(g_Pin2PortMapArray[ucPin].GPIOx_Port,
                  g_Pin2PortMapArray[ucPin].Pin_abstraction,
                  (GPIO_Mode_TypeDef)(u32Mode & GPIO_MODE_MASK), out);
}

extern void digitalWrite( uint8_t ucPin, uint8_t u32Val )
{
  if (ucPin >= NUM_DIGITAL_PINS) return;

  if (u32Val) {
    GPIO_PinOutSet(g_Pin2PortMapArray[ucPin].GPIOx_Port, g_Pin2PortMapArray[ucPin].Pin_abstraction);
  } else {
    GPIO_PinOutClear(g_Pin2PortMapArray[ucPin].GPIOx_Port, g_Pin2PortMapArray[ucPin].Pin_abstraction);
  }
}

extern int digitalRead(uint8_t ucPin )
{
  if (ucPin >= NUM_DIGITAL_PINS) return LOW;

  return bitRead(GPIO_PortInGet(g_Pin2PortMapArray[ucPin].GPIOx_Port), g_Pin2PortMapArray[ucPin].Pin_abstraction);
}

extern void digitalToggle(uint8_t ucPin )
{
  if (ucPin >= NUM_DIGITAL_PINS) return;

  GPIO_PinOutToggle(g_Pin2PortMapArray[ucPin].GPIOx_Port, g_Pin2PortMapArray[ucPin].Pin_abstraction);
}

extern uint32_t pulseIn(uint8_t ucPin, uint8_t ucState, uint32_t u32Timeout )
{
  uint32_t startMicros;

  if (ucPin >= NUM_DIGITAL_PINS) return 0;

  startMicros = micros();

  // wait for any previous pulse to end
  while (digitalRead(ucPin) == (int)ucState) {
    if (micros() - startMicros > u32Timeout)
      return 0;
  }

  // wait for the pulse to start
  while (digitalRead(ucPin) != (int)ucState) {
    if (micros() - startMicros > u32Timeout)
      return 0;
  }

  uint32_t start = micros();
  // wait for the pulse to stop
  while (digitalRead(ucPin) == (int)ucState) {
    if (micros() - startMicros > u32Timeout)
      return 0;
  }
  return (micros() - start);
}

#ifdef __cplusplus
}
#endif

