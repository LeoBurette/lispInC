#ifndef EVAL_H
#define EVAL_H
#include "lval.h"
#include "../lib/mpc-master/mpc.h"
#include "lenv.h"

lval* eval_op(lval* x, char* operator, lval* y);
lval* single_eval_op(lval* x, char* operator);

lval* eval(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
lval* lval_read_str(mpc_ast_t* t);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read_bool(mpc_ast_t* t);
lval* lval_eval_sexpr(lval* v, lenv* e);
lval* lval_eval(lval* v, lenv* e);

lval* lval_call(lenv* e, lval* f, lval*a);
#endif