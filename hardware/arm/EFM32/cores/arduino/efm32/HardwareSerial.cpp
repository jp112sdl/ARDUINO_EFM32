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

/* Workaround for missing UART_ methods for EFM32G890 */
#ifdef UART0
#undef UART0
#endif

#if defined(USART0)&& (USE_USART0 >0)
USART_Buf_TypeDef* USART0_buf = 0;
#endif
#if defined(USART1)&& (USE_USART1 >0)
USART_Buf_TypeDef* USART1_buf = 0;
#endif
#if defined(USART2)&& (USE_USART2 >0)
USART_Buf_TypeDef* USART2_buf = 0;
#endif
#if defined(USART3)&& (USE_USART3 >0)
USART_Buf_TypeDef* USART3_buf = 0;
#endif
#if defined(USART4)&& (USE_USART4 >0)
USART_Buf_TypeDef* USART4_buf = 0;
#endif
#if defined(USART5)&& (USE_USART5 >0)
USART_Buf_TypeDef* USART5_buf = 0;
#endif
#if  defined(UART0) && (USE_UART0 >0)
USART_Buf_TypeDef* UART0_buf = 0;
#endif
#if  defined(UART1) && (USE_UART1 >0)
USART_Buf_TypeDef* UART1_buf = 0;
#endif
#if defined(LEUART0)&& (USE_LEUART0 >0)
USART_Buf_TypeDef* LEUART0_buf = 0;
#endif
#if defined(LEUART1)&& (USE_LEUART1 >0)
USART_Buf_TypeDef* LEUART1_buf = 0;
#endif

HardwareSerial::HardwareSerial(USART_TypeDef *instance) {
  this->instance = instance;
}

/* Usable before begin(), unlike the buf->mode check. */
bool HardwareSerial::isLeuartInstance(void) {
#if defined(LEUART0) && (USE_LEUART0 >0)
  if (this->instance == (USART_TypeDef *)LEUART0) return true;
#endif
#if defined(LEUART1) && (USE_LEUART1 >0)
  if (this->instance == (USART_TypeDef *)LEUART1) return true;
#endif
  return false;
}

void HardwareSerial::setRouteLoc(uint8_t route) {
      routeLoc = route;
      USART_ROUTE_LOCATION_LOCx = route<<8;
}


/* Synchronous (SPI) setup. Must run *after* initPort(), which turns the
 * peripheral clock on and resets the USART - the registers written here are
 * dropped while the clock is gated and wiped again by a later USART_Reset().
 *
 * Rewritten on top of USART_InitSync(): the hand-rolled version set only
 * USART_CTRL_SYNC and never enabled MASTEREN, TXEN or RXEN in master mode, so
 * a master never clocked anything out. It also derived the baud rate divider
 * from the compile-time F_CPU rather than the live HFPERCLK, and underflowed
 * to a ~0xFFFFFFFF divider (i.e. a stopped clock) for any SPI_BAUDRATE above
 * half the peripheral clock. */
void HardwareSerial::initSpiGpio(USART_Mode_TypeDef  spiMode) {

  USART_TypeDef *spi = this->instance;
  bool master = (spiMode == SPI_MAST_TYPE);

  USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;
  init.master   = master;
  init.baudrate = SPI_BAUDRATE;    /* refFreq stays 0: use the real HFPERCLK */
  init.msbf     = true;
  USART_InitSync(spi, &init);

  spi->IEN = 0;                    /* this driver polls the SPI path */
  spi->IFC = _USART_IFC_MASK;
  spi->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN
             | USART_ROUTE_CSPEN | USART_ROUTE_LOCATION_LOCx;

  /* A master drives MOSI, CLK and CS and listens on MISO; a slave is the
   * other way round. The old code used the master pin directions for both. */
  GPIO_Mode_TypeDef gpioModeMosi = master ? gpioModePushPull : gpioModeInput;
  GPIO_Mode_TypeDef gpioModeMiso = master ? gpioModeInput    : gpioModePushPull;
  GPIO_Mode_TypeDef gpioModeCs   = master ? gpioModePushPull : gpioModeInputPull;
  GPIO_Mode_TypeDef gpioModeClk  = master ? gpioModePushPull : gpioModeInput;
  const unsigned int csIdle      = 1u;   /* CS is active low, so it idles high */

  /* IO configuration. Every branch used to read the pin number from the
   * AF_USART0_* macros regardless of which USART it was configuring. */
#if defined(USART0) && (USE_USART0 >0)
  if(spi == USART0){
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART0_TX_PORT(routeLoc), AF_USART0_TX_PIN(routeLoc), gpioModeMosi, 0);       /* TX/MOSI */
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART0_RX_PORT(routeLoc), AF_USART0_RX_PIN(routeLoc), gpioModeMiso, 0);       /* RX/MISO */
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART0_CS_PORT(routeLoc), AF_USART0_CS_PIN(routeLoc), gpioModeCs,   csIdle);  /* CS */
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART0_CLK_PORT(routeLoc),AF_USART0_CLK_PIN(routeLoc),gpioModeClk,  0);       /* Clock */
  }
#endif
#if defined(USART1) && (USE_USART1 >0)
  if(spi == USART1){
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART1_TX_PORT(routeLoc), AF_USART1_TX_PIN(routeLoc), gpioModeMosi, 0);
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART1_RX_PORT(routeLoc), AF_USART1_RX_PIN(routeLoc), gpioModeMiso, 0);
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART1_CS_PORT(routeLoc), AF_USART1_CS_PIN(routeLoc), gpioModeCs,   csIdle);
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART1_CLK_PORT(routeLoc),AF_USART1_CLK_PIN(routeLoc),gpioModeClk,  0);
  }
