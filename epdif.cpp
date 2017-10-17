/**
 *  @filename   :   epdif.cpp
 *  @brief      :   Implements EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
 *              :   Updates & Enhancements by pjakobs
 *
 *  Copyright (C) Waveshare     August 10 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "epdif.h"
#include <SPI.h>

// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
#ifdef SPI_HAS_TRANSACTION
static inline void spi_begin(void) __attribute__((always_inline));
static inline void spi_begin(void) {
#if defined (ARDUINO_ARCH_ARC32)
  // max speed!
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
#else
    // max speed!
  SPI.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0));
#endif
}
static inline void spi_end(void) __attribute__((always_inline));
static inline void spi_end(void) {
  SPI.endTransaction();
}
#else
#define spi_begin()
#define spi_end()
#endif

// Constructor when using software SPI.  All output pins are configurable.
EpdIf::EpdIf();
EpdIf::EpdIf(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst, int8_t miso, int8_t busy) {
  _cs   = cs;
  _dc   = dc;
  _mosi = mosi;
  _miso = miso;
  _sclk = sclk;
  _rst  = rst;
  _busy = busy;
  hwSPI = false;
}

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
EpdIf::EpdIf(int8_t cs, int8_t dc, int8_t rst){
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  _busy = busy;
  hwSPI = true;
  _mosi  = _sclk = 0;
}

void EpdIf::DigitalWrite(int pin, int value) {
    digitalWrite(pin, value);
}

int EpdIf::DigitalRead(int pin) {
    return digitalRead(pin);
}

void EpdIf::DelayMs(unsigned int delaytime) {
    delay(delaytime);
}

void EpdIf::SpiTransfer(unsigned char data) {
  /*
   * write a single byte to SPI, data or command defined externally.
   */
  #if defined (USE_FAST_PINIO)
    *dcport &= ~dcpinmask;
  #else
    digitalWrite(_dc, LOW);
    digitalWrite(_sclk, LOW);
  #endif

  spiwrite(data);
  #if defined (USE_FAST_PINIO)
    *csport |= cspinmask;
  #else
    digitalWrite(_cs, HIGH);
  #endif
}

void EpdIf::spiwrite(uint8_t c) {

  if (hwSPI) {
    #if defined (__AVR__)
      #ifndef SPI_HAS_TRANSACTION
        uint8_t backupSPCR = SPCR;
        SPCR = mySPCR;
      #endif
      SPDR = c;
      while(!(SPSR & _BV(SPIF)));
      #ifndef SPI_HAS_TRANSACTION
        SPCR = backupSPCR;
      #endif
    #else
      SPI.transfer(c);
    #endif
  } else {
#if defined(ESP8266) || defined (ARDUINO_ARCH_ARC32)
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
      if(c & bit) {
	       digitalWrite(_mosi, HIGH);
      } else {
	       digitalWrite(_mosi, LOW);
      }
      digitalWrite(_sclk, HIGH);
      digitalWrite(_sclk, LOW);
    }
#else
  // Fast SPI bitbang swiped from LPD8806 library
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
      if(c & bit) {
	       //digitalWrite(_mosi, HIGH);
	       *mosiport |=  mosipinmask;
      } else {
	       //digitalWrite(_mosi, LOW);
	       *mosiport &= ~mosipinmask;
      }
      //digitalWrite(_sclk, HIGH);
      *clkport |=  clkpinmask;
      //digitalWrite(_sclk, LOW);
      *clkport &= ~clkpinmask;
    }
#endif
  }
}

void EpdIf::writeCommand(uint8_t c) {
#if defined (USE_FAST_PINIO)
  *dcport &= ~dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, LOW);
  digitalWrite(_sclk, LOW);
  digitalWrite(_cs, LOW);
#endif

  spiwrite(c);

#if defined (USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif
}

void EpdIf::writeData(uint8_t c) {
#if defined (USE_FAST_PINIO)
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#else
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
#endif

  spiwrite(c);

#if defined (USE_FAST_PINIO)
  *csport |= cspinmask;
#else
  digitalWrite(_cs, HIGH);
#endif
}

int EpdIf::IfInit(void) {
  if (_rst > 0) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
  }

  pinMode(_dc, OUTPUT);
  pinMode(_cs, OUTPUT);

#if defined (USE_FAST_PINIO)
  csport    = portOutputRegister(digitalPinToPort(_cs));
  cspinmask = digitalPinToBitMask(_cs);
  dcport    = portOutputRegister(digitalPinToPort(_dc));
  dcpinmask = digitalPinToBitMask(_dc);
#endif

  if(hwSPI) { // Using hardware SPI
    SPI.begin();

#ifndef SPI_HAS_TRANSACTION
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
  #if defined (_AVR__)
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
    mySPCR = SPCR;
  #elif defined(TEENSYDUINO)
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
  #elif defined (__arm__)
    SPI.setClockDivider(11); // 8-ish MHz (full! speed!)
  #endif
#endif
  } else {
    pinMode(_sclk, OUTPUT);
    pinMode(_mosi, OUTPUT);
    pinMode(_miso, INPUT);

#if defined (USE_FAST_PINIO)
    clkport     = portOutputRegister(digitalPinToPort(_sclk));
    clkpinmask  = digitalPinToBitMask(_sclk);
    mosiport    = portOutputRegister(digitalPinToPort(_mosi));
    mosipinmask = digitalPinToBitMask(_mosi);
    *clkport   &= ~clkpinmask;
    *mosiport  &= ~mosipinmask;
#endif
return 0;
}
