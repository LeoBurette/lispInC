#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lib/mpc-master/mpc.h"

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
long eval_op(long x, char* operator, long y){
    if(strcmp(operator, "+")==0 || strcmp(operator, "add")==0){
        return x + y;
    }else if(strcmp(operator, "-")==0 || strcmp(operator, "sub")==0){
        return x - y;
    }else if(strcmp(operator, "*")==0 || strcmp(operator, "mul")==0){
        return x * y;
    }else if(strcmp(operator, "/")==0 || strcmp(operator, "div")==0){
        return x / y;
    }else if(strcmp(operator, "\%")==0){
        return x % y;
    }else if(strcmp(operator, "^")==0){
        return pow(x, y);
    }else if(strcmp(operator, "min")==0){
        return (x < y)?x:y;
    }else if(strcmp(operator, "max")==0){
        return (x > y)?x:y;
    }else{
        return 0;
    }
}

long single_eval_op(long x, char* operator){
    if(strcmp(operator, "-")==0){
        return -x;
    }else{
        return 0;
    }
}

long eval(mpc_ast_t* t){
    if(strstr(t->tag, "number")){
        return atoi(t->contents);
    }

    char* operator = t->children[1]->contents;

    long x = eval(t->children[2]);

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

int main(int argc, char** argv) {
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expression = mpc_new("expression");
    mpc_parser_t* Lisply = mpc_new("lisply");

    mpca_lang(MPCA_LANG_DEFAULT, "\
        number     : /-?[0-9]+(\\.[0-9]+)?/ ;\
        operator   : '+' | '-' | '*' | '/' | '\%' | \"add\" | \"mul\" | \"div\" | \"sub\" | '^' | \"min\" | \"max\";\
        expression : <number> | '(' <operator> <expression>+ ')' ;\
        lisply     : /^/ <operator> <expression>+ /$/ ;\
    ", Number, Operator, Expression, Lisply);

    puts("Lisply REPL");
    puts("Press ctrl-c to exit \n");

    while (1){
        char* input = readline("lisply >> ");
        add_history(input);
        
        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Lisply, &r)){
            long result = eval(r.output);
            printf("%li\n", result);
            mpc_ast_delete(r.output);
        }else{
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }
    
    mpc_cleanup(4, Number, Operator, Expression, Lisply);
    return 0;
}