/*
  ARM CLOCK UTILS
  
  Copyright (c) 2018 huaweiwx@sina.com 2018.7.1

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef  __ARMCLOCK_H__
 #define __ARMCLOCK_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

extern volatile uint32_t msTicks;

/* Clock source setup, implemented in efm32init.c and called from the
 * variant's SystemClock_Config(). The crystal variants return false if the
 * oscillator did not start within HFXO_STARTUP_SPIN / LFXO_STARTUP_SPIN and
 * the HFRCO was used instead. */
bool HFXO_enter_DefaultMode_from_RESET(void);
bool LFXO_enter_DefaultMode_from_RESET(void);
void enter_DefaultMode_from_RESET(void);

/* Nonzero when a crystal failed to start. SystemCoreClock still reflects the
 * clock actually running, so millis() and baud rates remain correct, but
 * anything derived from the compile-time F_CPU will not match. */
extern volatile uint8_t SystemClockFallback;

/* static inline, not plain inline: these live in a header that is also
 * included from C translation units, where a bare `inline` definition emits
 * no out-of-line copy and fails to link as soon as the compiler decides not
 * to inline it. */
static inline uint32_t millis(void) {
   /* single aligned 32-bit read, atomic on Cortex-M */
   return msTicks;
}

/***************************************************************************//**
 * @brief
 *   Microseconds since reset, wrapping every ~71.6 minutes.
 *
 *   msTicks and the SysTick counter have to be sampled as a consistent pair.
 *   The counter can wrap between the two reads, and the SysTick handler that
 *   would advance msTicks may not have run yet - it is pending, or interrupts
 *   are masked. Sampling with interrupts masked and then folding in the
 *   pending-tick flag (SCB->ICSR PENDSTSET) covers both cases; the previous
 *   read-twice-and-compare version silently returned a value up to one
 *   millisecond in the past whenever it lost that race.
 ******************************************************************************/
static inline uint32_t micros(void)
{
  uint32_t primask = __get_PRIMASK();
  __disable_irq();

  uint32_t load = SysTick->LOAD;          /* reload value = ticksPerMs - 1 */
  uint32_t m    = msTicks;
  uint32_t val  = SysTick->VAL;

  if ((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) != 0u) {
    /* The counter has wrapped but the handler has not run, so msTicks is one
     * behind. Re-read VAL to be sure it belongs to the period after the wrap
     * rather than the one before it. */
    val = SysTick->VAL;
    m++;
  }

  if (primask == 0u) __enable_irq();

  uint32_t elapsed = load - val;          /* SysTick counts down */
  return (m * 1000u) + ((elapsed * 1000u) / (load + 1u));
}

void delay(uint32_t dlyTicks);

#if USE_WDOG > 0
/* Restart the watchdog timeout. Called by the main loop and by delay();
 * a sketch that blocks longer than WDOG_PERIOD elsewhere must call it too. */
void wdogFeed(void);
#endif

/* `start + microseconds > micros()` overflowed: once start was close to
 * UINT32_MAX the sum wrapped to a small value, the condition was false
 * immediately and the delay was skipped entirely. Comparing the unsigned
 * difference against the requested span is wrap-safe.
 *
 * Note the same dependency delay() has: micros() only keeps advancing past
 * the current millisecond while the SysTick interrupt can run, so a delay of
 * more than 1 ms with interrupts masked will not complete. */
static inline void delayMicroseconds(uint32_t microseconds){
  uint32_t start = micros();

  while ((micros() - start) < microseconds) {
  }
}

#ifdef __cplusplus
}  //extern "C"{
#endif /* __cplusplus*/

/*10 cycle*/
#define _delay_loop_2(x)  do{for(uint32_t i=0;i<x;i++) {asm volatile("nop");}}while(0)

#endif /*__ARMCLOCK_H__*/