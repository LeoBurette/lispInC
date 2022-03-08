#include "parsing.h"

void setLisply(){
    Number = mpc_new("number");
    Symbol = mpc_new("symbol");
    Sexpr = mpc_new("sexpr");
    Qexpr = mpc_new("qexpr");
    Expression = mpc_new("expression");
    Lisply = mpc_new("lisply");

    mpca_lang(MPCA_LANG_DEFAULT, "\
        number     : /-?[0-9]+(\\.[0-9]+)?/ ;\
        symbol     : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;\
        sexpr      : '(' <expression>* ')';\
        qexpr      : '{' <expression>* '}';\
        expression : <number> | <symbol> | <sexpr> | <qexpr> ;\
        lisply     : /^/ <expression>* /$/ ;\
    ", Number, Symbol, Expression, Lisply, Sexpr, Qexpr);
}

void cleanLisply(){
    mpc_cleanup(6, Number, Symbol, Expression, Lisply, Sexpr, Qexpr);
}

int main(int argc, char** argv) {
    setLisply();

    puts("Lisply REPL");
    puts("Press ctrl-c to exit \n");

    lenv* e = lenv_new();
    lenv_add_builtins(e);

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
    
    lenv_del(e);
    cleanLisply();
    
    return 0;
}