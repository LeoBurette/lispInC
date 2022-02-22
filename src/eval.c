#include "eval.h"
#include "util.h"

lval* eval_op(lval* x, char* operator, lval* y){
    if(strcmp(operator, "+")==0 || strcmp(operator, "add")==0){
        return lval_num(x->num + y->num);
    }else if(strcmp(operator, "-")==0 || strcmp(operator, "sub")==0){
        return lval_num(x->num - y->num);
    }else if(strcmp(operator, "*")==0 || strcmp(operator, "mul")==0){
        return lval_num(x->num * y->num);
    }else if(strcmp(operator, "/")==0 || strcmp(operator, "div")==0){
        return (y->num==0)?lval_err("Error: Division by zero!\n"):lval_num(x->num / y->num);
    }else if(strcmp(operator, "\%")==0){
        if(!isInteger(x->num) || !isInteger(y->num)){
            return lval_err("Error: Invalid Number!\n");
        }
        int a = x->num;
        int b = y->num;
        return lval_num(a % b);
    }else if(strcmp(operator, "^")==0){
        return lval_num(pow(x->num, y->num));
    }else if(strcmp(operator, "min")==0){
        return (x->num < y->num)?x:y;
    }else if(strcmp(operator, "max")==0){
        return (x->num > y->num)?x:y;
    }else{
        return lval_err("Error: Invalid Number!\n");
    }
}

lval* single_eval_op(lval* x, char* operator){
    if(strcmp(operator, "-")==0){
        return lval_num(-x->num);
    }else{
        return lval_err("Error: Invalid Operator!\n");
    }
}

lval* lval_read_num(mpc_ast_t* t){
    errno = 0;
    double x =strtod(t->contents, NULL);
    return errno != ERANGE ? lval_num(x) : lval_err("Error: Invalid Number!\n");
}

lval* lval_read(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
    if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
        if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

lval* eval(mpc_ast_t* t){
    if(strstr(t->tag, "number")){
        errno = 0;
        double x = strtod(t->contents, NULL);
        return errno != ERANGE ? lval_num(x) : lval_err("Error: Invalid Number!\n");
    }

    char* operator = t->children[1]->contents;

    lval* x = eval(t->children[2]);

    int i = 3;
    while(strstr(t->children[i]->tag, "expression")){
        x = eval_op(x, operator, eval(t->children[i]));
        i++;
    }
    if(i==3){
        x = single_eval_op(x, operator);
    }

    return x;
}

lval* lval_eval_sexpr(lval* v){
    for(int i = 0; i < v->count; i++){
        v->cell[i] = lval_eval(v->cell[i]);
    }

    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) { 
            return lval_take(v, i); 
        }
    }

    if (v->count == 0) { 
        return v; 
    }

    if (v->count == 1) { 
        return lval_take(v, 0); 
    }

    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_SYM) {
        lval_del(f); 
        lval_del(v);
        return lval_err("S-expression Does not start with symbol!");
    }

    lval* result = builtin(v, f->sym);
    lval_del(f);
    return result;
}

lval* lval_eval(lval* v) {
    if (v->type == LVAL_SEXPR) { 
        return lval_eval_sexpr(v); 
    }
    
    return v;
}

lval* builtin_op(lval* a, char* op){
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
        }else{
            x = lval_err("Error: Invalid Number!\n");
        }

        lval_del(y);
    }

    lval_del(a);
    return x;
}

lval* builtin_head(lval* a){
    LASSERT(a, a->count==1, "Function 'head' passed too many arguments!");

    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,"Function 'head' passed incorrect type!");
    
    LASSERT(a, a->cell[0]->count != 0,"Function 'head' passed {}!");

    lval* v = lval_take(a, 0);

    while (v->count > 1){
        lval_del(lval_pop(v, 1));
    }
    return v;
}

lval* builtin_tail(lval* a){
    LASSERT(a, a->count == 1,"Function 'tail' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'tail' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0,"Function 'tail' passed {}!");

    lval* v = lval_take(a, 0);

    lval_del(lval_pop(v, 0));
    return v;
}

lval* builtin_list(lval* a){
    a->type = LVAL_QEXPR;
    return a;
}

lval* builtin_eval(lval* a){
    LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");

    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,"Function 'eval' passed incorrect type!");

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

lval* builtin_join(lval* a){
    for (int i = 0; i < a->count; i++) {
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR,"Function 'join' passed incorrect type.");
    }

    lval* x = lval_pop(a, 0);

    while(a->count){
        x = lval_join(x, lval_pop(a, 0));
    }

    lval_del(a);
    return x;
}


lval* builtin_cons(lval* a){
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

lval* builtin_init(lval* a){
    LASSERT(a, a->count==1, "Function 'init' passed too many arguments!");

    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,"Function 'init' passed incorrect type!");
    
    LASSERT(a, a->cell[0]->count != 0,"Function 'init' passed {}!");

    lval* x = lval_take(a, 0);
    lval_pop(x, x->count-1);
    return x;
};
lval* builtin_len(lval* a){
    LASSERT(a, a->count = 1, "Need 1 arguments");

    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'len' need a qexpr");

    lval* x = lval_num(lval_pop(a, 0)->count);
    return x;
};

lval* lval_join(lval* x, lval* y){
    while(y->count){
        x = lval_add(x, lval_pop(y, 0));
    }

    lval_del(y);
    return x;
}

lval* builtin(lval* a, char* func) {
    if (strcmp("list", func) == 0) { return builtin_list(a); }
    if (strcmp("head", func) == 0) { return builtin_head(a); }
    if (strcmp("tail", func) == 0) { return builtin_tail(a); }
    if (strcmp("join", func) == 0) { return builtin_join(a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(a); }
    if (strcmp("cons", func) == 0) { return builtin_cons(a); }
    if (strcmp("len", func) == 0) { return builtin_len(a); }
    if (strcmp("init", func) == 0) { return builtin_init(a); }
    if (strstr("+-/*", func)) { return builtin_op(a, func); }
    lval_del(a);
    return lval_err("Unknown Function!");
}