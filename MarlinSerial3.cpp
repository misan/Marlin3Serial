/*
  HardwareSerial.cpp - Hardware serial library for Wiring
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
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 13 August 2012 by Free Beachler
*/

#include "Marlin.h"

#ifdef P3SUPPORT
#include "MarlinSerial3.h"

#if MOTHERBOARD != 8 // !teensylu
// this next line disables the entire HardwareSerial.cpp, 
// this is so we can support Attiny series and any other chip without a uart?
#if defined(UBRR3H)

p3_ring_buffer p3_rx_buffer  =  { { 0 }, 0, 0 };

FORCE_INLINE void store_char(unsigned char c)
{
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


//#elif defined(SIG_USART_RECV)
#if defined(USART3_RX_vect)
  // fixed by Mark Sproul this is on the 644/644p
  //SIGNAL(SIG_USART_RECV)
  SIGNAL(USART3_RX_vect)
  {
  #if defined(UDR3)
    unsigned char c  =  UDR3;
  //#elif defined(UDR)
  //  unsigned char c  =  UDR;  //  atmega8, atmega32
  #else
    #error UDR3 not defined
  #endif
    store_char(c);
  }
#endif

// Constructors ////////////////////////////////////////////////////////////////

MarlinP3Serial::MarlinP3Serial()
{

}

// Public Methods //////////////////////////////////////////////////////////////

void MarlinP3Serial::begin(long baud)
{
  uint16_t baud_setting;
  bool useU2X3 = true;

#if F_CPU == 16000000UL
  // hardcoded exception for compatibility with the bootloader shipped
  // with the Duemilanove and previous boards and the firmware on the 8U2
  // on the Uno and Mega 2560.
  if (baud == 57600) {
    useU2X3 = false;
  }
#endif
  
  if (useU2X3) {
    UCSR3A = 1 << U2X3;
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  } else {
    UCSR3A = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  UBRR3H = baud_setting >> 8;
  UBRR3L = baud_setting;

  sbi(UCSR3B, RXEN3);
  sbi(UCSR3B, TXEN3);
  sbi(UCSR3B, RXCIE3);
}

void MarlinP3Serial::end()
{
  cbi(UCSR3B, RXEN3);
  cbi(UCSR3B, TXEN3);
  cbi(UCSR3B, RXCIE3);  
}



int MarlinP3Serial::peek(void)
{
  if (p3_rx_buffer.head == p3_rx_buffer.tail) {
    return -1;
  } else {
    return p3_rx_buffer.buffer[p3_rx_buffer.tail];
  }
}

int MarlinP3Serial::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (p3_rx_buffer.head == p3_rx_buffer.tail) {
    return -1;
  } else {
    unsigned char c = p3_rx_buffer.buffer[p3_rx_buffer.tail];
    p3_rx_buffer.tail = (unsigned int)(p3_rx_buffer.tail + 1) % RX_BUFFER_SIZE;
    return c;
  }
}

void MarlinP3Serial::flush()
{
  // don't reverse this or there may be problems if the RX interrupt
  // occurs after reading the value of rx_buffer_head but before writing
  // the value to rx_buffer_tail; the previous value of rx_buffer_head
  // may be written to rx_buffer_tail, making it appear as if the buffer
  // don't reverse this or there may be problems if the RX interrupt
  // occurs after reading the value of rx_buffer_head but before writing
  // the value to rx_buffer_tail; the previous value of rx_buffer_head
  // may be written to rx_buffer_tail, making it appear as if the buffer
  // were full, not empty.
  p3_rx_buffer.head = p3_rx_buffer.tail;
}




/// imports from print.h




void MarlinP3Serial::print(char c, int base)
{
  print((long) c, base);
}

void MarlinP3Serial::print(unsigned char b, int base)
{
  print((unsigned long) b, base);
}

void MarlinP3Serial::print(int n, int base)
{
  print((long) n, base);
}

void MarlinP3Serial::print(unsigned int n, int base)
{
  print((unsigned long) n, base);
}

void MarlinP3Serial::print(long n, int base)
{
  if (base == 0) {
    write(n);
  } else if (base == 10) {
    if (n < 0) {
      print('-');
      n = -n;
    }
    printNumber(n, 10);
  } else {
    printNumber(n, base);
  }
}

void MarlinP3Serial::print(unsigned long n, int base)
{
  if (base == 0) write(n);
  else printNumber(n, base);
}

void MarlinP3Serial::print(double n, int digits)
{
  printFloat(n, digits);
}

void MarlinP3Serial::println(void)
{
  print('\r');
  print('\n');  
}

void MarlinP3Serial::println(const String &s)
{
  print(s);
  println();
}

void MarlinP3Serial::println(const char c[])
{
  print(c);
  println();
}

void MarlinP3Serial::println(char c, int base)
{
  print(c, base);
  println();
}

void MarlinP3Serial::println(unsigned char b, int base)
{
  print(b, base);
  println();
}

void MarlinP3Serial::println(int n, int base)
{
  print(n, base);
  println();
}

void MarlinP3Serial::println(unsigned int n, int base)
{
  print(n, base);
  println();
}

void MarlinP3Serial::println(long n, int base)
{
  print(n, base);
  println();
}

void MarlinP3Serial::println(unsigned long n, int base)
{
  print(n, base);
  println();
}

void MarlinP3Serial::println(double n, int digits)
{
  print(n, digits);
  println();
}

// Private Methods /////////////////////////////////////////////////////////////

void MarlinP3Serial::printNumber(unsigned long n, uint8_t base)
{
  unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars. 
  unsigned long i = 0;

  if (n == 0) {
    print('0');
    return;
  } 

  while (n > 0) {
    buf[i++] = n % base;
    n /= base;
  }

  for (; i > 0; i--)
    print((char) (buf[i - 1] < 10 ?
      '0' + buf[i - 1] :
      'A' + buf[i - 1] - 10));
}

void MarlinP3Serial::printFloat(double number, uint8_t digits) 
{ 
  // Handle negative numbers
  if (number < 0.0)
  {
     print('-');
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i)
    rounding /= 10.0;
  
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0)
    print("."); 

  // Extract digits from the remainder one at a time
  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toPrint = int(remainder);
    print(toPrint);
    remainder -= toPrint; 
  } 
}

// Preinstantiate Objects //////////////////////////////////////////////////////
MarlinP3Serial P3Serial;

#endif // whole file
#endif // port check
#endif //teensylu

