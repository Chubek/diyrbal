#ifndef OBJECT_H
#define OBJECT_H

#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>

#include "config.h"
#include "tyydecl.h"

#define OBJ_ObjType(o) (o->type)
#define OBJ_ValueType(o) (o->as.value->type)
#define OBJ_AsList(o) (o->as.value->as.list)
#define OBJ_AsArray(o) (o->as.value->as.array)
#define OBJ_AsTuple(o) (o->as.value->as.tuple)
#define OBJ_AsSet(o) (o->as.value->as.set)
#define OBJ_AsString(o) (o->as.value->as.string)
#define OBJ_AsHash(o) (o->as.value->as.hash)
#define OBJ_AsClass(o) (o->as.value->as.cls)
#define OBJ_AsPort(o) (o->as.value->as.port)
#define OBJ_AsClosure(o) (o->as.value->as.closure)
#define OBJ_AsCoroutine(o) (o->as.value->as.coroutine)
#define OBJ_AsProg(o) (o->as.value->as.prog)
#define OBJ_AsBox(o) (o->as.value->as.box)
#define OBJ_AsPattern(o) (o->as.value->as.pattern)
#define OBJ_AsMatchResult(o) (o->as.value->as.matchresult)
#define OBJ_AsGrammar(o) (o->as.value->as.grammar)
#define OBJ_AsParser(o) (o->as.value->as.parser)
#define OBJ_AsDSL(o) (o->as.value->as.dsl)
#define OBJ_AsGraph(o) (o->as.value->as.graph)
#define OBJ_AsTree(o) (o->as.value->as.tree)
#define OBJ_AsEmitter(o) (o->as.value->as.emitter)
#define OBJ_AsInteger(o) (o->as.integer)
#define OBJ_AsReal(o) (o->as.real)
#define OBJ_AsBoolean(o) (o->as.boolean)
#define OBJ_AsSymbol(o) (o->as.symbol)
#define OBJ_AsRange(o) (o->as.range)
#define OBJ_AsChar(o) (o->as.chr)
#define OBJ_AsCfunc(o) (o->as.cfunc)

struct Value
{
  enum ValueType
  {
    VAL_List,
    VAL_Array,
    VAL_Tuple,
    VAL_Set,
    VAL_String,
    VAL_Hash,
    VAL_Class,
    VAL_Port,
    VAL_Closure,
    VAL_Coroutine,
    VAL_Prog,
    VAL_Box,
    VAL_Pattern,
    VAL_MatchResult,
    VAL_Grammar,
    VAL_Parser,
    VAL_DSL,
    VAL_Tree,
    VAL_Graph,
    VAL_Emitter,
  } type;

  union
  {
    struct List
    {
      Object *head, *tail;
    } list;

    struct Array
    {
      Object **data;
      size_t cnt, cap;
      Object *dims;
    } array;

    struct Tuple
    {
      Object **elts;
      size_t cnt;
    } tuple;

    struct Set
    {
      Object **membs;
      size_t cnt, cap;
      bool frozen;
    } set;

    struct String
    {
      const char32_t *buff;
      size_t len, cap;
      bool utf8;
    } string;

    struct Hash
    {
      struct Entry
      {
        const Object *key;
        Object *value;
        Entry *next;
      } *entries;
      bool *occupied;
      size_t cnt, cap;
    } hash;

    struct Class
    {
      Object *super;
      Object *fields;
      Object *methods;
      Object *cnsts;
      Symtbl *env;
      ASTNode *src;
    } cls;

    struct Port
    {
      Object *path;
      FILE *stream;
      bool r, w, a, b;
      int stdfd;
      size_t offs;
    } port;

    struct Closure
    {
      Object *cnsts;
      Object *prog;

      Object *parent;
      size_t nformals;
      bool isvarargs;
      Symtbl *env;
      ASTNode *srcnode;
      Object *boxchain;
    } closure;

    struct Coroutine
    {
      enum CoroState
      {
        CORO_Ready,
        CORO_Running,
        CORO_Suspended,
        CORO_Done,
      } state;

      Object *fn;
      Context *ctx;
      int id;
    } coroutine;

    struct Prog
    {
      struct Instr
      {
        Opcode opcode;
        Object *operand;
      } *instrs;
      size_t cntinstrs, capinstrs;
      int localpc;
      const Object *src;
    } prog;

    struct Box
    {
      Object *objref;
      int nestlvl;
      bool closed;
      Object *next;
    } box;

    struct Pattern
    {
      struct PatternElt
      {

        enum PatternType
        {
          PATT_Literal,
          PATT_Class,
          PATT_AnchorStart,
          PATT_AnchorEnd,
          PATT_CaptureStart,
          PATT_CaptureEnd,
          PATT_AnyChar,
          PATT_Frontier,
        } type;

        enum PatternQuantifier
        {
          PQUANT_Once,
          PQUANT_ZeroOrMore,
          PQUANT_OnceOrMore,
          PQUANT_ZeroOrOne,
        } quantifier;

