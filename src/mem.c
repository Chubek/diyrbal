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

static inline void
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
  size_t i;
  for (i = 0; i < frm->nlocals; i++)
    heap_push_root (h, frm->locptr[i]);
  for (; i < frm->nlocals + frm->nparams; i++)
    heap_push_root (h, frm->locptr[i]);
}

static inline void
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
  if (!obj || obj->marked)
    return;
  obj->marked = true;

  if (obj->type != OBJ_Value)
    return;

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
        gc_mark (OBJ_AsTuple (obj).elts[i]);
      continue;
    case VAL_Set:
      for (size_t i = 0; i < OBJ_AsSet (obj).cnt; i++)
        gc_mark (OBJ_AsTuple (obj).membs[i]);
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
    case VAL_Class:
      for (size_t i = 0; i < OBJ_AsClass (obj).cntfields; i++)
        gc_mark (OBJ_AsClass (obj).fields[i]);
      for (size_t i = 0; i < OBJ_AsClass (obj).cntmethods; i++)
        gc_mark (OBJ_AsClass (obj).methods[i]);
      for (size_t i = 0; i < OBJ_AsClass (obj).cntcnsts; i++)
        gc_mark (OBJ_AsClass (obj).cnsts[i]);
      continue;
    case VAL_Closure:
      for (size_t i = 0; i < OBJ_AsClosure (obj).cntcnsts; i++)
        gc_mark (OBJ_AsClosure (obj).cnsts[i]);
      gc_mark (OBJ_AsClosure (obj).boxes);
      continue;
    case OBJ_Box:
      for (Object *o = obj; o; o = o->next)
        {
          Box *b = OBJ_AsBox (o);
          if (b->closed)
            gc_mark (b->objref);
        }
      break;
    case VAL_Grammar:
      gc_mark (OBJ_AsGrammar (obj).terms);
      gc_mark (OBJ_AsGrammar (obj).nterms);
      for (size_t i = 0; i < OBJ_AsGrammar (obj).cntprods; i++)
        {
          Production *prod = &OBJ_AsGrammar (obj).prods[i];
          gc_mark (prod->rhs);
          gc_mark (prod->lhs);
        }
      continue;
    case VAL_Parser:
      gc_mark (OBJ_AsParser (obj).eps);
      gc_mark (OBJ_AsParser (obj).firsts);
      gc_mark (OBJ_AsParser (obj).follows);
      gc_mark (OBJ_AsParser (obj).predicts);
      continue;
    case VAL_DSL:
      gc_mark (OBJ_AsDSL (obj).grammar);
      gc_mark (OBJ_AsDSL (obj).absyn);
      gc_mark (OBJ_AsDSL (obj).emitter);
      for (size_t i = 0; i < OBJ_AsDSL (obj).cnttriplets; i++)
        {
          Triple *trp = &OBJ_AsDSL (obj).triplets[i];
          gc_mark (trp->precond);
          gc_mark (trp->stmt);
          gc_mark (trp->postcond);
        }
      continue;
    case VAL_Tree:
      gc_mark (OBJ_AsTree (obj).data);
      gc_mark (OBJ_AsTree (obj).children);
      gc_mark (OBJ_AsTree (obj).next);
      continue;
    case VAL_Graph:
      gc_mark (OBJ_AsTree (obj).nodes);
      gc_mark (OBJ_AsTree (obj).incdmat);
      continue;
    case VAL_Emitter:
      // TODO
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
  if (obj->marked && obj->forwarding_addr)
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
    {
      gc_update_ref (&obj);
      return;
    }

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
        gc_update_ref (&OBJ_AsTuple (obj).elts[i]);
      continue;
    case VAL_Set:
      for (size_t i = 0; i < OBJ_AsSet (obj).cnt; i++)
        gc_update_ref (&OBJ_AsTuple (obj).membs[i]);
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
    case VAL_Class:
      for (size_t i = 0; i < OBJ_AsClass (obj).cntfields; i++)
        gc_update_ref (&OBJ_AsClass (obj).fields[i]);
      for (size_t i = 0; i < OBJ_AsClass (obj).cntmethods; i++)
        gc_update_ref (&OBJ_AsClass (obj).methods[i]);
      for (size_t i = 0; i < OBJ_AsClass (obj).cntcnsts; i++)
        gc_update_ref (&OBJ_AsClass (obj).cnsts[i]);
      continue;
    case VAL_Closure:
      for (size_t i = 0; i < OBJ_AsClosure (obj).cntcnsts; i++)
        gc_update_ref (&OBJ_AsClosure (obj).cnsts[i]);
      gc_update_obj_ref (OBJ_AsClosure (obj).boxes);
      continue;
    case VAL_Boxes:
      for (Object *o = obj; o; o = o->next)
        {
          Box *b = OBJ_AsBox (o);
          if (b->closed)
            gc_update_ref (&b->objref);
        }
    case VAL_Grammar:
      gc_update_obj_ref (OBJ_AsGrammar (obj).terms);
      gc_update_obj_ref (OBJ_AsGrammar (obj).nterms);
      for (size_t i = 0; i < OBJ_AsGrammar (obj).cntprods; i++)
        {
          Production *prod = &OBJ_AsGrammar (obj).prods[i];
          gc_update_ref (prod->lhs);
          gc_update_obj_ref (prod->rhs);
        }
      continue;
    case VAL_Parser:
      gc_update_obj_ref (OBJ_AsParser (obj).eps);
      gc_update_obj_ref (OBJ_AsParser (obj).firsts);
      gc_update_obj_ref (OBJ_AsParser (obj).follows);
      gc_update_obj_ref (OBJ_AsParser (obj).predicts;);
      continue;
    case VAL_DSL:
      gc_update_obj_ref (OBJ_AsDSL (obj).grammar);
      gc_update_obj_ref (OBJ_AsDSL (obj).absyn);
      gc_update_obj_ref (OBJ_AsDSL (obj).emitter);
      for (size_t i = 0; i < OBJ_AsDSL (obj).cnttriplets; i++)
        {
          Triple *trp = &OBJ_AsDSL (obj).triplets[i];
          gc_update_ref (trp->precond);
          gc_update_ref (trp->stmt);
          gc_update_ref (trp->postcond);
          // NOTE: I have not yet decided the internal representation
          // for the triplet members. Therefore, these calls might be
          // changed to `gc_update_obj_ref` later.
        }
      continue;
    case VAL_Tree:
      gc_update_obj_ref (OBJ_AsTree (obj).data);
      gc_update_obj_ref (OBJ_AsTree (obj).children);
      gc_update_obj_ref (OBJ_AsTree (obj).next);
      continue;
    case VAL_Graph:
      gc_update_obj_ref (OBJ_AsTree (obj).nodes);
      gc_update_obj_ref (OBJ_AsTree (obj).incdmat);
      continue;
    case VAL_Emitter:
      // TODO
    default:
      gc_update_ref (&obj);
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
