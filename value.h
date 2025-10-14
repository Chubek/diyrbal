#ifndef VALUE_H
#define VALUE_H

#include <cstdint>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define QNAN ((uint64_t)0x7ffc000000000000)
#define SIGN_BIT ((uint64_t)0x8000000000000000)

#define TAG_NIL 1
#define TAG_BOOLEAN 2
#define TAG_OBJECT 3

#define IS_NUMBER(value) (((value) & QNAN) != QNAN)

#define IS_NIL(value) ((value) == NIL_VAL)
#define IS_BOOLEAN(value)                                                     \
  (((value) & (QNAN | TAG_BOOLEAN)) == (QNAN | TAG_BOOLEAN))
#define IS_OBJECT(value)                                                      \
  (((value) & (QNAN | TAG_OBJECT)) == (QNAN | TAG_OBJECT))

#define AS_OBJECT(value) ((Obj *)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))
#define AS_BOOLEAN(value) ((value) & 1)

#define NIL_VAL ((diy_value_t)(QNAN | TAG_NIL))
#define TRUE_VAL ((diy_value_t)(QNAN | TAG_BOOLEAN | 1))
#define FALSE_VAL ((diy_value_t)(QNAN | TAG_BOOLEAN | 0))

#define OBJECT_VAL(obj_ptr)                                                   \
  (diy_value_t) (SIGN_BIT | QNAN | TAG_OBJECT | (uint64_t)(uintptr_t)(obj_ptr))

typedef uint64_t diy_value_t;

static inline double
diy_value_to_num (diy_value_t value)
{
  union
  {
    uint64_t u;
    double d;
  } unbox;
  unbox.u = value;
  return unbox.d;
}

static inline diy_value_t
diy_num_to_value (double num)
{
  union
  {
    double d;
    uint64_t u;
  } box;
  box.d = num;
  return box.u;
}

#endif
