/*!
 * @file src/bit_things.h
 * @brief some convenience macros for bit twidling with single byte bit fields
 * @details
 * BB(..) allows defining a Byte as a sequence of Bits, MSB first (example: BB(1,0,0,1,0,1,1,0) = 0x96)
 *
 * setb(x,b) sets the b'th bit in byte x (lower numbered bits are less significant)
 *
 * clrb(x,b) clears the b'th bit in byte x
 *
 * getb(x,b) gets the b'th bit as either 0x01 or 0x00
 *
 * getbas(x,b,p) gets the b'th bit as either zero or a number with a one in the p'th bit position
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef BIT_THINGS_H
#include <stdlib.h>
#define BIT_THINGS_H

#define BB(x7,x6,x5,x4,x3,x2,x1,x0) (x7*0x80 + x6*0x40 + x5*0x20 + x4*0x10 + x3*0x08 + x2*0x04 + x1*0x02 + x0*0x01)

#define setb(x,b) x |= (1<<b)
#define clrb(x,b) x &= ~(1<<b)

#define getb(x,b) ((x & (1<<b)) >> b)
#define getasb(x,b,p) ((b>p) ?  ((x & (1<<b)) >> abs(b-p)) : ((x & (1<<b)) << abs(p-b)))
/* abs is used to avoid compiler warnings about negative shifts (that are never executed) */

#endif