#endif
#if defined(USART2) && (USE_USART2 >0)
  if(spi == USART2){
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART2_TX_PORT(routeLoc), AF_USART2_TX_PIN(routeLoc), gpioModeMosi, 0);
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART2_RX_PORT(routeLoc), AF_USART2_RX_PIN(routeLoc), gpioModeMiso, 0);
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART2_CS_PORT(routeLoc), AF_USART2_CS_PIN(routeLoc), gpioModeCs,   csIdle);
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART2_CLK_PORT(routeLoc),AF_USART2_CLK_PIN(routeLoc),gpioModeClk,  0);
  }
#endif
#if defined(USART3) && (USE_USART3 >0)
  if(spi == USART3){
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART3_TX_PORT(routeLoc), AF_USART3_TX_PIN(routeLoc), gpioModeMosi, 0);
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART3_RX_PORT(routeLoc), AF_USART3_RX_PIN(routeLoc), gpioModeMiso, 0);
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART3_CS_PORT(routeLoc), AF_USART3_CS_PIN(routeLoc), gpioModeCs,   csIdle);
	GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART3_CLK_PORT(routeLoc),AF_USART3_CLK_PIN(routeLoc),gpioModeClk,  0);
  }
#endif
}

void HardwareSerial::initSerialGpio(void) {

#if  defined(USART0)&& (USE_USART0 >0)
  if (this->instance == USART0) {
	 
    /* To avoid false start, configure output as high */
    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART0_TX_PORT(this->routeLoc),
                    AF_USART0_TX_PIN(this->routeLoc),
                    gpioModePushPull, 1);

    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART0_RX_PORT(this->routeLoc),
                    AF_USART0_RX_PIN(this->routeLoc),
                    gpioModeInputPull, 0);
	buf->mode = USART_TYPE;
    }
#endif

#if   defined(USART1)&& (USE_USART1 >0)
  if (this->instance == USART1) {
    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART1_TX_PORT(this->routeLoc),
                    AF_USART1_TX_PIN(this->routeLoc),
                    gpioModePushPull, 1);

    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART1_RX_PORT(this->routeLoc),
                    AF_USART1_RX_PIN(this->routeLoc),
                    gpioModeInputPull, 0);
	buf->mode = USART_TYPE;
  }
#endif

#if   defined(USART2)&& (USE_USART2 >0)
  if (this->instance == USART2) {
    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART2_TX_PORT(this->routeLoc),
                    AF_USART2_TX_PIN(this->routeLoc),
                    gpioModePushPull, 1);

    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART2_RX_PORT(this->routeLoc),
                    AF_USART2_RX_PIN(this->routeLoc),
                    gpioModeInputPull, 0);
	buf->mode = USART_TYPE;
  }
#endif

#if   defined(USART3)&& (USE_USART3 >0)
  if (this->instance == USART3) {
    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART3_TX_PORT(this->routeLoc),
                    AF_USART3_TX_PIN(this->routeLoc),
                    gpioModePushPull, 1);

    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART3_RX_PORT(this->routeLoc),
                    AF_USART3_RX_PIN(this->routeLoc),
                    gpioModeInputPull, 0);
	buf->mode = USART_TYPE;
  }
#endif

#if   defined(USART4)&& (USE_USART4 >0)
  if (this->instance == USART4) {
    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART4_TX_PORT(this->routeLoc),
                    AF_USART4_TX_PIN(this->routeLoc),
                    gpioModePushPull, 1);

    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART4_RX_PORT(this->routeLoc),
                    AF_USART4_RX_PIN(this->routeLoc),
                    gpioModeInputPull, 0);
	buf->mode = USART_TYPE;
  }
#endif

#if   defined(USART5)&& (USE_USART5 >0)
  if (this->instance == USART5) {
    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART5_TX_PORT(this->routeLoc),
                    AF_USART5_TX_PIN(this->routeLoc),
                    gpioModePushPull, 1);

    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_USART5_RX_PORT(this->routeLoc),
                    AF_USART5_RX_PIN(this->routeLoc),
                    gpioModeInputPull, 0);
	buf->mode = USART_TYPE;
  }
#endif
#if   defined(UART0)&& (USE_UART0 >0)
  if (this->instance == UART0) {
    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_UART0_TX_PORT(this->routeLoc),
                    AF_UART0_TX_PIN(this->routeLoc),
                    gpioModePushPull, 1);

    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_UART0_RX_PORT(this->routeLoc),
                    AF_USART0_RX_PIN(this->routeLoc),
                    gpioModeInputPull, 0);
	buf->mode = UART_TYPE;
  }
#endif
#if   defined(UART1)&& (USE_UART1 >0)
  if (this->instance == UART1) {
    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_UART1_TX_PORT(this->routeLoc),
                    AF_UART1_TX_PIN(this->routeLoc),
                    gpioModePushPull, 1);

    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_UART1_RX_PORT(this->routeLoc),
                    AF_UART1_RX_PIN(this->routeLoc),
                    gpioModeInputPull, 0);
	buf->mode = UART_TYPE;
  }
#endif
#if defined(LEUART0)&& (USE_LEUART0 >0)
  if (this->instance == (USART_TypeDef *)LEUART0) {
    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_LEUART0_TX_PORT(this->routeLoc),
                    AF_LEUART0_TX_PIN(this->routeLoc),
                    gpioModePushPull, 1);

    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_LEUART0_RX_PORT(this->routeLoc),
                    AF_LEUART0_RX_PIN(this->routeLoc),
                    gpioModeInputPull, 0);
	buf->mode = LEUART_TYPE;
  }
