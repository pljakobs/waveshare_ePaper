/**
 *  @filename   :   epdif.h
 *  @brief      :   Header file of epdif.cpp providing EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
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

#ifndef EPDIF_H
#define EPDIF_H

#include <arduino.h>


class EpdIf {
public:
    //EpdIf(void);
    EpdIf(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst, int8_t miso, int8_t busy);
    EpdIf(int8_t cs, int8_t dc, int8_t rst);
    ~EpdIf(void);

    static int  IfInit(void);
    static void DigitalWrite(int pin, int value);
    static int  DigitalRead(int pin);
    static void DelayMs(unsigned int delaytime);
    static void SpiTransfer(unsigned char data);
    static void writecommand(unsigned char command);

  protected:
    int8_t _busy;
    boolean  hwSPI;
  #if defined (__AVR__) || defined(TEENSYDUINO)
    uint8_t mySPCR;
    volatile uint8_t *mosiport, *clkport, *dcport, *rsport, *csport;
    int8_t  _cs, _dc, _rst, _mosi, _miso, _sclk;
    uint8_t  mosipinmask, clkpinmask, cspinmask, dcpinmask;
  ////This def is for the Arduino.ORG M0!!!
  //#elif defined(ARDUINO_SAM_ZERO)
  //    volatile PORT_OUT_Type *mosiport, *clkport, *dcport, *rsport, *csport;
  //    int32_t  _cs, _dc, _rst, _mosi, _miso, _sclk;
  //    PORT_OUT_Type  mosipinmask, clkpinmask, cspinmask, dcpinmask;
  #elif defined (__arm__)
      volatile RwReg *mosiport, *clkport, *dcport, *rsport, *csport;
      int32_t  _cs, _dc, _rst, _mosi, _miso, _sclk;
      uint32_t  mosipinmask, clkpinmask, cspinmask, dcpinmask;
  #elif defined (ESP8266)
      int32_t  _cs, _dc, _rst, _mosi, _miso, _sclk;
  #else
      int8_t  _cs, _dc, _rst, _mosi, _miso, _sclk;
  #endif
};

#endif