        bool nongreedy;
        bool inverted;
        uint8_t literal;
        uint8_t clsname;
        uint8_t bitmap[UCHAR_MAX + 1];
      } *elts;
      size_t cntelts, capelts;
      Object *srcstr;
    } pattern;

    struct MatchResult
    {
      Object *captrngs;
      Object *matchrngs;
      bool success;
      Object *matchstr;
    } matchresult;

    struct Grammar
    {
      Object *terms;
      Object *nterms;
      struct Production
      {
        Object *lhs;
        Object *rhs;
        Object *semaction;
      } *prods;
      size_t cntprods, capprods;
    } grammar;

    struct Parser
    {
      struct ParseTableEntry
      {
        enum ParseAction
        {
          PACTION_Predict,
          PACTION_Error,
        } type;
        size_t predidx;
      } *entries;
      size_t cntentires, capentries;
      Object *eps;
      Object *firsts;
      Object *follows;
      Object *predicts;
      Object *srcgrm;
    } parser;

    struct DSL
    {
      Object *grammar;
      Object *absyn;
      Object *states;
      Object *emitter;
      struct Triple
      {
        Object *precond;
        Object *stmt;
        Object *postcond;
      } *triples;
      size_t cnttriples, captriples;
      Symtbl *env;
      Object *src;
    } dsl;

    struct Tree
    {
      Object *data;
      Object *children;
    } tree;

    struct Graph
    {
      Object *nodes;
      Object *incdmat;
    } graph;

    struct Emitter
    {
      // TODO
    } emitter;
  } as;
};

struct Object
{
  enum ObjType
  {
    OBJ_Value,
    OBJ_Integer,
    OBJ_Real,
    OBJ_Boolean,
    OBJ_Char,
    OBJ_Symbol,
    OBJ_Range,
    OBJ_Nil,
    OBJ_CFunc,
  } type;

  union
  {
    Value *value;
    intmax_t integer;
    double real;
    bool boolean;
    char32_t chr;
    uint32_t symbol;
    struct Range
    {
      uint64_t start : 31;
      uint64_t end : 31;
      uint64_t step : 2;
    } range;
    CFunc *cfunc;
  } as;

  size_t size;
  bool marked;
  Object *next;
  Object *forwarding_addr;
  Hashfn *hashfunc;
};

/* set #1 */
static Object *object_new (ObjType type);
static Object *object_new_value (ValueType type);

/* set #2 */
Object *object_new_integer (intmax_t ival);
Object *object_new_real (double rval);
Object *object_new_boolean (bool bval);
Object *object_new_symbol (const char *sval);
Object *object_new_range (int start, int end, int step);
Object *object_new_char (char32_t chrval);
Object *object_new_cfunc (CFunc *cfnval);
Object *object_new_nil (void);

/* set #3 */
Object *object_new_list (void);
void object_append_list (Object *lst, Object *newobj);
void object_delitem_list (Object *lst, Object *idx);
Object *object_reverse_list (Object *lst);
Object *object_idxof_list (Object *lst, Object *item);
Object *object_shift_list (Object *lst);
Object *object_pop_list (Object *lst);
Object *object_getat_list (Object *lst, Object *idx);
void object_setat_list (Object *lst, Object *newitem, Object *idx);
Object *object_getrange_list (Object *lst, Object *rng);
void object_setrange_list (Object *lst, Object *slice, Object *rng);

/* set #4 */
Object *object_new_array (size_t cap, Object *dims);
void object_insert_array (Object *arr, Object *item, Object *dim);
Object *object_idxof_array (Object *arr, Object *item);
Object *object_getat_array (Object *arr, Object *idx, Object *dim);
void object_setat_array (Object *arr, Object *newitem, Object *idx,
                         Object *dim);
Object *object_getrange_array (Object *arr, Object *range, Object *dim);
void object_setrange_array (Object *arr, Object *slice, Object *rng,
                            Object *dim);

/* set #5 */
Object *object_new_tuple (size_t cnt, ...);
Object *object_getat_tuple (Object *tup, Object *idx);
Object *object_getrange_tuple (Object *tup, Object *range);

/* set #6 */
Object *object_new_set (size_t cap);
void object_freeze_set (Object *set);
Object *object_add_set (Object *set, Object *item);
Object *object_remove_set (Object *set, Object *item);
Object *object_removeat_set (Object *set, Object *idx);
Object *object_union_set (Object *set1, Object *set2);
Object *object_intersect_set (Object *set1, Object *set2);
Object *object_differ_set (Object *set1, Object *set2);
Object *object_symdiffer_set (Object *set1, Object *set2);
Object *object_getat_set (Object *set, Object *idx);
void object_setat_set (Object *set, Object *idx, Object *item);
Object *object_getrng_set (Object *set, Object *rng);
void object_setrng_set (Object *set, Object *slice, Object *rng);
Object *object_issubset_set (Object *set, Object *sub);
Object *object_issuperset_set (Object *set, Object *sup);

