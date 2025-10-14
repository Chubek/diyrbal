#ifndef OBJECT_H
#define OBJECT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>

#include "bignum.h"
#include "concurr.h"
#include "function.h"
#include "peg.h"
#include "value.h"

typedef uint32_t diy_symbol_t;
typedef struct DIY_String diy_string_t;
typedef struct DIY_ByteBuffer diy_bytebuffer_t;
typedef struct DIY_Tuple diy_tuple_t;
typedef struct DIY_Array diy_array_t;
typedef struct DIY_Map diy_map_t;
typedef struct DIY_MapEntry diy_mapentry_t;
typedef struct DIY_Function diy_function_t;
typedef struct DIY_PEG diy_peg_t;
typedef struct DIY_Bignum diy_bignum_t;
typedef struct DIY_Fiber diy_fiber_t;
typedef struct DIY_NativeFn diy_nativefn_t;
typedef struct DIY_Closure diy_closure_t;
typedef struct DIY_Upvalue diy_upvalue_t;
typedef struct DIY_Port diy_port_t;
typedef struct DIY_Channel diy_channel_t;
typedef struct DIY_PEGProd diy_pegprod_t;

typedef enum FiberState FIBER_STATE;
typedef enum ChannelState CHANNEL_STATE;

struct DIY_String
{
  DIY_GC_HEADER;
  char32_t *buffer;
  size_t len, cap;
};

struct DIY_ByteBuffer
{
  DIY_GC_HEADER;
  uint8_t *buffer;
  size_t len, cap;
  bool is_readonly;
};

struct DIY_Tuple
{
  DIY_GC_HEADER;
  diy_value_t *contents;
  size_t size;
};

struct DIY_Array
{
  DIY_GC_HEADER;
  diy_value_t *elements;
  size_t len, cap;
  bool is_assoc;
};

struct DIY_Map
{
  DIY_GC_HEADER;
  struct DIY_MapEntry
  {
    diy_value_t key;
    diy_value_t value;
    struct DIY_MapEntry *next;
  } *entries;
  size_t size, cap;
};

struct DIY_Bignum
{
  DIY_GC_HEADER;
  diy_bitfield8_t *digits;
  size_t cnt;
  diy_numbase_t base;
  diy_symbol_t source;
};

struct DIY_Function
{
  DIY_GC_HEADER;
  diy_funsig_t *sig;
  diy_chunk_t *code;
  diy_environ_t *env;
  diy_value_t *cnstpool;
  diy_value_t *fields;
  size_t cnt_cnstpol, cap_cnstpool;
  size_t cnt_fields, cap_fields;
  bool is_constructor;
};

struct DIY_Closure
{
  DIY_GC_HEADER;
  diy_function_t *fn;
  diy_upvalue_t *upvals;
  size_t upvals_cnt;
  bool is_block;
};

struct DIY_Upvalue
{
  DIY_GC_HEADER;
  diy_value_t *box;
  bool is_open;
};

struct DIY_PEG
{
  DIY_GC_HEADER;
  struct PEGProd
  {
    diy_symbol_t name;
    diy_pegrule_t *match_rule;
    diy_closure_t *action;
    bool is_onfail;
  } *prods;
  size_t cnt_prods;
  size_t cap_prods;
  diy_port_t *stream;
};

struct DIY_Port
{
  DIY_GC_HEADER;
  struct
  {
    uint8_t read : 1;
    uint8_t write : 1;
    uint8_t append : 1;
    uint8_t exec : 1;
    uint8_t mmap : 1;
    uint8_t binary : 1;
    uint8_t strio : 1;
    uint8_t stdio : 1;
  } flags;
  diy_symbol_t source;
  union
  {
    FILE *disk_io;
    diy_string_t *str_io;
  } as;
};

struct DIY_NativeFn
{
  DIY_GC_HEADER;
  diy_value_t (*handler) (int nformals, diy_value_t *formals);
  diy_symbol_t name;
};

struct DIY_Fiber
{
  DIY_GC_HEADER;
  enum FiberState
  {
    FIBER_Init,
    FIBER_Running,
    FIBER_Stopped,
    FIBER_Destroyed,
  } state;

  int id;
  diy_thrdctx_t ctx;
  diy_closure_t *entry;
  diy_value_t *args;
  size_t cnt_args, cap_args;

  struct DIY_Fiber *next;
};

struct DIY_Channel
{
  DIY_GC_HEADER;
  bool in, out;
  enum ChannelState
  {
    CHAN_TransIn,
    CHAN_TransOut,
    CHAN_Wait,
    CHAN_Spent,
  } state;

  int from_thrd, to_thrd;
  diy_value_t *data_in;
  diy_value_t *data_out;
  size_t size_in, cap_in;
  size_t size_out, cap_out;
};

#endif
