#include <stdio.h>
#include "lval.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "lenv.h"
#include "../lib/mpc-master/mpc.h"

lval* lval_num(double x){
    lval* v = malloc(sizeof(lval));
    v->num = x;
    v->type = LVAL_NUM;
    return v;
}

lval* lval_err(char* fmt, ...){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;

    va_list va;
    va_start(va, fmt);

    v->err = malloc(512);
    vsnprintf(v->err, 511, fmt, va);

    v->err = realloc(v->err, strlen(v->err)+1);

    va_end(va);
    return v;
}

lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

lval* lval_sexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval* lval_qexpr(void){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval* lval_fun(lbuiltin builtin){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->builtin = builtin;
    return v;
}

lval* lval_str(char* str) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_STR;
  v->str = malloc(strlen(str) + 1);
  strcpy(v->str, str);
  return v;
}

lval* lval_lambda(lval* formals, lval* body){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;

    v->env = lenv_new();

    v->builtin = NULL;

    v->body = body;
    v->formals = formals;

    return v;
}

void lval_del(lval* v){
    switch(v->type){
        case LVAL_NUM:
            break;
        case LVAL_FUN:
            if(!v->builtin){
                lval_del(v->body);
                lenv_del(v->env);
                lval_del(v->formals);
            }
            break;
        case LVAL_STR:
            free(v->str);
            break;
        case LVAL_ERR:
            free(v->err);
            break;
        case LVAL_SYM:
            free(v->sym);
            break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            for(int i = 0; i < v->count; i++){
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
    }

    free(v);
}

lval* lval_add(lval* v, lval* x){
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*)*v->count);
    v->cell[v->count-1] = x;
    return v;
}

void lval_print(lval* v){
    switch (v->type){
        case LVAL_NUM:
            if(isInteger(v->num)){
                printf("%li", (long)(v->num));
            }else{
                printf("%lf", v->num);
            }
            break;
        
        case LVAL_STR:
            lval_print_str(v);
            break;

        case LVAL_ERR:
            printf("Error : %s", v->err);
            break;

        case LVAL_SYM:   printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
        case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
        case LVAL_FUN: 
            if(v->builtin){
                printf("<function>");
            }else{
                printf("(\\");
                lval_print(v->formals);
                putchar(' ');
                lval_print(v->body);
                putchar(')');
            } 
            break;
    }
}

void lval_print_str(lval* v){
    char* escaped = malloc(strlen(v->str)+1);
    strcpy(escaped, v->str);
    escaped = mpcf_escape(escaped);
    printf("\"%s\"", escaped);
    free(escaped);
}

lval* lval_copy(lval* v){
    lval* next = malloc(sizeof(lval));
    next->type = v->type;
    switch (next->type){
        case LVAL_NUM:
            next->num = v->num;
            break;
        case LVAL_ERR:
            next->err = malloc(strlen(v->err)+1);
            strcpy(next->err, v->err);
            break;
        case LVAL_SYM:
            next->sym = malloc(strlen(v->sym)+1);
            strcpy(next->sym, v->sym);
            break;
        case LVAL_STR:
            next->str = malloc(strlen(v->str) + 1);
            strcpy(next->str, v->str);
            break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            next->count = v->count;
            next->cell = malloc(sizeof(lval*) * next->count);
            for (int i = 0; i < next->count; i++) {
                next->cell[i] = lval_copy(v->cell[i]);
            }
            break;
        case LVAL_FUN:
            if(v->builtin){
                next->builtin = v->builtin;
            }else{
                next->builtin = NULL;
                next->body = lval_copy(v->body);
                next->formals = lval_copy(v->formals);
                next->env = lenv_copy(v->env);
            }
            break;
    }
    return next;
}

void lval_expr_print(lval* v, char open, char close){
    putchar(open);
    for(int i = 0; i < v->count; i++){
        lval_print(v->cell[i]);

        if(i != (v->count -1)){
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_println(lval* v) {
     lval_print(v); 
     putchar('\n'); 
}


lval* lval_pop(lval* v, int i){
    lval* x = v->cell[i];

    memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*)*(v->count-i-1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(lval*) * v->count);

    return x;
}

int lval_equal(lval* a, lval* b){
    if(a->type != b->type){return 0;}

    switch (a->type){
    case LVAL_NUM:
        return a->num == b->num;
    case LVAL_ERR:
        return strcmp(a->err, b->err);
    case LVAL_SYM: 
        return (strcmp(a->sym, b->sym) == 0);
    case LVAL_FUN:
        if (a->builtin || b->builtin) {
            return a->builtin == b->builtin;
        } else {
            return lval_equal(a->formals, b->formals) && lval_equal(a->body, b->body);
        }
    case LVAL_STR:
        return (strcmp(a->str, b->str)==0);
    case LVAL_QEXPR:
    case LVAL_SEXPR:
        if (a->count != b->count) { 
            return 0; 
        }
        for (int i = 0; i < a->count; i++) {
            if (!lval_equal(a->cell[i], b->cell[i])) { 
                return 0; 
            }
        }
        
        return 1;
    }
    return 0;
}

lval* lval_take(lval* v, int i){
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

char* ltype_name(int t) {
  switch(t) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    case LVAL_STR: return "String";
    default: return "Unknown";
  }
}