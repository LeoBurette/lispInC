#include "lenv.h"
#include <stdlib.h>
#include <string.h>  
#include "util.h"
#include <math.h>
#include "eval.h"
#include "parsing.h"

lenv* lenv_new(void){
    lenv* v = malloc(sizeof(lenv));
    v->count = 0;
    v->syms = NULL;
    v->vals = NULL;
    v->parent = NULL;
    return v;
}

void lenv_del(lenv* e){
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

lval* lenv_get(lenv* e, lval* k){
    for(int i = 0; i < e->count; i++){
        if(strcmp(k->sym, e->syms[i])==0){
            return lval_copy(e->vals[i]);
        }
    }

    if(e->parent){
        return lenv_get(e->parent, k);
    }

    return lval_err("unbound symbol '%s'!", k->sym);
}

lenv* lenv_copy(lenv*e){
    lenv* next = malloc(sizeof(lenv));
    next->count = e->count;
    next->parent = e->parent;
    next->syms = malloc(sizeof(char*) * next->count);
    next->vals = malloc(sizeof(lval) * next->count);
    for(int i = 0; i < next->count; i++){
        next->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(next->syms[i], e->syms[i]);
        next->vals[i] = lval_copy(e->vals[i]);
    }
    return next;
}

void lenv_def(lenv* e, lval* k, lval* v){
    while(e->parent){
        e = e->parent;
    }

    lenv_put(e, k, v);
}

void lenv_put(lenv* e, lval* k, lval* v) {
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }

    e->count++;
    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);

    e->vals[e->count-1] = lval_copy(v);
    e->syms[e->count-1] = malloc(strlen(k->sym)+1);
    strcpy(e->syms[e->count-1], k->sym);
}

lval* builtin(lenv* e, lval* a, char* func) {
    if (strcmp("list", func) == 0) { return builtin_list(e, a); }
    if (strcmp("head", func) == 0) { return builtin_head(e, a); }
    if (strcmp("tail", func) == 0) { return builtin_tail(e, a); }
    if (strcmp("join", func) == 0) { return builtin_join(e, a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(e, a); }
    if (strcmp("cons", func) == 0) { return builtin_cons(e, a); }
    if (strcmp("len", func) == 0) { return builtin_len(e, a); }
    if (strcmp("init", func) == 0) { return builtin_init(e, a); }
    if (strstr("+-/*", func)) { return builtin_op(e, a, func); }
    lval_del(a);
    return lval_err("Unknown Function!");
}

lval* builtin_op(lenv* e, lval* a, char* op){
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != LVAL_NUM) {
            lval_del(a);
            return lval_err("Cannot operate on non-number!");
        }
    }

    lval* x = lval_pop(a, 0);

    if((strcmp(op, "-")==0) && a->count==0){
        x->num = -x->num;
    }

    while(a->count!=0){
        lval* y = lval_pop(a, 0);
        if(strcmp(op, "+")==0 || strcmp(op, "add")==0){
            x->num = x->num + y->num;
        }else if(strcmp(op, "-")==0 || strcmp(op, "sub")==0){
            x->num = x->num - y->num;
        }else if(strcmp(op, "*")==0 || strcmp(op, "mul")==0){
             x->num = x->num * y->num;
        }else if(strcmp(op, "/")==0 || strcmp(op, "div")==0){
            if(y->num==0){
                x = lval_err("Error: Division by zero!\n");
                break;
            }else{
                x->num = x->num / y->num;
            }
        }else if(strcmp(op, "\%")==0){
            if(!isInteger(x->num) || !isInteger(y->num)){
                x = lval_err("Error: Invalid Number!\n");
                break;
            }
            int a = x->num;
            int b = y->num;
            x->num = a % b;
        }else if(strcmp(op, "^")==0){
            x->num = pow(x->num, y->num);
        }else if(strcmp(op, "min")==0){
            x->num = (x->num < y->num)?x->num:y->num;
        }else if(strcmp(op, "max")==0){
             x->num = (x->num > y->num)?x->num:y->num;
        }else if(strcmp(op, ">")==0){
            return lval_num(x->num > y->num);
        }else if(strcmp(op, "<")==0){
            return lval_num(x->num < y->num);
        }else if(strcmp(op, ">=")==0){
            return lval_num(x->num >= y->num);
        }else if(strcmp(op, "<=")==0){
            return lval_num(x->num <= y->num);
        }else{
            x = lval_err("Error: Invalid Number!\n");
        }

        lval_del(y);
    }

    lval_del(a);
    return x;
}

