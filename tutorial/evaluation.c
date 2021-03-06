#include "mpc.h"

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

/* Use operator stirng to see wich operation to perform */
long eval_op(long x, char* op, long y) {
  if ( strcmp(op, "+") == 0 || strcmp(op, "add") == 0 ) { return x + y; }
  if ( strcmp(op, "-") == 0 || strcmp(op, "sub") == 0 ) { return x - y; }
  if ( strcmp(op, "*") == 0 || strcmp(op, "mul") == 0 ) { return x * y; }
  if ( strcmp(op, "/") == 0 || strcmp(op, "div") == 0) { return x / y; }
  if ( strcmp(op, "%") == 0 || strcmp(op, "rem") == 0) { return x / y; }
  if ( strcmp(op, "^") == 0 || strcmp(op, "pow") == 0) { return pow(x, y); }
  return 0;
}
long count_leaves(mpc_ast_t* t) {
  long l = 0;
  if (strstr(t->tag, "number")) return ++l;
  l++;
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    l+= count_leaves(t->children[i]);
    i++;
  }
  return l;
}
#include <limits.h>
long min(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) return strtol(t->contents, NULL, 10);
  // Arguments begin at 2
  int i = 2;
  long a = LONG_MAX;
  long b;
  while (strstr(t->children[i]->tag, "number")) {
    b = strtol(t->children[i]->contents, NULL, 10);
    a = a < b ? a : b;
    i++;
  }
  return a;
}
long max(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) return strtol(t->contents, NULL, 10);
  // Arguments begin at 2
  int i = 2;
  long a = LONG_MIN;
  long b;
  while (strstr(t->children[i]->tag, "number")) {
    b = strtol(t->children[i]->contents, NULL, 10);
    a = a > b ? b : a;
    i++;
  }
  return a;
}

long count_branches(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) return 0;
  long b = 1;
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    b += strstr(t->children[i]->tag, "number") == 0 ? count_branches(t->children[i]) : 0;
    i++;
  }
  return b;
}
long eval(mpc_ast_t* t) {
  /* If tagged as number return it directly */
  if (strstr( t->tag, "number")) {
    return atoi(t->contents);
  }

  /* The operator is always second child */
  char* op = t->children[1]->contents;

  if (strcmp(op, "min") == 0) {
    return min(t);
  }
  if (strcmp(op, "max") == 0) {
    return max(t);
  }
  
  /* We store the third child in 'x' */
  long x = eval(t->children[2]);

  /* Iterate the remaining children and combining */
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}

int main(int argc, char** argv) {

  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                               \
      number    : /-?[0-9]+\\.?[0-9]*/ ;                            \
      operator  : '+' | '-' | '*' | '/' | '%' | '^'                 \
                | \"add\" | \"sub\" | \"mul\" | \"div\"             \
                | \"rem\" | \"pow\" | \"min\" | \"max\" ;           \
      expr     : <number> | '(' <operator> <expr>+ ')' ;            \
      lispy    : /^/ <operator> <expr>+ /$/ ;                       \
    ",
    Number, Operator, Expr, Lispy);

  puts("Lispy Version 0.0.0.0.3");
  puts("Press Ctrl+c to Exit\n");

  while (1) {

    char* input = readline("lispy> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      long l = count_leaves(r.output);
      printf("Leaves: %li\n", l);
      long b = count_branches(r.output);
      printf("Branches: %li\n", b);
      long result = eval(r.output);
      printf("%li\n", result);
      mpc_ast_delete(r.output);

    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);

  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);

  return 0;
}
