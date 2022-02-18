#include "mpc.h"
#include <math.h>
#ifdef _WIN32

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

struct lval;
struct lval;
typedef struct lval lval;
typedef struct lenv lenv;
typedef lval*(*lbuiltin)(lenv*, lval*);

mpc_parser_t* Number;
mpc_parser_t* Symbol;
mpc_parser_t* String;
mpc_parser_t* Comment;
mpc_parser_t* Sexpr;
mpc_parser_t* Qexpr;
mpc_parser_t* Expr;
mpc_parser_t* Lispy;

/* Lisp Value */
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_STR,
       LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };

struct lval {
  int type;

  /* Basic */
  double num;
  char* err;
  char* sym;
  char* str;
  
  /* Functions */
  lbuiltin builtin;
  lenv *env;
  lval *formals;
  lval *body;

  /* Expression */
  int count;
  lval** cell;
};

struct lenv {
  lenv *par;
  int count;
  char** syms;
  lval** vals;
};

lval *lval_add(lval*, lval*);
lval* lval_builtin(lbuiltin);
lval *lval_cons(lval*, lval*);
lval *lval_copy(lval*);
lval *lval_err(char*, ...);
int lval_eq(lval*, lval*);
lval *lval_eval(lenv*, lval*);
lval *lval_eval_sexpr(lenv*, lval*);
lval *lval_init(lval*);
lval *lval_join(lval*, lval*);
lval *lval_lambda(lval*, lval*);
lval *lval_nth(lval*, int);
lval *lval_num(long);
lval *lval_pop(lval*, int);
lval *lval_qexpr(void);
lval *lval_read(mpc_ast_t*);
lval *lval_read_num(mpc_ast_t*);
lval *lval_read_str(mpc_ast_t *);
lval *lval_sexpr(void);
lval *lval_str(char*);
lval *lval_sym(char*);
lval *lval_take(lval*, int);
void lval_del(lval*);
void lval_print(lval *);
void lval_print_expr(lval*, char, char);
void lval_print_str(lval*);
void lval_println(lval*);

lenv *lenv_copy(lenv *);
lenv *lenv_new(void);
lval *lenv_get(lenv*, lval*);
void lenv_add_builtin(lenv*, char*, lbuiltin);
void lenv_add_builtins(lenv* e);
void lenv_def(lenv *, lval *, lval *);
void lenv_del(lenv*);
void lenv_put(lenv*, lval*, lval*);

char *ltype_name(int);

lval *builtin_cmp(lenv*, lval*, char*);
lval *builtin_def(lenv*, lval*);
lval *builtin_eq(lenv*, lval*);
lval *builtin_error(lenv*, lval*);
lval *builtin_ge(lenv*, lval*);
lval *builtin_gt(lenv*, lval*);
lval *builtin_if(lenv*, lval*);
lval *builtin_le(lenv*, lval*);
lval *builtin_load(lenv*, lval*);
lval *builtin_lt(lenv*, lval*);
lval *builtin_ne(lenv*, lval*);
lval *builtin_ord(lenv*, lval*, char*);
lval *builtin_print(lenv*, lval*);
lval *builtin_put(lenv*, lval*);
lval *builtin_add(lenv*, lval*);
lval *builtin_cons(lenv*, lval*);
lval *builtin_div(lenv*, lval*);
lval *builtin_eval(lenv*, lval*); 
lval *builtin_exit(lenv*, lval*);
lval *builtin_front(lenv*, lval*);
lval *builtin_head(lenv*, lval*);
lval *builtin_init(lenv*, lval*); 
lval *builtin_join(lenv*, lval*);
lval *builtin_lambda(lenv*, lval*);
lval *builtin_len(lenv*, lval*); 
lval *builtin_list(lenv*, lval*);
lval *builtin_mul(lenv*, lval*);
lval *builtin_nth(lenv*, lval*);
lval *builtin_op(lenv*, lval*, char*);
lval *builtin_pow(lenv*, lval*);
lval *builtin_rem(lenv*, lval*);
lval *builtin_sub(lenv*, lval*a);
lval *builtin_tail(lenv*, lval*);
lval *builtin_var(lenv*, lval*, char*);

#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

#define LASSERT_TYPE(func, args, index, expect) \
  LASSERT(args, args->cell[index]->type == expect, \
      "Function '%s' passed incorrect type for argument %i. Got %s, Expcted %s.", \
      func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num) \
  LASSERT(args, args->count == num, \
      "Function '%s' passed incorrect number of arguments. Got %i, Expected %i.", \
      func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
  LASSERT(args, args->cell[index]->count != 0, \
      "Function '%s' passed {} for argument %i.", func, index);