/* set #7 */
Object *object_new_string (const char32_t *from, size_t nfrom);
Object *object_appendchar_string (Object *str, Object *chr);
Object *object_catstr_string (Object *str1, Object *str2);
Object *object_getrange_string (Object *str, Object *range);
void object_setrange_string (Object *str, Object *newrng, Object *range);
Object *object_getlen_string (Object *str);
Object *object_matchpatt_string (Object *str, Object *patt);
Object *object_findpatt_string (Object *str, Object *patt);

/* set #8 */
Object *object_new_hash (size_t cap);
void object_setitem_hash (Object *hash, const Object *key, Object *value);
bool object_getitem_hash (Object *hash, const Object *key, Object **valdst);
bool object_delitem_hash (Object *hash, const Object *key);
static bool object_shoudgrow_hash (Object *hash);
static void object_grow_hash (Object *hash);

/* set #9 */
Object *object_new_closure (Object *parent, size_t nformals, bool isvarargs);
void object_setenv_closure (Object *clsr, const Object *symbol, Object *value);
Object *object_getenv_closure (Object *clsr, const Object *symbol);
void object_addbox_closure (Closure *clsr, Object *box);
void object_addconst_closure (Closure *clsr, Object *cnst);
void object_addinstr_closure (Closure *clsr, Instr *instr);
void object_execute_closure (Closure *clsr, Interp *interp);

/* set #10 */
Object *object_new_box (Object *obref, int nestlvl);
void object_closeiflvl_box (Object *box, int lvlcond);

/* set #11 */
Object *object_new_class (Object *super);
void object_fieldfn_class (Object *cls, const Object *name, Object *value);
void object_methdfn_class (Object *cls, const Object *name, Object *method);
void object_cnstadd_class (Object *cls, Object *cnst);
Object *object_getsym_class (Object *cls, const Object *name);
Object *object_getcsnt_class (Object *cls, size_t offs);

/* set #12 */
Object *object_new_port (Object *path, bool r, bool w, bool a, bool b,
                         int stdfd);
void object_openstrm_port (Object *port);
void object_closestrm_port (Object *port);
Object *object_readline_port (Object *port);
Object *object_readchar_port (Object *port);
Object *object_readall_port (Object *port);
Object **object_readnlines_port (Object *port, Object *n);
Object **object_readnchars_port (Object *port, Object *n);
void object_writeline_port (Object *port, Object *ln);
void object_writechar_port (Object *port, Object *chr);
void object_writeall_port (Object *port, Object *txt);
void object_writenlines_port (Object *port, Object **lns, size_t n);
void object_writenchars_port (Object *port, Object **chrs, size_t n);
void object_setseek_port (Object *port, Object *nseek, int rel);
Object *object_getseek_port (Object *port);

/* set #13 */
Object *object_new_pattern (Object *srcstr);
void object_addelt_pattern (Object *patt, PatternType type,
                            PatternQuantifier quant, bool nongreedy,
                            bool inverted, uint8_t literal, uint8_t clsname,
                            uint8_t *bitmap);
Object *object_runmatch_pattern (Object *patt, Object *matchstr);

/* set #14 */
Object *object_new_matchresult (Object *matchstr);
void object_addcaptrng_matchresult (Object *mres, Object *crng);
void object_addmatchrng_matchresult (Object *mres, Object *mrng);
void object_setsuccess_matchresult (Object *mres, bool succres);

/* set #15 */
Object *object_new_grammar (void);
void object_addterm_grammar (Object *grm, Object *term);
void object_addnterm_grammar (Object *grm, Object *nterm);
void object_addprod_grammar (Object *grm, Object *lhs, Object *rhs,
                             Object *semaction);

/* set #16 */
Object *object_new_parser (Object *srcgrm);
void object_addentry_parser (Object *prs, ParseAction action, size_t predidx);
void object_addeps_parser (Object *prs, Object *eps);
void object_addfirst_parser (Object *prs, Object *first);
void object_addfollow_parser (Object *prs, Object *follow);
void object_addpredict_parser (Object *prs, Object *predict);

/* set #17 */
Object *object_new_prog (const Object *src);
void object_pushinstr_prog (Object *prog, Opcode opcode, Object *operand);
void object_popinstr_prog (Object *prog, Instr *instrdst);

/* set #18 */
Object *object_new_dsl (Object *grm, Object *absyn, Object *stt, Object *emt);
void object_pushtriple_dsl (Object *dsl, Object *precond, Object *stmt,
                            Object *postcond);
void object_gettriple_dsl (Object *dsl, Object **tripledst, size_t idx);

/* set #19 */
Object *object_new_tree (void);
void object_addchild_tree (Object *tree, Object *data);

/* set #20 */
Object *object_new_graph (void);
void object_addnode_graph (Object *grph, Object *node);
void object_connode_graph (Object *grph, Object *from, Object *to);

/* set #21 */
Object *object_new_coroutine (Object *fn, Context *ctx, int id);


#endif
