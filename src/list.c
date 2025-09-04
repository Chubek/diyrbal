#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "tyydecl.h"
#include "xmalloc.h"

Object *
object_new_list (void)
{
  Object *lst = object_value_new (VAL_List);
  return lst;
}

void
object_append_list (Object *lst, Object *newobj)
{
  if (OBJ_AsList (lst).head == OBJ_AsList (lst).tail == NULL)
    {
      OBJ_AsList (lst).head = OBJ_AsList (lst).tail = newobj;
      return;
    }
  OBJ_AsList (lst).tail->next = newobj;
  OBJ_AsList (lst).tail = newobj;
}

void
object_delitem_list (Object *lst, Object *idx)
{
  Object *item = NULL, *previtem = NULL;
  size_t idx = 0;
  for (item = OBJ_AsList (lst).head; item != OBJ_AsList (lst).tail;
       item = item->next)
    {
      if (idx++ == OBJ_AsInteger (idx))
        break;
      previtem = item;
    }
  if (item->next == NULL)
    {
      previtem->next = NULL;
      return;
    }
  else if (previtem == NULL && item->next)
    {
      item->next = item->next->next;
    }
  else if (previtem && item->next)
    {
      item->next = item->next->next;
      previtem->next = item->next;
    }
}

void
object_reverse_list (Object *lst)
{
  Object *previtem = NULL, *nextitem = NULL;
  Object *curritem = OBJ_AsList (lst).head;

  OBJ_AsList (lst).tail = OBJ_AsList (lst).head;

  while (curritem != NULL)
    {
      nextitem = curritem->next;
      curritem->next = previtem;
      previtem = curritem;
      curritem = nextitem;
    }

  OBJ_AsList (lst).head = previtem;
}

Object *object_idxof_list (Object *lst, Object *item);
Object *object_shift_list (Object *lst);
Object *object_pop_list (Object *lst);
Object *object_getat_list (Object *lst, Object *idx);
void object_setat_list (Object *lst, Object *newitem, Object *idx);
Object *object_getrange_list (Object *lst, Object *rng);
void object_setrange_list (Object *lst, Object *slice, Object *rng);