#endif
#if   defined(LEUART1)&& (USE_LEUART1 >0)
  if (this->instance == (USART_TypeDef *)LEUART1) {
    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_LEUART1_TX_PORT(this->routeLoc),
                    AF_LEUART1_TX_PIN(this->routeLoc),
                    gpioModePushPull, 1);

    GPIO_PinModeSet((GPIO_Port_TypeDef)AF_LEUART1_RX_PORT(this->routeLoc),
                    AF_LEUART1_RX_PIN(this->routeLoc),
                    gpioModeInputPull, 0);
	buf->mode = LEUART_TYPE;
  }
#endif
}

/* USART_Buf_TypeDef declares default member initialisers, but a raw malloc()
 * never runs them, so the ring indices used to start out as heap garbage. After
 * a warm reset that garbage survives in RAM and flush()/write() block forever.
 * Allocate zeroed and set every field explicitly, and report a failed
 * allocation instead of dereferencing NULL. */
bool HardwareSerial::allocBuffer(USART_Mode_TypeDef initialMode) {
  if (buf != NULL) return true;

  buf = (USART_Buf_TypeDef*)calloc(1, sizeof(USART_Buf_TypeDef));
  if (buf == NULL) return false;   /* out of heap: leave the port uninitialised */

  buf->instance = this->instance;
  buf->mode     = initialMode;
  buf->txStart  = 0;
  buf->txEnd    = 0;
  buf->rxStart  = 0;
  buf->rxEnd    = 0;
  return true;
}

void HardwareSerial::begin(USART_Mode_TypeDef  spiMode){
  if(spiMode & SPI_MODE ){
     /* The LEUART has no synchronous mode and a different register layout;
      * running the USART SPI setup on it would just scribble over it. */
     if (isLeuartInstance()) return;

     if (!allocBuffer(spiMode)) return;
     /* Clock on and peripheral reset first. The other order wrote the SPI
      * configuration into a gated peripheral (dropped), and initPort()'s
      * USART_Reset() then cleared whatever had survived. */
     initPort();
     /* spiMode, not the SPI_MODE mask - passing the mask made the
      * `spiMode == SPI_MAST_TYPE` test inside always false, so a master was
      * configured as a slave. */
     initSpiGpio(spiMode);
  } else {
	 begin(SERIAL_BAUDRATE,SERIAL_8N1); /**/
  }
}

void HardwareSerial::begin(const uint32_t baud,uint8_t config ){
  this->baud = baud;
  this->config = config;
  /* mode is filled in by initSerialGpio() once the instance is identified */
  if (!allocBuffer(USART_MODE_NONE)) return;
  initSerialGpio();
  initPort();

  if (buf->mode == USART_TYPE) {
    USART_InitAsync_TypeDef initasync = USART_INITASYNC_DEFAULT;
    initasync.baudrate = baud;
    initasync.databits = usartDatabits8;

    if (config < 0x0f) {
      initasync.parity = usartNoParity;   /* No parity. */
    } else if (config < 0x2f) {
      initasync.parity = usartEvenParity; /* Even parity. */
    } else if (config < 0x3f) {
      initasync.parity =  usartOddParity; /* Odd parity. */
    }

    if ((config & 0x0f) == 0x0E) {
      initasync.stopbits = usartStopbits2; /* 1 stopbits. */
    } else {
      initasync.stopbits = usartStopbits1; /* 2 stopbits. */
    }

    initasync.oversampling = usartOVS16;
#if defined( USART_INPUT_RXPRS ) && defined( USART_CTRL_MVDIS )
    initasync.mvdis = 0;
    initasync.prsRxEnable = 0;
    initasync.prsRxCh = usartPrsRxCh0;
#endif
    USART_InitAsync(this->instance, &initasync);
    this->instance->IFC = _USART_IFC_MASK;
	this->instance->IEN = USART_IEN_RXDATAV;
    this->instance->ROUTE |=  USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOCx;
  }
  
  if (buf->mode == UART_TYPE) {
	  /*add me*/
  }
}

