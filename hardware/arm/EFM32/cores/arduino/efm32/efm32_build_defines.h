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


#ifndef EFM32_BUILD_DEFINES_H
#define EFM32_BUILD_DEFINES_H

#define  ARDUINO_EXTEND 1

#if __has_include("HAL_Conf.h")
# include "HAL_Conf.h"
#endif

#if __has_include("configs/HAL_Conf.h")
# include "configs/HAL_Conf.h"
#endif

/*EFM/STM32 Compatibility definition */
#ifdef DEBUG_EFM
#  ifndef USE_FULL_ASSERT
#    define USE_FULL_ASSERT
#  endif
#endif
#ifdef USE_FULL_ASSERT
#  ifndef DEBUG_EFM
#    define DEBUG_EFM
#  endif
#endif

//default defines,  overriden by HAL_Conf.h  in path sketch or path valiants/valiant/configs/
/***************  HAL_Conf default here ******************/

//OS
#ifndef  FREERTOS
# define FREERTOS  0
#endif

#ifndef  UCOSII	/*running with ucosii*/
# define UCOSII      0
#endif

#ifndef BOOTLOADER	     /*check & go addr if avalible */
# define BOOTLOADER       0
#endif

#ifndef  USE_BITBAND
#define  USE_BITBAND     1
#endif

/* Fault handling.
 * USE_FAULT_RESET: reset the device after a fault instead of spinning in
 * while(1) forever. A silent spin turns every fault into a permanent freeze
 * that looks exactly like a hang; a reset gets the device running again and
 * leaves a trace in RMU->RSTCAUSE. Suppressed automatically while a debugger
 * is attached so faults can still be inspected. Set to 0 for the old
 * spin-forever behaviour. */
#ifndef USE_FAULT_RESET
# define USE_FAULT_RESET  1
#endif

/* Crystal oscillators.
 * HFXO_BOOST is the start-up drive current. The register reset value is
 * 100 %; the old variant code lowered it to 50 %, which is the wrong
 * direction on a start-up path - it saves a little current but makes the
 * crystal less likely to come up. Lower it only for a crystal you have
 * measured. HFXO_STARTUP_SPIN / LFXO_STARTUP_SPIN bound the wait for the
 * oscillator so a missing or dead crystal falls back to the HFRCO instead of
 * hanging inside init(). */
#ifndef HFXO_BOOST
# define HFXO_BOOST          CMU_CTRL_HFXOBOOST_100PCENT
#endif
#ifndef LFXO_BOOST
# define LFXO_BOOST          CMU_CTRL_LFXOBOOST_100PCENT
#endif
#ifndef HFXO_STARTUP_SPIN
# define HFXO_STARTUP_SPIN   2000000UL
#endif
#ifndef LFXO_STARTUP_SPIN
# define LFXO_STARTUP_SPIN   4000000UL
#endif
#ifndef LFXO_FREQ
# define LFXO_FREQ           32768UL
#endif

/* Watchdog (opt-in).
 * When enabled, init() starts the WDOG and the main loop feeds it, as does
 * delay(). A sketch that blocks longer than WDOG_PERIOD outside of delay()
 * has to call wdogFeed() itself. Off by default because it changes the
 * behaviour of existing sketches with long blocking sections.
 * WDOG_PERIOD counts ULFRCO cycles at ~1 kHz, so wdogPeriod_2k is ~2 s. */
#ifndef USE_WDOG
# define USE_WDOG         0
#endif
#ifndef WDOG_PERIOD
# define WDOG_PERIOD      wdogPeriod_2k
#endif
#ifndef WDOG_CLKSEL
# define WDOG_CLKSEL      wdogClkSelULFRCO
#endif
/* Keep the counter stopped in EM2/EM3 by default: a sleeping sketch cannot
 * feed the dog and would otherwise be reset by it. */
#ifndef WDOG_EM2RUN
# define WDOG_EM2RUN      0
#endif
#ifndef WDOG_EM3RUN
# define WDOG_EM3RUN      0
#endif

#ifndef USE_CORECALLBACK
#define USE_CORECALLBACK 0
#endif

#ifndef USE_AVREMULATION
#define USE_AVREMULATION 1
#endif

#ifndef USE_TIMER0_PWM
# define USE_TIMER0_PWM 1
#endif

#ifndef USE_TIMER1_PWM
# define USE_TIMER1_PWM 1
#endif

#ifndef USE_TIMER2_PWM
# define USE_TIMER2_PWM 1
#endif

