#ifndef MEM_H
#define MEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tyydecl.h"

struct Heap
{
  char *space;
  char *start, *end;
  char *allocptr;
  size_t szheap;
  Object **const roots;
  size_t cntroots, caproots;
};

Heap *heap_new (size_t szheap, size_t nroots);
void heap_destroy (Heap *h);
static inline void heap_push_root (Heap *h, Object *const newroot);
void heap_push_frame_roots (Heap *h, Stackframe *frm);
static inline void heap_pop_root (Heap *h);
void heap_pop_frame_roots (Heap *h, Stackframe *frm);
static void gc_mark (Object *obj);
static void gc_mark_all (Heap *h);
static void gc_compute_forwarding_addr (Heap *h);
static void gc_slide_objects (Heap *h);
static inline void gc_update_ref (Object **ref);
static void gc_update_object_ref (Object *obj);
static void gc_update_refs (Heap *h);
void *gc_alloc (Heap *h, size_t sz);
void gc_collect (Heap *h);

#endif
