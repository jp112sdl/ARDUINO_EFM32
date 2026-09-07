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

#include <stddef.h>
#include "efm32.h"

void yield(void);
void ClockUpdate(void);
void RTC_Trigger(uint32_t msec, void(*funcPointer)(void));
uint16_t CRC_calc(uint8_t *start, uint8_t *end);

volatile uint32_t msTicks;

void systicCallback(void)  __attribute__ ((weak));
void systicCallback(void){}

#if (FREERTOS == 0)
void SysTick_Handler(void)
{
	++msTicks;
	systicCallback();   
}
#endif

#if USE_WDOG > 0
void wdogFeed(void)
{
  WDOGn_Feed(DEFAULT_WDOG);
}

static void initWdog(void)
{
  WDOG_Init_TypeDef wdogInit = WDOG_INIT_DEFAULT;

  wdogInit.enable   = true;
  wdogInit.debugRun = false;          /* do not count while halted in a debugger */
  wdogInit.em2Run   = (WDOG_EM2RUN != 0);
  wdogInit.em3Run   = (WDOG_EM3RUN != 0);
  wdogInit.clkSel   = WDOG_CLKSEL;
  wdogInit.perSel   = WDOG_PERIOD;

  CMU_ClockEnable(cmuClock_HFLE, true);   /* WDOG sits in the LE clock domain */
  WDOGn_Init(DEFAULT_WDOG, &wdogInit);
}
#endif

void delay(uint32_t dlyTicks)
{
  uint32_t curTicks;
  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) {
#if USE_WDOG > 0
    wdogFeed();      /* a long delay() is legitimate, do not let it trip the dog */
#endif
    yield();
  }
}

void Fatal_Handler(void)
{
	while(1);
//    errorFlash(ERR_HardFault);
}

/**************************************************************************//**
 * @brief	Callback function for RTC
 *****************************************************************************/
volatile uint32_t Seconds;
void ClockUpdate(void)
{
	++Seconds;
}

extern  void SystemClock_Config(void);

/* ---------------------------------------------------------------------------
 * Clock sources
 *
 * These used to live in variant.c - badly. Only efm32g200f had an HFXO
 * function at all, and it just poked CMU->CTRL without ever enabling or
 * selecting the oscillator, so picking "HFXO (24M)" left the part running on
 * the 14 MHz HFRCO while every F_CPU-derived constant assumed 24 MHz. The
 * other four variants called HFXO_/LFXO_/enter_DefaultMode_from_RESET(),
 * none of which was defined anywhere - selecting a crystal there did not even
 * link. Implemented once here so every variant gets the same behaviour.
 *
 * Both crystal paths wait for the oscillator with a bounded loop and fall back
 * to the HFRCO if it never starts. emlib is not usable directly for this:
 * CMU_OscillatorEnable() and CMU_ClockSelectSet() spin on the ready flag
 * without any timeout, so a missing or dead crystal hangs the board inside
 * init(), before setup() ever runs.
 * ------------------------------------------------------------------------- */

/* Nonzero once a crystal failed to start and the HFRCO was used instead.
 * SystemCoreClock is still correct in that case, so millis()/baud rates stay
 * right, but anything computed from the compile-time F_CPU will be off. */
volatile uint8_t SystemClockFallback = 0;

/* Highest HFRCO band that does not exceed F_CPU. For every band offered by the
 * boards.txt menu this picks that exact band; for a crystal frequency it picks
 * the closest band at or below it. */
static void selectHfrcoBand(void)
{
#if   F_CPU >= 28000000L
  CMU_HFRCOBandSet(cmuHFRCOBand_28MHz);
#elif F_CPU >= 21000000L
  CMU_HFRCOBandSet(cmuHFRCOBand_21MHz);
#elif F_CPU >= 14000000L
  CMU_HFRCOBandSet(cmuHFRCOBand_14MHz);
#elif F_CPU >= 11000000L
  CMU_HFRCOBandSet(cmuHFRCOBand_11MHz);
#elif F_CPU >=  7000000L
  CMU_HFRCOBandSet(cmuHFRCOBand_7MHz);
#else
  CMU_HFRCOBandSet(cmuHFRCOBand_1MHz);
#endif
}

void enter_DefaultMode_from_RESET(void)
{
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
  selectHfrcoBand();
  CMU_ClockEnable(cmuClock_HFPER, true);
}

static void fallbackToHfrco(void)
{
  SystemClockFallback = 1;
  enter_DefaultMode_from_RESET();
}

