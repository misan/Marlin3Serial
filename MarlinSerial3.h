/*
  HardwareSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 28 September 2010 by Mark Sproul
  Modified 13 August 2012 by Free Beachler
*/

#include "Marlin.h"
#ifdef P3SUPPORT

#ifndef MarlinP3Serial_h
#define MarlinP3Serial_h

#ifndef DEC
#define DEC 10
#endif
#ifndef HEX
#define HEX 16
#endif
#ifndef OCT
#define OCT 8
#endif
#ifndef BIN
#define BIN 2
#endif
#ifndef BYTE
#define BYTE 0
#endif


#if MOTHERBOARD != 8 // !teensylu
// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which rx_buffer_head is the index of the
// location to which to write the next incoming character and rx_buffer_tail
// is the index of the location from which to read.
#define P3_RX_BUFFER_SIZE 128

struct p3_ring_buffer
{
  unsigned char buffer[P3_RX_BUFFER_SIZE];
  int head;
  int tail;
};

#if defined(UBRR2H)
  extern p3_ring_buffer p3_rx_buffer;
#endif

class MarlinP3Serial //: public Stream
{

  public:
    MarlinP3Serial();
    void begin(long);
    void end();
    int peek(void);
    int read(void);
    void flush(void);
    
    FORCE_INLINE int available(void)
    {
      return (unsigned int)(P3_RX_BUFFER_SIZE + p3_rx_buffer.head - p3_rx_buffer.tail) % P3_RX_BUFFER_SIZE;
    }
    
    FORCE_INLINE void write(uint8_t c)
    {
      while (!((UCSR3A) & (1 << UDRE3)))
        ;

      UDR3 = c;
    }
    
    
    FORCE_INLINE void checkRx(void)
    {
      if((UCSR3A & (1<<RXC3)) != 0) {
        unsigned char c  =  UDR3;
        int i = (unsigned int)(p3_rx_buffer.head + 1) % P3_RX_BUFFER_SIZE;

        // if we should be storing the received character into the location
        // just before the tail (meaning that the head would advance to the
        // current location of the tail), we're about to overflow the buffer
        // and so we don't write the character or advance the head.
        if (i != p3_rx_buffer.tail) {
          p3_rx_buffer.buffer[p3_rx_buffer.head] = c;
          p3_rx_buffer.head = i;
        }
      }
    }
    
    
    private:
    void printNumber(unsigned long, uint8_t);
    void printFloat(double, uint8_t);
    
    
  public:
    
    FORCE_INLINE void write(const char *str)
    {
      while (*str)
        write(*str++);
    }


    FORCE_INLINE void write(const uint8_t *buffer, size_t size)
    {
      while (size--)
        write(*buffer++);
    }

    FORCE_INLINE void print(const String &s)
    {
      for (int i = 0; i < (int)s.length(); i++) {
        write(s[i]);
      }
    }
    
    FORCE_INLINE void print(const char *str)
    {
      write(str);
    }
    void print(char, int = BYTE);
    void print(unsigned char, int = BYTE);
    void print(int, int = DEC);
    void print(unsigned int, int = DEC);
    void print(long, int = DEC);
    void print(unsigned long, int = DEC);
    void print(double, int = 2);

    void println(const String &s);
    void println(const char[]);
    void println(char, int = BYTE);
    void println(unsigned char, int = BYTE);
    void println(int, int = DEC);
    void println(unsigned int, int = DEC);
    void println(long, int = DEC);
    void println(unsigned long, int = DEC);
    void println(double, int = 2);
    void println(void);
};

extern MarlinP3Serial P3Serial;
#endif // !teensylu

#endif

#endif // P3SUPPORT