#ifndef USE_TIMER3_PWM
# define USE_TIMER3_PWM 1
#endif

//devices
//USB
#ifndef USE_USBSERIAL
# ifdef MENU_USB_SERIAL
#   define USE_USBSERIAL 1
#   define USE_USB 1
# endif
#endif

#ifndef USE_USBDMSC
# ifdef MENU_USB_MASS_STORAGE
#   define USE_USBDMSC  1
#   define USE_USB 1
# endif
#endif

#ifndef USE_USBDDFU
# ifdef MENU_USB_DFU
#   define USE_USBDDFU  1
#   define USE_USB 1
# endif
#endif

#ifndef USE_USBDCOMPOSITE
# ifdef MENU_USB_IAD
#	define USE_USBDCOMPOSITE  1
#   define USE_USB 1
# endif
#endif

#ifndef USE_USBDCONF
#  define USE_USBDCONF  1
#endif

//USART
#ifndef USE_USART0
#define USE_USART0  1
#endif
#ifndef USE_USART1
#define USE_USART1  1
#endif
#ifndef USE_USART2
#define USE_USART2  1
#endif
#ifndef USE_USART3
#define USE_USART3  1
#endif
#ifndef USE_USART4
#define USE_USART4  1
#endif
#ifndef USE_USART5
#define USE_USART5  1
#endif
#ifndef USE_UART0
#define USE_UART0   1
#endif
#ifndef USE_UART1
#define USE_UART1   1
#endif

#ifndef USE_LEUART0
#define USE_LEUART0 1
#endif

#ifndef USE_LEUART1
#define USE_LEUART1 1
#endif


//SPI
#ifndef USE_SPI1
#define USE_SPI1 1
#endif

#ifndef USE_SPI2
#define USE_SPI2 1
#endif

#ifndef USE_SPI3
#define USE_SPI3 1
#endif

#ifndef USE_SPI4
#define USE_SPI4 1
#endif

#ifndef USE_SPI5
#define USE_SPI5 1
#endif

#ifndef USE_SPI6
#define USE_SPI6 1
#endif

//I2C
#ifndef USE_I2C1
#define USE_I2C1 1
#endif

#ifndef USE_I2C2
#define USE_I2C2 1
#endif

#ifndef USE_I2C3
#define USE_I2C3 1
#endif

#ifndef USE_I2C4
#define USE_I2C4 1
#endif

/***************  HAL_Conf default end  ******************/


#if __IGNORE

#elif defined(EFM32G222F128)||defined(EFM32G222F64)||defined(EFM32G222F32)
  #define EFM32G  1  
  #define CMSIS_STARTUP_C	"SiliconLabs/EFM32G/Source/startup_efm32g.c"
  #define CMSIS_SYSTEM_C	"SiliconLabs/EFM32G/Source/system_efm32g.c"
  #define CHIP_PIN_LIST PIN(A,0),PIN(A,1),PIN(A,2),PIN(A,8),PIN(A,9),PIN(A,10),PIN(B,7),PIN(B,8),PIN(B,11),PIN(B,13),PIN(B,14),PIN(C,0),PIN(C,1),PIN(C,2),PIN(C,3),PIN(C,4),PIN(C,8),PIN(C,9),PIN(C,10),PIN(C,11),PIN(C,13),PIN(C,14),PIN(C,15),PIN(D,4),PIN(D,5),PIN(D,6),PIN(D,7),PIN(E,10),PIN(E,11),PIN(E,12),PIN(E,13),PIN(F,0),PIN(F,1),PIN(F,2),PIN(F,3),PIN(F,4),PIN(F,5)
#elif defined(EFM32TG110F32)||defined(EFM32TG110F16)
  #define EFM32TG  1  
  #define CMSIS_STARTUP_C   "SiliconLabs/EFM32TG/Source/startup_efm32tg.c"
  #define CMSIS_SYSTEM_C    "SiliconLabs/EFM32TG/Source/system_efm32tg.c"
  #define CHIP_PIN_LIST PIN(A,0),PIN(B,7),PIN(B,8),PIN(B,11),PIN(B,13),PIN(B,14),PIN(C,0),PIN(C,1),PIN(C,14),PIN(C,15),PIN(D,6),PIN(D,7),PIN(E,12),PIN(E,13),PIN(F,0),PIN(F,1),PIN(F,2)
