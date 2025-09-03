#ifndef STACKS_H
#define STACKS_H

#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "tyydecl.h"

struct Stackframe
{
  Object **locptr;
  uint8_t nlocals;
  uint8_t nparams;
};

struct Ctrlstack
{
  ssize_t sp, fp, ip;
  Instr *currprog;
  Stackframe *frms;
  size_t cntfrms, capfrms;
};

struct Valstack
{
  Object *slots;
  size_t cntslots, capslots;
};

#endif