void HardwareSerial::initPort(void) {
  
#if  defined(USART0)&& (USE_USART0 >0)
  if (this->instance == USART0) {
    USART0_buf = buf;
    /* the clock must run before any register of the peripheral is touched,
     otherwise the reset below is silently dropped */
    CMU_ClockEnable(cmuClock_USART0, true);
    USART_Reset(USART0);
  }
#endif

#if   defined(USART1)&& (USE_USART1 >0)
  if (this->instance == USART1) {
    USART1_buf = buf;
    /* the clock must run before any register of the peripheral is touched,
     otherwise the reset below is silently dropped */
    CMU_ClockEnable(cmuClock_USART1, true);
    USART_Reset(USART1);
  }
#endif

#if   defined(USART2)&& (USE_USART2 >0)
  if (this->instance == USART2) {
    USART2_buf = buf;
    /* the clock must run before any register of the peripheral is touched,
     otherwise the reset below is silently dropped */
    CMU_ClockEnable(cmuClock_USART2, true);
    USART_Reset(USART2);
  }
#endif

#if   defined(USART3)&& (USE_USART3 >0)
  if (this->instance == USART3) {
    USART3_buf = buf;
    /* the clock must run before any register of the peripheral is touched,
     otherwise the reset below is silently dropped */
    CMU_ClockEnable(cmuClock_USART3, true);
    USART_Reset(USART3);
  }
#endif

#if   defined(USART4)&& (USE_USART4 >0)
  if (this->instance == USART4) {
    USART4_buf = buf;
    /* the clock must run before any register of the peripheral is touched,
     otherwise the reset below is silently dropped */
    CMU_ClockEnable(cmuClock_USART4, true);
    USART_Reset(USART4);
  }
#endif

#if   defined(USART5)&& (USE_USART5 >0)
  if (this->instance == USART5) {
    USART5_buf = buf;
    /* the clock must run before any register of the peripheral is touched,
     otherwise the reset below is silently dropped */
    CMU_ClockEnable(cmuClock_USART5, true);
    USART_Reset(USART5);
  }
#endif
#if   defined(UART0)&& (USE_UART0 >0)
  if (this->instance == UART0) {
    UART0_buf = buf;
    /* the clock must run before any register of the peripheral is touched,
     otherwise the reset below is silently dropped */
    CMU_ClockEnable(cmuClock_UART0, true);
    UART_Reset(UART0);
  }
#endif
#if   defined(UART1)&& (USE_UART1 >0)
  if (this->instance == UART1) {
    UART0_buf = buf;
    /* the clock must run before any register of the peripheral is touched,
     otherwise the reset below is silently dropped */
    CMU_ClockEnable(cmuClock_UART1, true);
    UART_Reset(UART1);
  }
#endif
#if defined(LEUART0)&& (USE_LEUART0 >0)
  if (this->instance == (USART_TypeDef *)LEUART0) {
    LEUART0_buf = buf;
    /* Enable clock for LEUART0 */
  /* Enable CORE LE clock in order to access LE modules */
    CMU_ClockEnable(cmuClock_HFLE, true); // Necessary for accessing LE modules
#if (LESERIAL_BAUDRATE >9600)
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_CORELEDIV2); // Set CORELEDIV2 reference clock
#elif USE_LFBLFXO >0
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);       // Set LFXO reference clock baud <=9600
#elif USE_LFBLFRCO > 0
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFRCO);      // Set LFRCO reference clock baud <=9600
#else
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_CORELEDIV2); // Set CORELEDIV2 reference clock
#endif
    CMU_ClockEnable(cmuClock_LEUART0, true);
 //   CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_1); // Don't prescale LEUART clock

    /* Only now is the LE peripheral reachable: HFCORECLKLE, the LFB source and
     * the module clock all have to be up before its registers are touched.
     * Resetting earlier left a running LEUART configured from a previous run. */
    LEUART_Reset(LEUART0);

    LEUART_Init_TypeDef init = LEUART_INIT_DEFAULT;
    init.enable = leuartDisable;
	init.baudrate = this->baud;
    LEUART_Init(LEUART0, &init);
    LEUART0->IEN = LEUART_IEN_RXDATAV;
    LEUART0->ROUTE = USART_ROUTE_LOCATION_LOCx | LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN;
  }
#endif
#if   defined(LEUART1)&& (USE_LEUART1 >0)
  if (this->instance == (USART_TypeDef *)LEUART1) {
    LEUART1_buf = buf;
    /* Enable clock for LEUART1 */
    CMU_ClockEnable(cmuClock_HFLE, true); // Necessary for accessing LE modules
#if USE_LFBLFXO >0
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO); // Set LFXO reference clock baud <=9600
#elif USE_LFBLFRCO > 0
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFRCO); // Set LFRCO reference clock baud <=9600
#else
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_CORELEDIV2); // Set CORELEDIV2 reference clock
#endif
    CMU_ClockEnable(cmuClock_LEUART1, true);

    /* Reset only after HFCORECLKLE, the LFB source and the module clock are up
     * - see the LEUART0 branch above. */
    LEUART_Reset(LEUART1);

    LEUART_Init_TypeDef init = LEUART_INIT_DEFAULT;
    init.enable = leuartDisable;
	init.baudrate = this->baud;
    LEUART_Init(LEUART1, &init);
    LEUART1->IEN = LEUART_IEN_RXDATAV;
    LEUART1->ROUTE = USART_ROUTE_LOCATION_LOCx | LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN;
  }
#endif
  
#if defined(USART0)&& (USE_USART0 >0)
  if (this->instance == USART0) {
    NVIC_ClearPendingIRQ(USART0_RX_IRQn);
    NVIC_EnableIRQ(USART0_RX_IRQn);
    NVIC_ClearPendingIRQ(USART0_TX_IRQn);
    NVIC_EnableIRQ(USART0_TX_IRQn);
  }
#endif
#if defined(USART1)&& (USE_USART1 >0)
  if (this->instance == USART1) {
    NVIC_ClearPendingIRQ(USART1_RX_IRQn);
    NVIC_EnableIRQ(USART1_RX_IRQn);
    NVIC_ClearPendingIRQ(USART1_TX_IRQn);
    NVIC_EnableIRQ(USART1_TX_IRQn);
  }
#endif
#if defined(USART2)&& (USE_USART2 >0)
  if (this->instance == USART2) {
    NVIC_ClearPendingIRQ(USART2_RX_IRQn);
    NVIC_EnableIRQ(USART2_RX_IRQn);
    NVIC_ClearPendingIRQ(USART2_TX_IRQn);
    NVIC_EnableIRQ(USART2_TX_IRQn);
  }
#endif
#if defined(USART3)&& (USE_USART3 >0)
  if (this->instance == USART3) {
    NVIC_ClearPendingIRQ(USART3_RX_IRQn);
    NVIC_EnableIRQ(USART3_RX_IRQn);
    NVIC_ClearPendingIRQ(USART3_TX_IRQn);
    NVIC_EnableIRQ(USART3_TX_IRQn);
  }
