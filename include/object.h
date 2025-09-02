#ifndef VALECT_H
#define VALECT_H

#include <stdbool.h>
#include <stdint.h>

#include "tyydecl.h"

struct Value
{
  enum
  {
    VAL_List,
    VAL_Array,
    VAL_Tuple,
    VAL_Hash,
    VAL_Table,
    VAL_Closure,
    VAL_Prog,
    VAL_Upvalue,
    VAL_Frame,
  } type;

  union
  {
    struct List
    {
      Value *head, *tail;
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

    struct Hash
    {
      struct Entry
      {
        Value *key;
        Value *value;
        Entry *next;
      } *entries;
      bool *occupied;
      size_t cnt, cap;
    } hash;

    struct Table
    {
      Array *array_part;
      Hash *hash_part;
    } table;

    struct Closure
    {
      Hash *env;
      Prog *code;
      Upvalue *upvals;
    } closure;

    struct Upvalue
    {
      Value *obj;
      bool open;
    } upvalue;

    struct Prog
    {
      Instr *instrs;
      size_t cnt, cap;
    } chunk;

    struct Frame
    {
      size_t sttlnk;
      size_t dynlnk;
      Tuple *cnstpool;
      Frame *prev;
    } frame;
  } as;

  bool marked;
  Value *next;
  Value *forwarding_addr;
};

struct Object
{
  enum
  {
    OBJ_Value,
    OBJ_Integer,
    OBJ_Real,
    OBJ_Boolean,
    OBJ_Nil,
  } type;

  union
  {
    Value *value;
    intmax_t integer;
    double real;
    bool boolean;
  } as;
};

#endif