lval* builtin_head_qexpr(lenv* e, lval* a){
    LASSERT(a, a->count==1, "Function 'head' passed too many arguments! Got %i, excepted %i", a->count, 1);

    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,"Function 'head' passed incorrect type! Got %s, excepted %s", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
    
    LASSERT(a, a->cell[0]->count != 0,"Function 'head' passed {}!");

    lval* v = lval_take(a, 0);

    while (v->count > 1){
        lval_del(lval_pop(v, 1));
    }
    return v;
}

lval* builtin_head_string(lenv* e, lval* a){
    LASSERT(a, a->count==1, "Function 'head' passed too many arguments! Got %i, excepted %i", a->count, 1);

    LASSERT(a, a->cell[0]->type == LVAL_STR,"Function 'head' passed incorrect type! Got %s, excepted %s", ltype_name(a->cell[0]->type), ltype_name(LVAL_STR));

    LASSERT(a, strlen(a->cell[0]->str)!=0, "Function 'head' passed \"\"!");

    lval* v = lval_str("a");
    v->str[0] = a->cell[0]->str[0];

    lval_del(a);
    return v;
}

lval* builtin_head(lenv* e, lval* a){
    if(a->cell[0]->type == LVAL_QEXPR){
        return builtin_head_qexpr(e, a);
    }
    
    if(a->cell[0]->type == LVAL_STR){
        return builtin_head_string(e, a);
    }

    return lval_err("head need %s or %s, get %s.", ltype_name(LVAL_QEXPR), ltype_name(LVAL_STR), ltype_name(a->cell[0]->type));
}


lval* builtin_tail_qexpr(lenv* e, lval* a){
    LASSERT(a, a->count == 1,"Function 'tail' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'tail' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0,"Function 'tail' passed {}!");

    lval* v = lval_take(a, 0);

    lval_del(lval_pop(v, 0));
    return v;

}

lval* builtin_tail_string(lenv* e, lval* a){
    LASSERT(a, a->count == 1,"Function 'tail' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_STR, "Function 'tail' passed incorrect type!");
    LASSERT(a, strlen(a->cell[0]->str)!=0,"Function 'tail' passed \"\"!");

    lval* v = lval_str(&a->cell[0]->str[1]);
    lval_del(a);
    return v;
}

lval* builtin_tail(lenv* e, lval* a){
    if(a->cell[0]->type == LVAL_QEXPR){
        return builtin_tail_qexpr(e, a);
    }
    
    if(a->cell[0]->type == LVAL_STR){
        return builtin_tail_string(e, a);
    }

    return lval_err("tail need %s or %s, get %s.", ltype_name(LVAL_QEXPR), ltype_name(LVAL_STR), ltype_name(a->cell[0]->type));
}

lval* builtin_list(lenv* e, lval* a){
    a->type = LVAL_QEXPR;
    return a;
}

lval* builtin_eval(lenv* e, lval* a){
    LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");

    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,"Function 'eval' passed incorrect type!");

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x, e);
}

lval* builtin_join_string(lenv* e, lval* a){
    for (int i = 0; i < a->count; i++) {
        LASSERT(a, (a->cell[i]->type == LVAL_STR),"Function 'join' passed incorrect type.");
    }

    lval* x = lval_pop(a, 0);

    while(a->count){
        strcat(x->str, lval_pop(a, 0)->str);
    }

    lval_del(a);
    return x;
}