#endif
#if defined(USART4)&& (USE_USART4 >0)
  if (this->instance == USART4) {
    NVIC_ClearPendingIRQ(USART4_RX_IRQn);
    NVIC_EnableIRQ(USART4_RX_IRQn);
    NVIC_ClearPendingIRQ(USART4_TX_IRQn);
    NVIC_EnableIRQ(USART4_TX_IRQn);
  }
#endif
#if defined(USART5)&& (USE_USART5 >0)
  if (this->instance == USART5) {
    NVIC_ClearPendingIRQ(USART5_RX_IRQn);
    NVIC_EnableIRQ(USART5_RX_IRQn);
    NVIC_ClearPendingIRQ(USART5_TX_IRQn);
    NVIC_EnableIRQ(USART5_TX_IRQn);
  }
#endif
#if defined(UART0)&& (USE_UART0 >0)
  if (this->instance == UART0) {
    NVIC_ClearPendingIRQ(UART0_RX_IRQn);
    NVIC_EnableIRQ(UART0_RX_IRQn);
    NVIC_ClearPendingIRQ(UART0_TX_IRQn);
    NVIC_EnableIRQ(UART0_TX_IRQn);
  }
#endif
#if defined(UART1)&& (USE_UART1 >0)
  if (this->instance == UART1) {
    NVIC_ClearPendingIRQ(UART1_RX_IRQn);
    NVIC_EnableIRQ(UART1_RX_IRQn);
    NVIC_ClearPendingIRQ(UART1_TX_IRQn);
    NVIC_EnableIRQ(UART1_TX_IRQn);
  }
#endif
#if defined(LEUART0)&& (USE_LEUART0 >0)
  if (this->instance == (USART_TypeDef *)LEUART0) {
    NVIC_ClearPendingIRQ(LEUART0_IRQn);
    NVIC_EnableIRQ(LEUART0_IRQn);
    LEUART0->IEN = LEUART_IEN_RXDATAV;
    LEUART_Enable(LEUART0, leuartEnable);

  }
#endif
#if defined(LEUART1)&& (USE_LEUART1 >0)
  if (this->instance == (USART_TypeDef *)LEUART1) {
    NVIC_ClearPendingIRQ(LEUART1_IRQn);
    NVIC_EnableIRQ(LEUART1_IRQn);
    LEUART1->IEN = LEUART_IEN_RXDATAV;
    LEUART_Enable(LEUART1, leuartEnable);
  }
#endif
}

void HardwareSerial::end(void) {}

int HardwareSerial::available(void)
{
  if (buf == NULL) return 0;   /* begin() never ran, or the allocation failed */
  return ((unsigned int)(SERIAL_RX_BUFFER_SIZE + buf->rxEnd - buf->rxStart)) % SERIAL_RX_BUFFER_SIZE;
}

int HardwareSerial::availableForWrite(void)
{
  if (buf == NULL) return 0;
  if (buf->txEnd >= buf->txStart) return SERIAL_TX_BUFFER_SIZE - 1 - (buf->txEnd - buf->txStart); /*SERIAL_TX_BUFFER_SIZE-1 ~ 0*/
  return buf->txStart - buf->txEnd - 1;
}

int HardwareSerial::peek(void)
{
  if (available()) {
    return buf->rxBuffer[buf->rxStart];
  } else {
    return -1;
  }
}

/* True while the peripheral can accept another byte right now. */
bool HardwareSerial::txPeripheralReady(void)
{
  if (buf->mode == LEUART_TYPE) {
    return (((LEUART_TypeDef *)buf->instance)->STATUS & LEUART_STATUS_TXBL) != 0;
  }
  return (buf->instance->STATUS & USART_STATUS_TXBL) != 0;
}

/* True once the last byte has actually left the shift register, not merely
 * been handed to the peripheral. */
bool HardwareSerial::txShiftRegisterEmpty(void)
{
  if (buf->mode == LEUART_TYPE) {
    return (((LEUART_TypeDef *)buf->instance)->STATUS & LEUART_STATUS_TXC) != 0;
  }
  return (buf->instance->STATUS & USART_STATUS_TXC) != 0;
}

/* Polled send, bypassing the ring and the TX interrupt. Only call once
 * txPeripheralReady() is true, otherwise the emlib helpers spin unbounded. */
void HardwareSerial::txByteDirect(uint8_t ch)
{
  if (buf->mode == LEUART_TYPE) {
    LEUART_Tx((LEUART_TypeDef *)buf->instance, ch);
  }
#if defined(UART0)||defined(UART1)
  else if (buf->mode == UART_TYPE) {
    UART_Tx(buf->instance, ch);
  }
#endif
  else {
    USART_Tx(buf->instance, ch);
  }
}

/* Push the queued bytes out by polling instead of waiting for the TX
 * interrupt, which cannot run in the contexts this is used from. Interrupts
 * are masked so the TX handler cannot advance txStart concurrently. Bounded:
 * gives up if the peripheral stops shifting. Returns true if at least one
 * ring slot was freed. */
bool HardwareSerial::drainTxPolled(void)
{
  bool progress = false;
  uint32_t primask = __get_PRIMASK();
  __disable_irq();

  while (buf->txStart != buf->txEnd) {
    uint32_t spin = SERIAL_TX_POLL_SPIN;
    while (!txPeripheralReady() && spin) spin--;
    if (spin == 0) break;                 /* peripheral is not draining */

    txByteDirect(buf->txBuffer[buf->txStart]);
    buf->txStart = (buf->txStart + 1) % SERIAL_TX_BUFFER_SIZE;
    progress = true;
  }

  if (primask == 0) __enable_irq();
  return progress;
}