bool HFXO_enter_DefaultMode_from_RESET(void)
{
  uint32_t spin = HFXO_STARTUP_SPIN;

  /* Record the crystal frequency before anything reads it back. */
  SystemHFXOClockSet(F_CPU);

  CMU->CTRL = (CMU->CTRL & ~(_CMU_CTRL_HFXOMODE_MASK
                             | _CMU_CTRL_HFXOBOOST_MASK
                             | _CMU_CTRL_HFXOTIMEOUT_MASK))
              | CMU_CTRL_HFXOMODE_XTAL
              | HFXO_BOOST
              | CMU_CTRL_HFXOTIMEOUT_16KCYCLES;

  CMU->OSCENCMD = CMU_OSCENCMD_HFXOEN;
  while ((CMU->STATUS & CMU_STATUS_HFXORDY) == 0u) {
    if (--spin == 0u) {
      CMU->OSCENCMD = CMU_OSCENCMD_HFXODIS;
      fallbackToHfrco();
      return false;
    }
  }

  /* HFXORDY is already set, so the wait inside CMU_ClockSelectSet() returns
   * immediately. Going through emlib still buys the HFLE wait-state handling
   * and the >32 MHz buffer-current adjustment. */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  CMU_ClockEnable(cmuClock_HFPER, true);
  (void)SystemCoreClockGet();            /* refresh the cached SystemCoreClock */

  CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);   /* no longer needed */
  return true;
}

bool LFXO_enter_DefaultMode_from_RESET(void)
{
  uint32_t spin = LFXO_STARTUP_SPIN;

  SystemLFXOClockSet(LFXO_FREQ);

  CMU->CTRL = (CMU->CTRL & ~(_CMU_CTRL_LFXOMODE_MASK | _CMU_CTRL_LFXOBOOST_MASK))
              | CMU_CTRL_LFXOMODE_XTAL
              | LFXO_BOOST;

  CMU->OSCENCMD = CMU_OSCENCMD_LFXOEN;
  while ((CMU->STATUS & CMU_STATUS_LFXORDY) == 0u) {
    if (--spin == 0u) {
      CMU->OSCENCMD = CMU_OSCENCMD_LFXODIS;
      fallbackToHfrco();
      return false;
    }
  }

  /* Drives HFCLK straight from the 32768 Hz crystal: lowest power, but SysTick
   * then has only ~33 ticks per millisecond. */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_LFXO);
  CMU_ClockEnable(cmuClock_HFPER, true);
  (void)SystemCoreClockGet();
  return true;
}

#if (FREERTOS == 0)
/* Interrupt priorities (Cortex-M: 0 = highest).
 * SysTick_Config() parks SysTick at the lowest priority while every peripheral
 * IRQ keeps its reset default of 0. A peripheral handler can therefore starve
 * SysTick_Handler, msTicks stops counting and delay()/millis() never advance.
 * Put SysTick above the peripherals and keep level 0 free for a user IRQ that
 * really needs to preempt the time base. */
#ifndef CORE_SYSTICK_PRIORITY
# define CORE_SYSTICK_PRIORITY     1u
#endif
#ifndef CORE_PERIPHERAL_PRIORITY
# define CORE_PERIPHERAL_PRIORITY  3u
#endif

static void initIrqPriority(void)
{
  for (int irq = 0; irq < EXT_IRQ_COUNT; irq++) {
    NVIC_SetPriority((IRQn_Type)irq, CORE_PERIPHERAL_PRIORITY);
  }
  NVIC_SetPriority(SysTick_IRQn, CORE_SYSTICK_PRIORITY);
  NVIC_SetPriority(PendSV_IRQn,  (1u << __NVIC_PRIO_BITS) - 1u);
  NVIC_SetPriority(SVCall_IRQn,  (1u << __NVIC_PRIO_BITS) - 1u);
}
#endif

void init(void)
{
    CHIP_Init();

    SystemClock_Config();

	if (SysTick_Config(SystemCoreClockGet() / 1000)){ //
	     Fatal_Handler();             // never return;
	}
#if (FREERTOS == 0)
	initIrqPriority();       /* must run after SysTick_Config() */
#endif
	CMU_ClockEnable(cmuClock_GPIO, true);
#if USE_WDOG > 0
	/* Started before setup() runs, so a setup() that blocks longer than
	 * WDOG_PERIOD has to call wdogFeed() itself. */
	initWdog();
#endif
}

