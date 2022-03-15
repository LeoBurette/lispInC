#ifndef PARSING_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../lib/mpc-master/mpc.h"
#include "lval.h"
#include "eval.h"
#include "lenv.h"
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
mpc_parser_t* Lisply;
mpc_parser_t* Number;
mpc_parser_t* Symbol;
mpc_parser_t* Sexpr;
mpc_parser_t* Qexpr;
mpc_parser_t* Expression;
mpc_parser_t* Lstring;
mpc_parser_t* Comment;

void setLisply();
void cleanLisply();

#endif