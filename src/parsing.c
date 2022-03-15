#include "parsing.h"

void setLisply(){
    Number = mpc_new("number");
    Symbol = mpc_new("symbol");
    Sexpr = mpc_new("sexpr");
    Qexpr = mpc_new("qexpr");
    Expression = mpc_new("expression");
    Lisply = mpc_new("lisply");
    Lstring = mpc_new("lstring");
    Comment = mpc_new("comment");

    mpca_lang(MPCA_LANG_DEFAULT, "\
        lstring    : /\"(\\\\.|[^\"])*\"/ ;\
        number     : /-?[0-9]+(\\.[0-9]+)?/ ;\
        symbol     : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;\
        sexpr      : '(' <expression>* ')';\
        qexpr      : '{' <expression>* '}';\
        comment    : /;[^\\r\\n]*/ ;\
        expression : <number> | <symbol> | <sexpr> | <qexpr> | <lstring> | <comment> ;\
        lisply     : /^/ <expression>* /$/ ;\
    ", Number, Symbol, Expression, Lisply, Sexpr, Qexpr, Lstring, Comment);
}

void cleanLisply(){
    mpc_cleanup(8, Number, Symbol, Expression, Lisply, Sexpr, Qexpr, Lstring, Comment);
}

int main(int argc, char** argv) {
    setLisply();

    puts("Lisply REPL");
    puts("Press ctrl-c to exit \n");

    lenv* e = lenv_new();
    lenv_add_builtins(e);

    if (argc == 1) {
        while (1){
            char* input = readline("lisply >> ");
            add_history(input);
            
            mpc_result_t r;
            if(mpc_parse("<stdin>", input, Lisply, &r)){
                lval* result = lval_eval(lval_read(r.output), e);
                lval_println(result);
                lval_del(result);
                mpc_ast_delete(r.output);
            }else{
                mpc_err_print(r.error);
                mpc_err_delete(r.error);
            }

            free(input);
        }
    }
    if (argc >= 2) {
        /* loop over each supplied filename (starting from 1) */
        for (int i = 1; i < argc; i++) {
        
        /* Argument list with a single argument, the filename */
        lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));
        
        /* Pass to builtin load and get the result */
        lval* x = builtin_load(e, args);
        
        /* If the result is an error be sure to print it */
        if (x->type == LVAL_ERR) { lval_println(x); }
        lval_del(x);
        }
    }
    
    lenv_del(e);
    cleanLisply();
    
    return 0;
}