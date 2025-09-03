#ifndef SYMTBL_H
#define SYMTBL_H

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "tyydecl.h"

struct Name
{
  const char *nm;
  int len;
};

struct Binding
{
  const Name *name;
  bool alias, statik;
  union
  {
    size_t accoffs;
    Name *alias;
  } as;

  Binding *next;
};

struct Symtbl
{
  Symtbl *parent;
  enum ScopeType
  {
    Global,
    Function,
    Block,
  } scope;
  Binding *bindings;
  size_t cntbindings;
  size_t capbindings;
};

#endif
