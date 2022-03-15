#ifndef LENV_H
#define LENV_H
#include "lval.h"
#include <stddef.h>
#define LASSERT(args, cond, fmt, ...)\
    if(!(cond)){\
        lval* err = lval_err(fmt, ##__VA_ARGS__);   \
        lval_del(args);                             \
        return err;                                 \
    }

#define LASSERT_NUM(a, i)\
  if(a->count < i){\
    return lval_err("Not enought arguments, got %i need %i", a->count, i);\
  }else if(a->count > i){\
    return lval_err("Too many argument, got %i, need %i", a->count, i);\
  }

#define LASSERT_TYPE(func, args, index, expect) \
  LASSERT(args, args->cell[index]->type == expect, \
    "Function '%s' passed incorrect type for argument %i. " \
    "Got %s, Expected %s.", \
    func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define TYPEERR(i, j)\
  return lval_err("Got %s, excepted %s", ltype_name(i), ltype_name(j));

struct lenv {
  lenv* parent;
  int count;
  char** syms;
  lval** vals;
};


lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);
void lenv_def(lenv* e, lval* k, lval* v);
lenv* lenv_copy(lenv*e);

lval* builtin_var(lenv* e, lval* a, char* func);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);

lval* builtin_op(lenv* e, lval* a, char* op);
lval* builtin_comp(lenv* e, lval* a, char* op);

lval* builtin_greater(lenv* e, lval* a);
lval* builtin_less(lenv* e, lval* a);
lval* builtin_equals(lenv* e, lval* a);
lval* builtin_inequals(lenv* e, lval* a);
lval* builtin_greaterequals(lenv* e, lval* a);
lval* builtin_and(lenv* e, lval* a);
lval* builtin_or(lenv* e, lval* a);
lval* builtin_not(lenv* e, lval* a);

lval* builtin_if(lenv* e, lval* a);
lval* builtin_lessequals(lenv* e, lval* a);
lval* builtin_head(lenv* e, lval* a);
lval* builtin_head_qexpr(lenv* e, lval* a);
lval* builtin_head_string(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_tail_qexpr(lenv* e, lval* a);
lval* builtin_tail_string(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_join_qexpr(lenv* e, lval* a);
lval* builtin_join_string(lenv* e, lval* a);
lval* builtin_cons(lenv* e, lval* a);
lval* builtin_init(lenv* e, lval* a);
lval* builtin_len(lenv* e, lval* a);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_mod(lenv* e, lval* a);
lval* builtin_exit(lenv*e, lval* a);
lval* builtin_lambda(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);
lval* builtin_fun(lenv* e, lval* a);
lval* builtin_load(lenv* e, lval* a);
lval* builtin_print(lenv* e, lval* a);
lval* builtin_error(lenv* e, lval* a);
lval* builtin(lenv* e, lval* a, char* func);

void add_lisply_fun(lenv* e, char* name, char* command);

lval* lval_join(lval* x, lval* y);

void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);
#endif