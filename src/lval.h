#ifndef LVAL_H__
#define LVAL_H__

typedef struct lval {
    int type;

    double num;

    char* err;

    char* sym;

    int count;
    struct lval** cell; 
} lval;

enum lval_type { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR } ;

lval* lval_num(double x);
lval* lval_err(char* err);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);

lval* lval_add(lval* v, lval* x);

void lval_print(lval* v);
void lval_del(lval* v);
void lval_expr_print(lval* v, char open, char close);
void lval_println(lval* v);


lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);

#endif