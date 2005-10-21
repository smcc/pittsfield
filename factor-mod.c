#include "libc.h"

/* factor -- print prime factors of n.
   Copyright (C) 86, 1995-2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* Written by Paul Rubin <phr@ocf.berkeley.edu>.
   Adapted for GNU, fixed to factor UINT_MAX by Jim Meyering.  */

#define NDEBUG 1

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "factor"

#define AUTHORS "Paul Rubin"

#define uintmax_t unsigned long long

/* Token delimiters when reading from a file.  */
#define DELIM "\n\t "

/* FIXME: if this program is ever modified to factor integers larger
   than 2^128, this constant (and the algorithm :-) will have to change.  */
#define MAX_N_FACTORS 128

/* The trial divisor increment wheel.  Use it to skip over divisors that
   are composites of 2, 3, 5, 7, or 11.  The part from WHEEL_START up to
   WHEEL_END is reused periodically, while the "lead in" is used to test
   for those primes and to jump onto the wheel.  For more information, see
   http://www.utm.edu/research/primes/glossary/WheelFactorization.html  */

#define WHEEL_SIZE 5

static const unsigned int wheel_tab[] =
  {
/* The first 4 elements correspond to the incremental offsets of the
   first 5 primes (2 3 5 7 11).  The 5(th) element is the
   difference between that last prime and the next largest integer
   that is not a multiple of those primes.  The remaining numbers
   define the wheel.  For more information, see
   http://www.utm.edu/research/primes/glossary/WheelFactorization.html.  */
    1, 2, 2, 4, 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2, 6, 4, 6, 8,
    4, 2, 4, 2, 4, 14, 4, 6, 2, 10, 2, 6, 6, 4, 2, 4, 6, 2, 10, 2, 4, 2, 12,
    10, 2, 4, 2, 4, 6, 2, 6, 4, 6, 6, 6, 2, 6, 4, 2, 6, 4, 6, 8, 4, 2, 4, 6,
    8, 6, 10, 2, 4, 6, 2, 6, 6, 4, 2, 4, 6, 2, 6, 4, 2, 6, 10, 2, 10, 2, 4,
    2, 4, 6, 8, 4, 2, 4, 12, 2, 6, 4, 2, 6, 4, 6, 12, 2, 4, 2, 4, 8, 6, 4, 6,
    2, 4, 6, 2, 6, 10, 2, 4, 6, 2, 6, 4, 2, 4, 2, 10, 2, 10, 2, 4, 6, 6, 2,
    6, 6, 4, 6, 6, 2, 6, 4, 2, 6, 4, 6, 8, 4, 2, 6, 4, 8, 6, 4, 6, 2, 4, 6,
    8, 6, 4, 2, 10, 2, 6, 4, 2, 4, 2, 10, 2, 10, 2, 4, 2, 4, 8, 6, 4, 2, 4,
    6, 6, 2, 6, 4, 8, 4, 6, 8, 4, 2, 4, 2, 4, 8, 6, 4, 6, 6, 6, 2, 6, 6, 4,
    2, 4, 6, 2, 6, 4, 2, 4, 2, 10, 2, 10, 2, 6, 4, 6, 2, 6, 4, 2, 4, 6, 6, 8,
    4, 2, 6, 10, 8, 4, 2, 4, 2, 4, 8, 10, 6, 2, 4, 8, 6, 6, 4, 2, 4, 6, 2, 6,
    4, 6, 2, 10, 2, 10, 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 6, 6, 4, 6,
    8, 4, 2, 4, 2, 4, 8, 6, 4, 8, 4, 6, 2, 6, 6, 4, 2, 4, 6, 8, 4, 2, 4, 2,
    10, 2, 10, 2, 4, 2, 4, 6, 2, 10, 2, 4, 6, 8, 6, 4, 2, 6, 4, 6, 8, 4, 6,
    2, 4, 8, 6, 4, 6, 2, 4, 6, 2, 6, 6, 4, 6, 6, 2, 6, 6, 4, 2, 10, 2, 10, 2,
    4, 2, 4, 6, 2, 6, 4, 2, 10, 6, 2, 6, 4, 2, 6, 4, 6, 8, 4, 2, 4, 2, 12, 6,
    4, 6, 2, 4, 6, 2, 12, 4, 2, 4, 8, 6, 4, 2, 4, 2, 10, 2, 10, 6, 2, 4, 6,
    2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2, 10, 6, 8, 6, 4, 2, 4, 8, 6, 4, 6, 2, 4,
    6, 2, 6, 6, 6, 4, 6, 2, 6, 4, 2, 4, 2, 10, 12, 2, 4, 2, 10, 2, 6, 4, 2,
    4, 6, 6, 2, 10, 2, 6, 4, 14, 4, 2, 4, 2, 4, 8, 6, 4, 6, 2, 4, 6, 2, 6, 6,
    4, 2, 4, 6, 2, 6, 4, 2, 4, 12, 2, 12
  };

#define WHEEL_START (wheel_tab + WHEEL_SIZE)
#define WHEEL_END (wheel_tab + (sizeof wheel_tab / sizeof wheel_tab[0]))

/* The name this program was run with. */
char *program_name;

/* FIXME: comment */

static int
factor (uintmax_t n0, int max_n_factors, uintmax_t *factors)
{
  register uintmax_t n = n0, d, q;
  int n_factors = 0;
  unsigned int const *w = wheel_tab;

  if (n < 1)
    return n_factors;

  /* The exit condition in the following loop is correct because
     any time it is tested one of these 3 conditions holds:
      (1) d divides n
      (2) n is prime
      (3) n is composite but has no factors less than d.
     If (1) or (2) obviously the right thing happens.
     If (3), then since n is composite it is >= d^2. */

  d = 2;
  do
    {
      q = n / d;
      while (n == q * d)
	{
	  factors[n_factors++] = d;
	  n = q;
	  q = n / d;
	}
      d += *(w++);
      if (w == WHEEL_END)
	w = WHEEL_START;
    }
  while (d <= q);

  if (n != 1 || n0 == 1)
    {
      factors[n_factors++] = n;
    }

  return n_factors;
}

/* FIXME: comment */

static int
print_factors (const char *s)
{
  uintmax_t factors[MAX_N_FACTORS];
  uintmax_t n;
  int n_factors;
  int i;

  n = atoll(s);
  n_factors = factor (n, MAX_N_FACTORS, factors);
  printf ("%llu:", n);
  for (i = 0; i < n_factors; i++)
    printf (" %llu", factors[i]);
  putchar ('\n');
  return 0;
}

int
main ()
{
  int argc;
  char **argv;
  int fail = 0;
  char *fake_argv[3];
  argv = fake_argv;

  argv[0] = "factor";
  /*argv[1] = "81064801949712479";*/
  argv[1] = "324259213033341373";
  argv[2] = 0;
  argc = 2;

  program_name = argv[0];

  {
    int i;
      for (i = 1; i < argc; i++)
	fail |= print_factors (argv[i]);
  }

  return fail;
}
