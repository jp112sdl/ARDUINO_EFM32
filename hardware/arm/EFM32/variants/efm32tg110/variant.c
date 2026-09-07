#include "efm32.h"

/* The clock sources themselves are implemented once in the core
 * (cores/arduino/efm32/efm32init.c). This file only picks one.
 *
 * Previously each variant carried its own copy of the HFRCO band table - with
 * different, partly wrong thresholds - while HFXO_/LFXO_/enter_DefaultMode_
 * from_RESET() were either a no-op or not defined at all. */

void SystemClock_Config(void) __attribute__ ((weak));

void SystemClock_Config(void) {
#if   defined(USE_HFXO)
    HFXO_enter_DefaultMode_from_RESET();   /* external high frequency crystal */
#elif defined(USE_LFXO)
    LFXO_enter_DefaultMode_from_RESET();   /* external low frequency crystal  */
#else                                      /* USE_HFRCO, and the default      */
    enter_DefaultMode_from_RESET();        /* internal high frequency RC      */
#endif
}
