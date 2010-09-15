/* rabinkarp.h: Hexinspector
   Copyright (c) 2010 Jen Freeman

   $Id$

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

   Author contact information:

   Web: http://github.com/jenf/hexinspector

*/

/**
 * Rabin Karp Hashing
 */


#ifndef RABIN_KARP_H
#define RABIN_KARP_H

#define GOOD_PRIME (8355967)
#define GOOD_BASE  (256)

/* Calculcate the value that a byte needs to be multiplied by to remove it from the hash */
static inline uint32_t rabinkarp_calculate_popvalue(int bytes, uint32_t prime, uint32_t base)
{
  uint32_t popvalue = 1;
  int i;
  for (i = 0; i < bytes; i++)
  {
    popvalue = (popvalue * base) % prime;
  }
  return popvalue;
}

/* Add a byte to the hash */
static inline uint32_t rabinkarp_add(uint32_t hash, uint32_t value, uint32_t prime, uint32_t base)
{
  return (hash*base+value) % prime;
}

/* Remove to the hash */
static inline uint32_t rabinkarp_remove(uint32_t hash, uint32_t value, uint32_t prime, uint32_t popvalue)
{
  return (hash+prime - ((value * popvalue) % prime)) % prime;
}

/* Utility function to roll with it */
static inline uint32_t rabinkarp_roll(uint32_t hash, uint32_t newvalue, uint32_t prevval,
                                      off_t curpos, off_t rolllen, uint32_t prime, uint32_t base, uint32_t popvalue)
{
  if (curpos >= rolllen)
  {
    hash = rabinkarp_remove(hash, prevval, prime, popvalue);
  }
  return rabinkarp_add(hash, newvalue, prime, base);
}
#endif