/* Wait for one free slot in the transmit ring. Returns false if it gave up,
 * in which case the caller drops the byte rather than hanging. */
bool HardwareSerial::waitTxSpace(void)
{
  if (availableForWrite() != 0) return true;

  /* No point waiting for the TX interrupt here - it cannot preempt us. */
  if (isInterrupt() || (__get_PRIMASK() != 0)) {
    drainTxPolled();
    return availableForWrite() != 0;
  }

  uint32_t start = millis();
  uint32_t guard = SERIAL_TX_POLL_SPIN;   /* backstop if msTicks is stalled */
  while (availableForWrite() == 0) {
    if ((millis() - start) >= SERIAL_TX_TIMEOUT_MS) break;
    if (--guard == 0) break;
    yield();
  }
  if (availableForWrite() != 0) return true;

  drainTxPolled();                        /* last resort before dropping */
  return availableForWrite() != 0;
}

void HardwareSerial::flush(void) {
  if (buf == NULL) return;

  /* Step 1: get the ring empty. */
  if (isInterrupt() || (__get_PRIMASK() != 0)) {
    drainTxPolled();
  } else {
    uint32_t start = millis();
    uint32_t guard = SERIAL_TX_POLL_SPIN;
    while (buf->txEnd != buf->txStart) {
      if ((millis() - start) >= SERIAL_TX_TIMEOUT_MS) { drainTxPolled(); break; }
      if (--guard == 0)                              { drainTxPolled(); break; }
      yield();
    }
  }

  /* Step 2: wait for the last byte to leave the shift register. Arduino's
   * flush() means "transmission finished"; stopping at an empty ring returned
   * with a byte still going out, so a sketch that flushed and then powered
   * down or reconfigured the pins truncated it. */
  uint32_t spin = SERIAL_TX_POLL_SPIN;
  while (!txShiftRegisterEmpty() && spin) spin--;
}

int HardwareSerial::read(void) {
  if (available()) {
    uint8_t rtn =  buf->rxBuffer[buf->rxStart];
    /* single store: txStart++ followed by %= briefly exposed an out-of-range
       index to the interrupt handler */
    buf->rxStart = (buf->rxStart + 1) % SERIAL_RX_BUFFER_SIZE;
    return rtn;
  } else {
    return -1;
  }
}

size_t HardwareSerial::write(unsigned char ch) {
  /* Must be checked before the availableForWrite() spin below, which would
   * otherwise never terminate on an unopened port. */
  if (buf == NULL) return 0;
#if DEBUG_EFM /*for debug unused interrupt*/
    if (buf->mode == LEUART_TYPE) {
      LEUART_Tx((LEUART_TypeDef *)buf->instance, ch);
    }
  #if defined(UART0)||defined(UART1)
    else if(buf->mode == UART_TYPE){
      UART_Tx(buf->instance, ch);
    }
  #endif
    else {
      USART_Tx(buf->instance, ch);
    }

#else
  if (!waitTxSpace()) return 0;  /* drop the byte instead of hanging forever */
  buf->txBuffer[buf->txEnd] = ch;
  buf->txEnd = (buf->txEnd + 1) % SERIAL_TX_BUFFER_SIZE;
  if (buf->mode == LEUART_TYPE) {
    LEUART_IntEnable((LEUART_TypeDef *)buf->instance, LEUART_IEN_TXC);
    LEUART_IntSet((LEUART_TypeDef *)buf->instance, LEUART_IFS_TXC);
  }
#if defined(UART0)||defined(UART1)
  else if(buf->mode == UART_TYPE){
    UART_IntEnable(buf->instance, UART_IEN_TXC);
    UART_IntSet(buf->instance, UART_IFS_TXC);
  }
#endif
  else {
    USART_IntEnable(buf->instance, USART_IEN_TXC);
    USART_IntSet(buf->instance, USART_IFS_TXC);
  }
#endif  
  return 1;
}

void USART_TXCallback(USART_Buf_TypeDef *interruptUART) {
  if (interruptUART->txStart != interruptUART->txEnd) {
    USART_Tx(interruptUART->instance, interruptUART->txBuffer[interruptUART->txStart]);
    /* single store: txStart++ followed by %= briefly exposed an
       out-of-range index to availableForWrite() */
    interruptUART->txStart = (interruptUART->txStart + 1) % SERIAL_TX_BUFFER_SIZE;
  } else {
    /* Ring is empty. TXC stayed enabled forever before, so every byte
       completing anywhere cost an extra interrupt for nothing.
       write() re-enables it when there is something to send. */
    USART_IntDisable(interruptUART->instance, USART_IEN_TXC);
  }
}
void USART_RXCallback(USART_Buf_TypeDef *interruptUART) {
  /* RXDATAV is not in the peripheral's IFC mask, so it clears only by reading
   * RXDATA. Never leave the handler without reading, or the NVIC re-enters it
   * immediately and the CPU never returns to thread mode.
   *
   * Drain everything that has arrived rather than one byte per interrupt: at
   * 115200 baud that saves an entry per character without the loop ever
   * running long, since it stops as soon as the receiver is empty. */
  do {
    uint8_t data = USART_Rx(interruptUART->instance);
    unsigned int next = (interruptUART->rxEnd + 1) % SERIAL_RX_BUFFER_SIZE;
    if (next == interruptUART->rxStart) break;   /* rx buffer full: drop it */
    interruptUART->rxBuffer[interruptUART->rxEnd] = data;
    interruptUART->rxEnd = next;
  } while ((interruptUART->instance->STATUS & USART_STATUS_RXDATAV));
}
#if defined(USART0) && (USE_USART0 >0)
void (*usart0_rxCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_RXCallback;
void (*usart0_txCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_TXCallback;
extern "C"
void USART0_RX_IRQHandler(void)
{
    uint32_t flags = USART_IntGet(USART0);
    USART_IntClear(USART0, flags);
    if (flags & USART_IF_RXDATAV) usart0_rxCallbBck(USART0_buf);
}
extern "C"
void USART0_TX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART0);
  USART_IntClear(USART0, flags);
  if (flags & USART_IF_TXC) usart0_txCallbBck(USART0_buf);
//  if (flags & USART_IF_TXC) USART_TXCallback(USART0_buf);
}
HardwareSerial SerialUSART0(USART0);
#endif

