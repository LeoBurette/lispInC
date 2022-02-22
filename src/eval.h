#ifndef EVAL_H
#define EVAL_H
#include "lval.h"
#include "../lib/mpc-master/mpc.h"
#define LASSERT(args, cond, err)\
    if(!(cond)){\
        lval_del(args);\
         return lval_err(err);\
    }

lval* eval_op(lval* x, char* operator, lval* y);
lval* single_eval_op(lval* x, char* operator);

lval* eval(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_eval_sexpr(lval* v);
lval* lval_eval(lval* v);
lval* builtin_op(lval* a, char* op);

lval* builtin_head(lval* a);
lval* builtin_tail(lval* a);
lval* builtin_list(lval* a);
lval* builtin_eval(lval* a);
lval* builtin_join(lval* a);
lval* builtin_cons(lval* a);
lval* builtin_init(lval* a);
lval* builtin_len(lval* a);
lval* lval_join(lval* x, lval* y);
lval* builtin(lval* a, char* func);

#endif