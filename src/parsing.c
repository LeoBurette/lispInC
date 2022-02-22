#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../lib/mpc-master/mpc.h"
#include "lval.h"
#include "eval.h"

#ifdef _WIN32
#include <string.h>
static char buffer[2048];

char* readline(char* prompt){
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = "\0";
    return cpy;
}

void add_history(char* unused){}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

int main(int argc, char** argv) {
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Expression = mpc_new("expression");
    mpc_parser_t* Lisply = mpc_new("lisply");

    mpca_lang(MPCA_LANG_DEFAULT, "\
        number     : /-?[0-9]+(\\.[0-9]+)?/ ;\
        symbol     : '+' | '-' | '*' | '/' | '\%' | \"add\" | \"mul\" | \"div\" | \"sub\" | '^' | \"min\" | \"max\" | \"list\" | \"head\" | \"tail\" | \"join\" | \"eval\" | \"cons\" | \"len\" | \"init\";\
        sexpr      : '(' <expression>* ')';\
        qexpr      : '{' <expression>* '}';\
        expression : <number> | <symbol> | <sexpr> | <qexpr> ;\
        lisply     : /^/ <expression>* /$/ ;\
    ", Number, Symbol, Expression, Lisply, Sexpr, Qexpr);

    puts("Lisply REPL");
    puts("Press ctrl-c to exit \n");

    while (1){
        char* input = readline("lisply >> ");
        add_history(input);
        
        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Lisply, &r)){
            lval* result = lval_eval(lval_read(r.output));
            lval_println(result);
            lval_del(result);
            mpc_ast_delete(r.output);
        }else{
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }
    
    mpc_cleanup(6, Number, Symbol, Expression, Lisply, Sexpr, Qexpr);
    return 0;
}