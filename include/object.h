#ifndef VALECT_H
#define VALECT_H

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "tyydecl.h"

struct Value
{
  enum
  {
    VAL_List,
    VAL_Array,
    VAL_Tuple,
    VAL_String,
    VAL_Hash,
    VAL_Table,
    VAL_Closure,
    VAL_Prog,
    VAL_Upvalue,
  } type;

  union
  {
    struct List
    {
      Object *head, *tail;
    } list;

    struct Array
    {
      Value **data;
      size_t cnt, cap;
    } array;

    struct Tuple
    {
      Value **data;
      size_t cnt;
    } tuple;

    struct String
    {
      const uint8_t *buff;
      size_t len, cap;
      bool utf8;
    } string;

    struct Hash
    {
      struct Entry
      {
        Object *key;
        Object *value;
        Entry *next;
      } *entries;
      bool *occupied;
      size_t cnt, cap;
    } hash;

    struct Table
    {
      Object *array_part;
      Object *hash_part;
    } table;

    struct Closure
    {
      Object *env;
      Object *code;
      Object *upvals;
    } closure;

    struct Upvalue
    {
      Object *obj;
      bool boxed;
    } upvalue;

    struct Prog
    {
      Instr *instrs;
      size_t cnt, cap;
      size_t insptr;
    } prog;
  } as;
};

struct Object
{
  Metatbl *mtbl;
  enum
  {
    OBJ_Value,
    OBJ_Integer,
    OBJ_Real,
    OBJ_Boolean,
    OBJ_Symbol,
    OBJ_Nil,
  } type;

  union
  {
    Value *value;
    intmax_t integer;
    double real;
    bool boolean;
    const char symbol[SYM_SIZE + 1];
  } as;

  size_t size;
  bool marked;
  Object *next;
  Object *forwarding_addr;
};

#endif
