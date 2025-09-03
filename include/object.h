#ifndef VALECT_H
#define VALECT_H

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "tyydecl.h"

#define OBJ_ObjType(o) (o->type)
#define OBJ_ValueType(o) (o->as.value->type)
#define OBJ_AsList(o) (o->as.value->as.list)
#define OBJ_AsArray(o) (o->as.value->as.array)
#define OBJ_AsTuple(o) (o->as.value->as.tuple)
#define OBJ_AsString(o) (o->as.value->as.string)
#define OBJ_AsHash(o) (o->as.value->as.hash)
#define OBJ_AsTable(o) (o->as.value->as.table)
#define OBJ_AsClosure(o) (o->as.value->as.closure)
#define OBJ_AsProg(o) (o->as.value->as.prog)
#define OBJ_AsUpvalue(o) (o->as.value->as.upvalue)
#define OBJ_AsInteger(o) (o->as.integer)
#define OBJ_AsReal(o) (o->as.real)
#define OBJ_AsBoolean(o) (o->as.boolean)
#define OBJ_AsSymbol(o) (&o->as.symbol[0])

struct Value
{
  enum ValueType
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
  enum ObjType
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

Object *object_new ();

#endif
