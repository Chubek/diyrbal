#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "mem.h"
#include "object.h"
#include "util.h"
#include "xalloc.h"

Heap *
heap_new (size_t szheap, size_t nroots)
{
  Heap *h = xmalloc (sizeof (Heap));
  h->space = xcalloc (szheap, sizeof (char));
  h->szheap = szheap;
  h->start = h->allocptr = h->space;
  h->end = h->start + szheap;
  h->roots = xcalloc (nroots, sizeof (Object *));
  h->cntroots = 0;
  h->caproots = nroots;
  return h;
}

void
heap_destroy (Heap *h)
{
  xfree (h->space);
  xfree (h->roots);
  xfree (h);
}

void
heap_push_root (Heap *h, Object *const newroot)
{
  if (h->cntroots / h->caproots >= ROOTS_GROWTH_RATIO)
    {
      size_t old_cap = h->caproots;
      h->caproots *= ROOTS_GROWTH_FACTOR;
      h->roots = xrealloc (h->roots, old_cap, h->caproots, sizeof (Object *));
    }
  h->roots[h->cntroots++] = newroot;
}

void
heap_push_frame_roots (Heap *h, Stackframe *frm)
{
  for (size_t i = 0; i < frm->nlocals; i++)
    heap_push_root (h, &frm->locptr[i]);
  for (size_t j = i; j < frm->nlocals + frm->nparams; j++)
    heap_push_root (h, &frm->locptr[j]);
}

void
heap_pop_root (Heap *h)
{
  if (h->cntroots == 0)
    return;
  h->cntroots--;
}

void
heap_pop_frame_roots (Heap *h, Stackframe *frm)
{
  size_t n = frm->nlocals + frm->nparams;
  while (n--)
    heap_pop_root (h);
}
static void
gc_mark (Object *obj)
{
  if (!obj || obj->marked || obj->type != OBJ_Value)
    return;
  obj->marked = true;

  switch (OBJ_ValueType (obj))
    {
    case VAL_List:
      for (Object *o = OBJ_AsList (obj).header; o; o = o->next)
        gc_mark (o);
      continue;
    case VAL_Array:
      for (size_t i = 0; i < OBJ_AsArray (obj).cnt; i++)
        gc_mark (OBJ_AsArray (obj).data[i]);
      continue;
    case VAL_Tuple:
      for (size_t i = 0; i < OBJ_AsTuple (obj).cnt; i++)
        gc_mark (OBJ_AsTuple (obj).data[i]);
      continue;
    case VAL_Hash:
      for (size_t i = 0; i < OBJ_AsHash (obj).cap; i++)
        {
          Entry *e = OBJ_AsHash (obj).entries[i];
          while (e)
            {
              gc_mark (e->key);
              gc_mark (e->value);
              e = e->next;
            }
        }
      continue;
    case VAL_Closure:
      gc_mark (OBJ_AsClosure (obj).env);
      gc_mark (OBJ_AsClosure (obj).parent);
      gc_mark (OBJ_AsClosure (obj).upvalues);
      continue;
    case VAL_Upvalue:
      if (OBJ_AsUpvalue (obj).boxed)
        gc_mark (OBJ_AsUpvalue (obj).objref);
      continue;
    default:
      continue;
    }
}

static void
gc_mark_all (Heap *h)
{
  for (size_t i = 0; i < h->cntroots; i++)
    gc_mark (h->roots[i]);
}

static void
gc_compute_forwarding_addr (Heap *h)
{
  char *scan = h->start;
  char *nloc = h->start;

  while (scan < h->allocptr)
    {
      Object *obj = (Object)scan;
      size_t sz = obj->size;

      if (obj->marked)
        {
          obj->forwarding_addr = nloc;
          nloc += sz;
        }

      scan += sz;
    }
}

static void
gc_slide_objects (Heap *h)
{
  char *scan = h->start;
  char *nallocptr = h->start;

  while (scan < h->allocptr)
    {
      Object *obj = (Object *)scan;
      size_t sz = obj->size;

      if (obj->marked)
        {
          if (scan != obj->forwarding_addr)
            xmemcopy (obj->forwarding_addr, obj, sz);

          nallocptr = obj->forwarding_addr + sz;
        }

      scan += sz;
    }

  h->allocptr = nallocptr;
}

static inline void
gc_update_ref (Object **ref)
{
  if (!ref || *ref == NULL)
    return;

  Object *obj = *ref;
  if (obj->marked && obj->type == OBJ_Value && obj->forwarding_addr)
    {
      *ref = obj->forwarding_addr;
      obj->forwarding_addr = NULL;
      obj->marked = false;
    }
  else
    *ref = NULL;
}

static void
gc_update_obj_ref (Object *obj)
{

  if (obj->type != OBJ_Value)
    continue;

  switch (OBJ_ValueType (obj))
    {
    case VAL_List:
      for (Object *o = OBJ_AsList (obj).head; o; o = o->next)
        gc_update_ref (&o);
      continue;
    case VAL_Array:
      for (size_t i = 0; i < OBJ_AsArray (obj).cnt; i++)
        gc_update_ref (&OBJ_AsArray (obj).data[i]);
      continue;
    case VAL_Tuple:
      for (size_t i = 0; i < OBJ_AsTuple (obj).cnt; i++)
        gc_update_ref (&OBJ_AsTuple (obj).data[i]);
      continue;
    case VAL_Hash:
      for (size_t i = 0; i < OBJ_AsHash (obj).cap; i++)
        {
          Entry *e = OBJ_AsHash (obj).entries[i];
          while (e)
            {
              gc_update_ref (&e->key);
              gc_update_ref (&e->value);
              e = e->next;
            }
        }
      continue;
    case VAL_Closure:
      gc_update_obj_ref (OBJ_AsClosure (obj).env);
      gc_update_obj_ref (OBJ_AsClosure (obj).parent);
      gc_update_obj_ref (OBJ_AsClosure (obj).upvalues);
      continue;
    case VAL_Upvalue:
      gc_update_obj_ref (OBJ_AsUpvalue (obj).obj);
      continue;
    default:
      continue;
    }
}

static void
gc_update_refs (Heap *h)
{
  for (size_t i = 0; i < h->cntroots; i++)
    gc_update_ref (h->roots[i]);

  for (char *ptr = h->start; ptr < h->allocptr;)
    {
      Object *obj = (Object *)ptr;
      size_t sz = obj->size;
    }
}

void *
gc_alloc (Heap *h, size_t sz)
{
  sz = align_ptr (sz);

  if (h->allocptr + sz > h->end)
    {
      gc_collect (h);

      if (h->allocptr + sz > h->end)
        {
          size_t old_sz = h->heapsz;
          h->heapsz *= HEAP_GROWTH_FACTOR;
          h->space = xrealloc (h->space, old_sz, h->heapsz, sizeof (char));
          h->start = h->space;
          h->end = h->start + h->heapsz;
        }
    }

  void *res = h->allocptr;
  h->allocptr += sz;
  return res;
}

void
gc_collect (Heap *h)
{
  gc_mark_all (h);
  gc_compute_forwarding_addr (h);
  gc_slide_objects (h);
  gc_update_refs (h);
}