lval* builtin_join_qexpr(lenv* e, lval* a){
    for (int i = 0; i < a->count; i++) {
        LASSERT(a, (a->cell[i]->type == LVAL_QEXPR),"Function 'join' passed incorrect type.");
    }

    lval* x = lval_pop(a, 0);

    while(a->count){
        x = lval_join(x, lval_pop(a, 0));
    }

    lval_del(a);
    return x;
}

lval* builtin_join(lenv* e, lval* a){
    if(a->cell[0]->type == LVAL_QEXPR){
        return builtin_join_qexpr(e, a);
    }
    
    if(a->cell[0]->type == LVAL_STR){
        return builtin_join_string(e, a);
    }

    return lval_err("Join need %s or %s, get %s.", ltype_name(LVAL_QEXPR), ltype_name(LVAL_STR), ltype_name(a->cell[0]->type));
}


lval* builtin_cons(lenv* e, lval* a){
    LASSERT(a, a->cell[0]->type == LVAL_NUM, "Function 'cons' need a value as first argument.")

    LASSERT(a, a->cell[1]->type == LVAL_QEXPR, "Function 'cons' need a qexp as second argument.")

    lval* num = lval_pop(a, 0);
    lval* x = lval_qexpr();
    x = lval_add(x, num);

    lval* y = lval_pop(a, 0);
    
    x = lval_join(x, y);
    lval_del(a);
    return x;
}

lval* builtin_init(lenv* e, lval* a){
    LASSERT(a, a->count==1, "Function 'init' passed too many arguments!");

    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,"Function 'init' passed incorrect type!");
    
    LASSERT(a, a->cell[0]->count != 0,"Function 'init' passed {}!");

    lval* x = lval_take(a, 0);
    lval_pop(x, x->count-1);
    return x;
};
lval* builtin_len(lenv* e, lval* a){
    LASSERT(a, a->count = 1, "Need 1 arguments");

    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'len' need a qexpr");

    lval* x = lval_num(lval_pop(a, 0)->count);
    return x;
};

lval* builtin_def(lenv* e, lval* a) {
  return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a){
  return builtin_var(e, a, "=");
}

lval* builtin_var(lenv* e, lval* a, char* func) {
  LASSERT_TYPE(func, a, 0, LVAL_QEXPR);

  lval* syms = a->cell[0];
  for (int i = 0; i < syms->count; i++) {
    LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
      "Function '%s' cannot define non-symbol. "
      "Got %s, Expected %s.", func,
      ltype_name(syms->cell[i]->type),
      ltype_name(LVAL_SYM));
  }

  LASSERT(a, (syms->count == a->count-1),
    "Function '%s' passed too many arguments for symbols. "
    "Got %i, Expected %i.", func, syms->count, a->count-1);

  for (int i = 0; i < syms->count; i++) {
    /* If 'def' define in globally. If 'put' define in locally */
    if (strcmp(func, "def") == 0) {
      lenv_def(e, syms->cell[i], a->cell[i+1]);
    }

    if (strcmp(func, "=")   == 0) {
      lenv_put(e, syms->cell[i], a->cell[i+1]);
    }
  }

  lval_del(a);
  return lval_sexpr();
}

lval* builtin_add(lenv* e, lval* a) {
  return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
  return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
  return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
  return builtin_op(e, a, "/");
}

lval* builtin_mod(lenv* e, lval* a){
    return builtin_op(e, a, "%");
}

lval* builtin_greater(lenv* e, lval* a){
    return builtin_op(e, a, ">");
}
lval* builtin_less(lenv* e, lval* a){
    return builtin_op(e, a, "<");
}

lval* builtin_equals(lenv* e, lval* a){
    return builtin_comp(e, a, "==");
}

