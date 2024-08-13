#ifndef __IO_H__
#define __IO_H__


#include "../../../include/types.h"

// static __inline unsigned char
// inb (unsigned short int __port)
// {
//   unsigned char _v;

//   __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (__port));
//   return _v;
// }

// static __inline unsigned short int
// inw (unsigned short int __port)
// {
//   unsigned short _v;

//   __asm__ __volatile__ ("inw %w1,%0":"=a" (_v):"Nd" (__port));
//   return _v;
// }

// static __inline void
// outb (unsigned char __value, unsigned short int __port)
// {
//   __asm__ __volatile__ ("outb %b0,%w1": :"a" (__value), "Nd" (__port));
// }

// static __inline void
// outw (unsigned short int __value, unsigned short int __port)
// {
//   __asm__ __volatile__ ("outw %w0,%w1": :"a" (__value), "Nd" (__port));

// }

// Function: outb - Output a byte to an I/O port
// Parameters:
//   data - The data byte to output
//   port - The I/O port address
void outb(uint8_t data, uint16_t port);

// Function: outw - Output a word to an I/O port
// Parameters:
//   data - The data word to output
//   port - The I/O port address
void outw(uint16_t data, uint16_t port);


// Function: inb - Input a byte from an I/O port
// Parameters:
//   port - The I/O port address
// Returns:
//   The data byte read from the I/O port
uint8_t inb(uint16_t port);

// Function: inb - Input a word from an I/O port
// Parameters:
//   port - The I/O port address
// Returns:
//   The data byte read from the I/O port
uint16_t inw(uint16_t port);

#endif
