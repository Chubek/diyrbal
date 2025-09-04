#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "object.h"
#include "symtbl.h"
#include "tyydecl.h"
#include "xalloc.h"

extern Heap *currheap;

static Object *
object_new (ObjType type)
{
  Object *obj = (Object *)gc_alloc_raw (currheap, sizeof (Object));

  obj->type = type;
  obj->size = sizeof (Object);
  obj->marked = false;
  obj->next = NULL;
  obj->forwarding_addr = NULL;

  xmemset (&obj->as, 0, sizeof (obj->as));
  return obj;
}

static Object *
object_new_value (ValueType type)
{
  Object *obj = object_new (OBJ_Value);

  Value *val = (Value *)gc_alloc_raw (currheap, sizeof (Value));
  val->type = type;
  xmemset (&val->as, 0, sizeof (v->as));

  obj->as.value = val;
  return obj;
}

Object *
object_new_integer (intmax_t ival)
{
  Object *obj = object_new (OBJ_Integer);
  obj->as.integer = ival;
  return obj;
}

Object *
object_new_real (double rval)
{
  Object *obj = object_new (OBJ_Real);
  obj->as.real = rval;
  return obj;
}

Object *
object_new_boolean (double bval)
{
  Object *obj = object_new (OBJ_Boolean);
  obj->as.boolean = bval;
  return obj;
}

Object *
object_new_symbol (const char *sval)
{
  Object *obj = object_new (OBJ_Symbol);
  obj->as.symbol = symtbl_intern_symbol (sval);
  return obj;
}

object *
object_new_range (int start, int end, int step)
{
  Object *obj = object_new (OBJ_Range);
  obj->as.range.start = start;
  obj->as.range.end = end;
  obj->as.range.step = step;
  return obj;
}

Object *
object_new_boolean (char32_t chval)
{
  Object *obj = object_new (OBJ_Char);
  obj->as.chr = chval;
  return obj;
}

Object *
object_new_cfunc (CFunc *cfnval)
{
  Object *obj = object_new (OBJ_CFunc);
  obj->as.cfunc = cfnval;
  return obj;
}

Object *
object_new_nil (void)
{
  Object *obj = object_new (OBJ_Nil);
  return obj;
}