#elif defined(EFM32G210F128)||defined(EFM32G200F64)||defined(EFM32G200F32)||defined(EFM32G200F16)
  #define EFM32G  1  
  #define CMSIS_STARTUP_C	"SiliconLabs/EFM32G/Source/startup_efm32g.c"
  #define CMSIS_SYSTEM_C	"SiliconLabs/EFM32G/Source/system_efm32g.c"
  #define CHIP_PIN_LIST PIN(A,0),PIN(A,1),PIN(A,2),PIN(B,7),PIN(B,8),PIN(B,11),PIN(B,13),PIN(B,14),PIN(C,0),PIN(C,1),PIN(C,13),PIN(C,14),PIN(C,15),PIN(D,4),PIN(D,5),PIN(D,6),PIN(D,7),PIN(E,10),PIN(E,11),PIN(E,12),PIN(E,13),PIN(F,0),PIN(F,1),PIN(F,2)
#elif defined(EFM32G840F128)||defined(EFM32G840F64)||defined(EFM32G840F32)
  #define EFM32G  1  
  #define CMSIS_STARTUP_C	"SiliconLabs/EFM32G/Source/startup_efm32g.c"
  #define CMSIS_SYSTEM_C	"SiliconLabs/EFM32G/Source/system_efm32g.c"
  #define CHIP_PIN_LIST PIN(A,0),PIN(A,1),PIN(A,2),PIN(A,3),PIN(A,4),PIN(A,5),PIN(A,6),PIN(A,12),PIN(A,13),PIN(A,14),PIN(A,15),PIN(B,3),PIN(B,4),PIN(B,5),PIN(B,6),PIN(B,7),PIN(B,8),PIN(B,11),PIN(B,12),PIN(B,13),PIN(B,14),PIN(C,4),PIN(C,5),PIN(C,6),PIN(C,7),PIN(C,12),PIN(C,13),PIN(C,14),PIN(C,15),PIN(D,0),PIN(D,1),PIN(D,2),PIN(D,3),PIN(D,4),PIN(D,5),PIN(D,6),PIN(D,7),PIN(D,8),PIN(E,4),PIN(E,5),PIN(E,6),PIN(E,7),PIN(E,8),PIN(E,9),PIN(E,10),PIN(E,11),PIN(E,12),PIN(E,13),PIN(E,14),PIN(E,15),PIN(F,0),PIN(F,1),PIN(F,2),PIN(F,3),PIN(F,4),PIN(F,5)
#elif defined(EFM32G890F128)||defined(EFM32G890F64)||defined(EFM32G890F32)
  #define EFM32G  1  
  #define CMSIS_STARTUP_C	"SiliconLabs/EFM32G/Source/startup_efm32g.c"
  #define CMSIS_SYSTEM_C	"SiliconLabs/EFM32G/Source/system_efm32g.c"
  #define CHIP_PIN_LIST PIN(A,0),PIN(A,1),PIN(A,2),PIN(A,3),PIN(A,4),PIN(A,5),PIN(A,6),PIN(A,7),PIN(A,8),PIN(A,9),PIN(A,10),PIN(A,11),PIN(A,12),PIN(A,13),PIN(A,14),PIN(A,15),PIN(B,0),PIN(B,1),PIN(B,2),PIN(B,3),PIN(B,4),PIN(B,5),PIN(B,6),PIN(B,7),PIN(B,8),PIN(B,9),PIN(B,10),PIN(B,11),PIN(B,12),PIN(B,13),PIN(B,14),PIN(B,15),PIN(C,0),PIN(C,1),PIN(C,2),PIN(C,3),PIN(C,4),PIN(C,5),PIN(C,6),PIN(C,7),PIN(C,8),PIN(C,9),PIN(C,10),PIN(C,11),PIN(C,12),PIN(C,13),PIN(C,14),PIN(C,15),PIN(D,0),PIN(D,1),PIN(D,2),PIN(D,3),PIN(D,4),PIN(D,5),PIN(D,6),PIN(D,7),PIN(D,8),PIN(D,9),PIN(D,10),PIN(D,11),PIN(D,12),PIN(D,13),PIN(D,14),PIN(D,15),PIN(E,0),PIN(E,1),PIN(E,2),PIN(E,3),PIN(E,4),PIN(E,5),PIN(E,6),PIN(E,7),PIN(E,8),PIN(E,9),PIN(E,10),PIN(E,11),PIN(E,12),PIN(E,13),PIN(E,14),PIN(E,15),PIN(F,0),PIN(F,1),PIN(F,2),PIN(F,3),PIN(F,4),PIN(F,5),PIN(F,6),PIN(F,7),PIN(F,8),PIN(F,9)
#else 
  #error UNKNOWN CHIP 
#endif

#endif
