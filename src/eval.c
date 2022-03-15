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

lval* lval_read_bool(mpc_ast_t* t){
    double x = atoi(t->contents);
    return lval_num(x);
}

lval* lval_read(mpc_ast_t* t) {
    if (strstr(t->tag, "lstring")) { return lval_read_str(t);};
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "lbool")) {return lval_read_bool(t); }
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
        if (strstr(t->children[i]->tag, "comment")) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

lval* lval_read_str(mpc_ast_t* t){
    
    t->contents[strlen(t->contents)-1] = '\0';
    
    char* unescaped = malloc(strlen(t->contents+1)+1);
    strcpy(unescaped, t->contents+1);
    
    unescaped = mpcf_unescape(unescaped);
    
    lval* str = lval_str(unescaped);
    
    free(unescaped);
    return str;
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

lval* lval_eval_sexpr(lval* v, lenv* e){
    for(int i = 0; i < v->count; i++){
        v->cell[i] = lval_eval(v->cell[i], e);
    }

    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) { 
            return lval_take(v, i); 
        }
    }

    if (v->count == 0) { 
        return v; 
    }
    if (v->count == 1 && v->cell[0]->builtin != builtin_exit) { 
        return lval_eval(lval_take(v, 0), e); 
    }

    lval* f = lval_pop(v, 0);

    if (f->type != LVAL_FUN) {
        lval_del(f); 
        lval_del(v);
        return lval_err("first element is not a function");
    }

    lval* result = lval_call(e, f, v);
    lval_del(f);
    return result;
}

lval* lval_eval(lval* v, lenv* e) {
    if(v->type == LVAL_SYM){
        lval* x = lenv_get(e, v);
        lval_del(v);
        return x;
    }

    if (v->type == LVAL_SEXPR) { 
        return lval_eval_sexpr(v, e); 
    }
    
    return v;
}

lval* lval_call(lenv* e, lval* f, lval*a){
    if(f->builtin){
        return f->builtin(e, a);
    }

    int given = a->count;
    int total = f->formals->count;

    while (a->count) {

        if (f->formals->count == 0) { 
            lval_del(a); 
            return lval_err("Function passed too many arguments. Got %i, Expected %i.", given, total);
        }

        lval* sym = lval_pop(f->formals, 0);

        if(strcmp(sym->sym, "&") == 0 || strcmp(sym->sym, "...") == 0){
            if( f->formals->count != 1){
                lval_del(a);
                return lval_err("Function format invalid. Symbol '&' not followed by single symbol.");
            }

            lval* nsym = lval_pop(f->formals, 0);
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_del(sym);
            lval_del(nsym);
            break;
        }

        lval* val = lval_pop(a, 0);

        lenv_put(f->env, sym, val);

        lval_del(sym);
        lval_del(val);
    }

    lval_del(a);

    if (f->formals->count > 0 && (strcmp(f->formals->cell[0]->sym, "&") == 0 || strcmp(f->formals->cell[0]->sym, "...") == 0)) {
        if (f->formals->count != 2) {
            return lval_err("Function format invalid. "
            "Symbol '&' not followed by single symbol.");
        }

        lval_del(lval_pop(f->formals, 0));

        lval* sym = lval_pop(f->formals, 0);
        lval* val = lval_qexpr();

        lenv_put(f->env, sym, val);
        lval_del(sym); lval_del(val);
    }

    if(f->formals->count == 0){
        f->env->parent = e;

        return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    }else{
        return lval_copy(f);
    }
    
}