#if defined(USART1) && (USE_USART1 >0)
void (*usart1_rxCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_RXCallback;
void (*usart1_txCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_TXCallback;
extern "C"
void USART1_RX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART1);
  USART_IntClear(USART1, flags);
  if (flags & USART_IF_RXDATAV) usart1_rxCallbBck(USART1_buf);
}
extern "C"
void USART1_TX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART1);
  USART_IntClear(USART1, flags);
  if (flags & USART_IF_TXC) usart1_txCallbBck(USART1_buf);
}
HardwareSerial SerialUSART1(USART1);
#endif

#if defined(USART2) && (USE_USART2 >0)
void (*usart2_rxCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_RXCallback;
void (*usart2_txCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_TXCallback;
extern "C"
void USART2_RX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART2);
  USART_IntClear(USART2, flags);
  if (flags & USART_IF_RXDATAV) usart2_rxCallbBck(USART2_buf);
}
extern "C"
void USART2_TX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART2);
  USART_IntClear(USART2, flags);
  if (flags & USART_IF_TXC) usart2_txCallbBck(USART2_buf);
}
HardwareSerial SerialUSART2(USART2);
#endif

#if defined(USART3) && (USE_USART3 >0)
void (*usart3_rxCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_RXCallback;
void (*usart3_txCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_TXCallback;
extern "C"
void USART3_RX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART3);
  USART_IntClear(USART3, flags);
  if (flags & USART_IF_RXDATAV) usart3_rxCallbBck(USART3_buf);
}
extern "C"
void USART3_TX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART3);
  USART_IntClear(USART3, flags);
  if (flags & USART_IF_TXC) usart3_txCallbBck(USART3_buf);
}
HardwareSerial SerialUSART3(USART3);
#endif

#if defined(USART4) && (USE_USART4 >0)
void (*usart4_rxCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_RXCallback;
void (*usart4_txCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_TXCallback;
extern "C"
void USART4_RX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART4);
  USART_IntClear(USART4, flags);
  if (flags & USART_IF_RXDATAV) usart4_rxCallbBck(USART4_buf);
}
extern "C"
void USART4_TX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART4);
  USART_IntClear(USART4, flags);
  if (flags & USART_IF_TXC) usart4_txCallbBck(USART4_buf);
}
HardwareSerial SerialUSART4(USART4);
#endif

#if defined(USART5) && (USE_USART5 >0)
void (*usart5_rxCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_RXCallback;
void (*usart5_txCallbBck)(USART_Buf_TypeDef *interruptUART) = USART_TXCallback;
extern "C"
void USART5_RX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART5);
  USART_IntClear(USART5, flags);
  if (flags & USART_IF_RXDATAV) usart5_rxCallbBck(USART5_buf);
}
extern "C"
void USART5_TX_IRQHandler(void)
{
  uint32_t flags = USART_IntGet(USART5);
  USART_IntClear(USART5, flags);
  if (flags & USART_IF_TXC) usart5_txCallbBck(USART5_buf);
}
HardwareSerial SerialUSART5(USART5);
#endif

