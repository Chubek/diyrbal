#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline uint64_t
splitmix_int_hash64 (uint64_t x)
{
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

static inline uint64_t
fnv1a_str_hash64 (const uint8_t *s)
{
  uint64_t h = 1469598103934665603ULL;
  while (*s)
    {
      h ^= *s++;
      h *= 1099511628211ULL;
    }
  return h;
}

static inline uint64_t
fnv1a_real_hash64 (double key)
{
  uint64_t *ptr = (uint64_t *)&key;
  uint64_t hash = 14695981039346656037U;

  hash ^= *ptr;
  hash *= 1099511628211U;

  return hash;
}

#endif
