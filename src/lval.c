#include <stdio.h>
#include "lval.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

lval* lval_num(double x){
    lval* v = malloc(sizeof(lval));
    v->num = x;
    v->type = LVAL_NUM;
    return v;
}

lval* lval_err(char* err){
    lval* v = malloc(sizeof(lval));
    v->err = malloc(sizeof(strlen(err)+1));
    strcpy(v->err, err);
    v->type = LVAL_ERR;
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

void lval_del(lval* v){
    switch(v->type){
        case LVAL_NUM:
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
        
        case LVAL_ERR:
            printf("%s", v->err);
            break;

        case LVAL_SYM:   printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
        case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
    }
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

lval* lval_take(lval* v, int i){
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}