#if defined(UART0)||defined(UART1)
void UART_TXCallback(USART_Buf_TypeDef *interruptUART) {
  if (interruptUART->txStart != interruptUART->txEnd) {
    UART_Tx((UART_TypeDef *)interruptUART->instance, interruptUART->txBuffer[interruptUART->txStart]);
    /* single store: txStart++ followed by %= briefly exposed an
       out-of-range index to availableForWrite() */
    interruptUART->txStart = (interruptUART->txStart + 1) % SERIAL_TX_BUFFER_SIZE;
  } else {
    /* Ring is empty. TXC stayed enabled forever before, so every byte
       completing anywhere cost an extra interrupt for nothing.
       write() re-enables it when there is something to send. */
    UART_IntDisable((UART_TypeDef *)interruptUART->instance, UART_IEN_TXC);
  }
}
void UART_RXCallback(USART_Buf_TypeDef *interruptUART) {
  /* RXDATAV is not in the peripheral's IFC mask, so it clears only by reading
   * RXDATA. Never leave the handler without reading, or the NVIC re-enters it
   * immediately and the CPU never returns to thread mode.
   *
   * Drain everything that has arrived rather than one byte per interrupt: at
   * 115200 baud that saves an entry per character without the loop ever
   * running long, since it stops as soon as the receiver is empty. */
  do {
    uint8_t data = USART_Rx((UART_TypeDef *)interruptUART->instance);
    unsigned int next = (interruptUART->rxEnd + 1) % SERIAL_RX_BUFFER_SIZE;
    if (next == interruptUART->rxStart) break;   /* rx buffer full: drop it */
    interruptUART->rxBuffer[interruptUART->rxEnd] = data;
    interruptUART->rxEnd = next;
  } while ((((UART_TypeDef *)interruptUART->instance)->STATUS & USART_STATUS_RXDATAV));
}
#if defined(UART0) && (USE_UART0 >0)
void (*uart0_rxCallbBck)(USART_Buf_TypeDef *interruptUART) = UART_RXCallback;
void (*uart0_txCallbBck)(USART_Buf_TypeDef *interruptUART) = UART_TXCallback;
extern "C"
void UART0_RX_IRQHandler(void)
{
  uint32_t flags = UART_IntGet(UART0);
  UART_IntClear(UART0, flags);
  if (flags & UART_IF_RXDATAV) uart0_rxCallbBck(UART0_buf);
}
extern "C"
void UART0_TX_IRQHandler(void)
{
  uint32_t flags = UART_IntGet(UART0);
  UART_IntClear(UART0, flags);
  if (flags & UART_IF_TXC) uart0_txCallbBck(UART0_buf);
}
HardwareSerial SerialUART0((USART_TypeDef *)UART0);
#  endif
#  if defined(UART1) && (USE_UART1 >0)
void (*uart1_rxCallbBck)(USART_Buf_TypeDef *interruptUART) = UART_RXCallback;
void (*uart1_txCallbBck)(USART_Buf_TypeDef *interruptUART) = UART_TXCallback;
extern "C"
void UART1_RX_IRQHandler(void)
{
  uint32_t flags = UART_IntGet(UART1);
  UART_IntClear(UART1, flags);
  if (flags & UART_IF_RXDATAV) uart1_rxCallbBck(UART1_buf);
}
extern "C"
void UART1_TX_IRQHandler(void)
{
  uint32_t flags = UART_IntGet(UART1);
  UART_IntClear(UART1, flags);
  if (flags & UART_IF_TXC) uart1_txCallbBck(UART1_buf);
}
HardwareSerial SerialUART1((USART_TypeDef *)UART1);
#  endif
#endif


#if (defined(LEUART0)&&(USE_LEUART0 >0))||(defined(LEUART1)&&(USE_LEUART1 >0))
void LEUART_TXCallback(USART_Buf_TypeDef *interruptUART) {
  if (interruptUART->txStart != interruptUART->txEnd) {
    LEUART_Tx((LEUART_TypeDef *)interruptUART->instance, interruptUART->txBuffer[interruptUART->txStart]);
    /* single store: txStart++ followed by %= briefly exposed an
       out-of-range index to availableForWrite() */
    interruptUART->txStart = (interruptUART->txStart + 1) % SERIAL_TX_BUFFER_SIZE;
  } else {
    /* Ring is empty. TXC stayed enabled forever before, so every byte
       completing anywhere cost an extra interrupt for nothing.
       write() re-enables it when there is something to send. */
    LEUART_IntDisable((LEUART_TypeDef *)interruptUART->instance, LEUART_IEN_TXC);
  }
}
void LEUART_RXCallback(USART_Buf_TypeDef *interruptUART) {
  /* RXDATAV is not in the peripheral's IFC mask, so it clears only by reading
   * RXDATA. Never leave the handler without reading, or the NVIC re-enters it
   * immediately and the CPU never returns to thread mode.
   *
   * Drain everything that has arrived rather than one byte per interrupt: at
   * 115200 baud that saves an entry per character without the loop ever
   * running long, since it stops as soon as the receiver is empty. */
  do {
    uint8_t data = LEUART_Rx((LEUART_TypeDef *)interruptUART->instance);
    unsigned int next = (interruptUART->rxEnd + 1) % SERIAL_RX_BUFFER_SIZE;
    if (next == interruptUART->rxStart) break;   /* rx buffer full: drop it */
    interruptUART->rxBuffer[interruptUART->rxEnd] = data;
    interruptUART->rxEnd = next;
  } while ((((LEUART_TypeDef *)interruptUART->instance)->STATUS & LEUART_STATUS_RXDATAV));
}
#if defined(LEUART0) && (USE_LEUART0 >0)
void (*leuart0_rxCallbBck)(USART_Buf_TypeDef *interruptUART) = LEUART_RXCallback;
void (*leuart0_txCallbBck)(USART_Buf_TypeDef *interruptUART) = LEUART_TXCallback;

extern "C"
void LEUART0_IRQHandler(void)
{
  uint32_t flags = LEUART_IntGet(LEUART0);
  LEUART_IntClear(LEUART0, flags);
  if (flags & LEUART_IF_RXDATAV) leuart0_rxCallbBck(LEUART0_buf);
  if (flags & LEUART_IF_TXC)     leuart0_txCallbBck(LEUART0_buf);
}
HardwareSerial SerialLEUART0((USART_TypeDef *)LEUART0);
#  endif
#  if defined(LEUART1) && (USE_LEUART1 >0)
void (*leuart1_rxCallbBck)(USART_Buf_TypeDef *interruptUART) = LEUART_RXCallback;
void (*leuart1_txCallbBck)(USART_Buf_TypeDef *interruptUART) = LEUART_TXCallback;
extern "C"
void LEUART1_IRQHandler(void)
{
  uint32_t flags = LEUART_IntGet(LEUART1);
  LEUART_IntClear(LEUART1, flags);
  if (flags & LEUART_IF_RXDATAV) leuart1_rxCallbBck(LEUART1_buf);
  if (flags & LEUART_IF_TXC)     leuart1_txCallbBck(LEUART1_buf);
}
HardwareSerial SerialLEUART1((USART_TypeDef *)LEUART1);
#  endif
#endif