lval* builtin_comp(lenv* e, lval* a, char* op){
    LASSERT_NUM(a, 2);
    if(strstr(op, "==")){
        return lval_num(lval_equal(a->cell[0], a->cell[1]));
    }else if(strstr(op, "!=")){
        return lval_num(!lval_equal(a->cell[0], a->cell[1]));
    }else if(strstr(op, "||")){
        return lval_num(lval_eval(a->cell[0], e)->num || lval_eval(a->cell[1], e)->num);
    }else if(strstr(op, "&&")){
        return lval_num(lval_eval(a->cell[0], e)->num && lval_eval(a->cell[1], e)->num);
    }
    return lval_err("Incorect symbol.");
}
lval* builtin_and(lenv* e, lval* a){
    return builtin_comp(e, a, "&&");
}
lval* builtin_or(lenv* e, lval* a){
    return builtin_comp(e, a, "||");
}
lval* builtin_not(lenv* e, lval* a){
    return lval_num(!a->cell[0]->num);
}

lval* builtin_inequals(lenv* e, lval* a){
    return builtin_comp(e, a, "!=");
}

lval* builtin_lessequals(lenv* e, lval* a){
    return builtin_op(e, a, "<=");
}

lval* builtin_greaterequals(lenv* e, lval* a){
    return builtin_op(e, a, ">=");
}

lval* builtin_if(lenv* e, lval* a){
    LASSERT_NUM(a, 3);
    LASSERT_TYPE("if", a, 0, LVAL_NUM);
    LASSERT_TYPE("if", a, 1, LVAL_QEXPR);
    LASSERT_TYPE("if", a, 2, LVAL_QEXPR);

    lval* x;
    if(a->cell[0]->num){
        a->cell[1]->type = LVAL_SEXPR;
        x = lval_eval(lval_pop(a, 1), e);
    }else{
        a->cell[2]->type = LVAL_SEXPR;
        x = lval_eval(lval_pop(a, 2), e);
    }

    lval_del(a);
    return x;
}

lval* builtin_exit(lenv*e, lval* a){
    exit(EXIT_SUCCESS);
}

lval* builtin_lambda(lenv* e, lval* a){
    LASSERT_NUM(a, 2);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "First argument isn't a %s, it's a %s", ltype_name(LVAL_QEXPR), ltype_name(a->cell[0]->type));
    LASSERT(a, a->cell[1]->type == LVAL_QEXPR, "Second argument isn't a %s, it's a %s", ltype_name(LVAL_QEXPR), ltype_name(a->cell[1]->type));

    for (int i = 0; i < a->cell[0]->count; i++) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM), "Cannot define non-symbol. Got %s, Expected %s.", ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
    }

    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}

lval* builtin_fun(lenv* e, lval* a){
    LASSERT_NUM(a, 2);
    LASSERT_TYPE("fun", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("fun", a, 1, LVAL_QEXPR);

    lval* tail = lval_copy(a->cell[0]);
    lval* head = lval_pop(tail, 0);

    lval* next = lval_lambda(tail , lval_copy(a->cell[1]));

    lenv_def(e, head, next);

    lval_del(a);

    return next;
}

lval* builtin_load(lenv* e, lval* a) {
  LASSERT_NUM(a, 1);
  LASSERT_TYPE("load", a, 0, LVAL_STR);
  
  /* Parse File given by string name */
  mpc_result_t r;
  if (mpc_parse_contents(a->cell[0]->str, Lisply, &r)) {
    
    /* Read contents */
    lval* expr = lval_read(r.output);
    mpc_ast_delete(r.output);

    /* Evaluate each Expression */
    while (expr->count) {
      lval* x = lval_eval(lval_pop(expr, 0), e);
      /* If Evaluation leads to error print it */
      if (x->type == LVAL_ERR) { lval_println(x); }
      lval_del(x);
    }
    
    /* Delete expressions and arguments */
    lval_del(expr);    
    lval_del(a);
    
    /* Return empty list */
    return lval_sexpr();
    
  } else {
    /* Get Parse Error as String */
    char* err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);
    
    /* Create new error message using it */
    lval* err = lval_err("Could not load Library %s", err_msg);
    free(err_msg);
    lval_del(a);
    
    /* Cleanup and return error */
    return err;
  }
}

