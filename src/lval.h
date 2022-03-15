#ifndef LVAL_H__
#define LVAL_H__

struct lval;
typedef struct lval lval;
struct lenv;
typedef struct lenv lenv;

typedef lval*(*lbuiltin)(lenv*, lval*);

struct lval {
    int type;

    double num;

    char* err;

    char* sym;
    
    char* str;

    lbuiltin builtin;

    lenv* env;
    lval* formals;
    lval* body;

    int count;
    struct lval** cell; 
};

enum lval_type { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_FUN, LVAL_STR} ;
char* ltype_name(int t);

lval* lval_num(double x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_fun(lbuiltin builtin);
lval* lval_lambda(lval* formals, lval* body);
lval* lval_str(char* str);

lval* lval_add(lval* v, lval* x);

void lval_print(lval* v);
void lval_print_str(lval* v);
void lval_del(lval* v);
void lval_expr_print(lval* v, char open, char close);
void lval_println(lval* v);

lval* lval_copy(lval* v);

lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);

int lval_equal(lval* a, lval* b);

#endif