lval* builtin_print(lenv* e, lval* a) {

  /* Print each argument followed by a space */
  for (int i = 0; i < a->count; i++) {
    lval_print(a->cell[i]); putchar(' ');
  }

  /* Print a newline and delete arguments */
  putchar('\n');
  lval_del(a);

  return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
  LASSERT_NUM(a, 1);
  LASSERT_TYPE("error", a, 0, LVAL_STR);

  /* Construct Error from first argument */
  lval* err = lval_err(a->cell[0]->str);

  /* Delete arguments and return */
  lval_del(a);
  return err;
}

lval* lval_join(lval* x, lval* y){
    while(y->count){
        x = lval_add(x, lval_pop(y, 0));
    }

    lval_del(y);
    return x;
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
  lval* k = lval_sym(name);
  lval* v = lval_fun(func);
  lenv_put(e, k, v);
  lval_del(k); 
  lval_del(v);
}

void lenv_add_builtins(lenv* e) {
    
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "exit", builtin_exit);
    lenv_add_builtin(e, "\\", builtin_lambda);
    lenv_add_builtin(e, "lambda", builtin_lambda);
    lenv_add_builtin(e, "lamb", builtin_lambda);
    lenv_add_builtin(e, "fun", builtin_fun);
    lenv_add_builtin(e, "=",   builtin_put);
    lenv_add_builtin(e, "print",   builtin_print);
    lenv_add_builtin(e, "error",   builtin_error);
    lenv_add_builtin(e, "load", builtin_load);

    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "add", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "sub", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "mul", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    lenv_add_builtin(e, "div", builtin_div);
    lenv_add_builtin(e, "%", builtin_mod);
    lenv_add_builtin(e, "mod", builtin_mod);
    lenv_add_builtin(e, "<", builtin_less);
    lenv_add_builtin(e, ">", builtin_greater);
    lenv_add_builtin(e, "==", builtin_equals);
    lenv_add_builtin(e, "!=", builtin_inequals);
    lenv_add_builtin(e, "<=", builtin_lessequals);
    lenv_add_builtin(e, ">=", builtin_greaterequals);
    lenv_add_builtin(e, "if", builtin_if);
    lenv_add_builtin(e, "&&", builtin_and);
    lenv_add_builtin(e, "||", builtin_or);
    lenv_add_builtin(e, "!", builtin_not);

    // add_lisply_fun(e, "fun", "def {fun} (\\ {args body} {def (head args) (\\ (tail args) body)})");
    // add_lisply_fun(e, "unpack", "fun {unpack f xs} {eval (join (list f) xs)}");
    // add_lisply_fun(e, "pack", "fun {pack f & xs} {f xs}");
    // add_lisply_fun(e, "len", "(fun {len l} { if (== l {}) {0} {+ 1 (len (tail l))} })");
    // add_lisply_fun(e, "reverse", "(fun {reverse l} { if (== l {}) {{}} {join (reverse (tail l)) (head l)} })");
    // add_lisply_fun(e, "getElement", "fun {getElement i l} { if (> i (- (len l) 1)) { {} }{ if (== i 0) { head l } { getElement (tail l) (- i 1) } } }");
    // add_lisply_fun(e, "isPresent", "fun {isPresent i l} { if (== l {}){ 0 }{ if (== (head l) (list i)){ 1 }{ (isPresent i (tail l)) } } }");
    // add_lisply_fun(e, "last", "fun {last l} {head (reverse l)}");

    add_lisply_fun(e, "stdlib", "load \"./stdLib.lisplib\"");
}

void add_lisply_fun(lenv* e, char* name, char* command){
    mpc_result_t r;
    if(mpc_parse(name, command, Lisply, &r)){
        lval* result = lval_eval(lval_read(r.output), e);
        lval_del(result);
        mpc_ast_delete(r.output);
    }
}