#include "lispy.h"

/* Construct a pointer to a nev Number lval */
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

/* Construct a pointer to a new Error lval */
lval* lval_err(char* fmt, ...) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;

  /* Create a va list and initialize it */
  va_list va;
  va_start(va, fmt);

  /* Allocate 512 bytes of space */
  v->err = malloc(512);

  /* printf the error string with a maximum of 511 characters */
  vsnprintf(v->err, 511, fmt, va);

  /* Reallocate to number of bytes actually used */
  v->err = realloc(v->err, strlen(v->err)+1);

  /* Cleanup our va list */
  va_end(va);

  return v;
}

/* Construct a pointer to a new Symbol lval */
lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

lval* lval_builtin(lbuiltin func) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->builtin = func;
  return v;
}
lval* lval_lambda(lval* formals, lval* body) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;

  /* Set BuiltIn to Null */
  v->builtin = NULL;

  /* Build new environment */
  v->env = lenv_new();

  /* Set Formals and Body */
  v->formals = formals;
  v->body = body;
  return v;
}
/* A pointer to a new empty Sexpr lval */
lval* lval_sexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

/* A pointer to a new empty QExpr lval */
lval* lval_qexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}
lval *lval_str(char *s) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_STR;
  v->str = malloc(strlen(s) + 1);
  strcpy(v->str, s);
  return v;
}
void lenv_del(lenv *e);

void lval_del(lval* v) {

  switch (v->type) {
    case LVAL_FUN: 
      if (!v->builtin) {
        lenv_del(v->env);
        lval_del(v->formals);
        lval_del(v->body);
      }
      break;
    case LVAL_NUM: break;
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;
    case LVAL_STR: free(v->str); break;

    case LVAL_QEXPR:
    case LVAL_SEXPR:
      for (int i = 0; i < v-> count; i++) {
        lval_del(v->cell[i]);
      }
      /* Also free the memory allocated to contain the poniters */
      free(v->cell);
    break;
  }

  /* Free the memory allocated or the "lval" struct itself */
  free(v);
}

lval* lval_copy(lval* v) {
  lval* x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type) {

    /* copy functions and numbers directly */
    case LVAL_FUN: 
      if (v->builtin) {
        x->builtin = v->builtin;
      } else {
        x-> builtin = NULL;
        x->env = lenv_copy(v->env);
        x->formals = lval_copy(v->formals);
        x->body = lval_copy(v->body);
      }
      break;
    case LVAL_NUM:
      x->num = v->num;
     break;
    case LVAL_STR: x->str = malloc(strlen(v->str) + 1);
      strcpy(x->str, v->str); break;

    /* copy strings using malloc and strcpy */
    case LVAL_ERR:
      x->err = malloc(strlen(v->err) + 1);
      strcpy(x->err, v->err);
     break;

    case LVAL_SYM:
      x->sym = malloc(strlen(v->sym) + 1);
      strcpy(x->sym, v->sym);
    break;

    /* copy lists by copying each sub-expression */
    case LVAL_SEXPR:
    case LVAL_QEXPR:
      x->count = v->count;
      x->cell = malloc(sizeof(lval*) * x->count);
      for (int i = 0; i < x->count; i++) {
        x->cell[i] = lval_copy(v->cell[i]);
      }
    break;
  }

  return x;
}

lval* lval_add(lval* v, lval* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

lval* lval_join(lval* x, lval* y) {
  for ( int i = 0; i < y->count; i++) {
    x = lval_add(x, y->cell[i]);
  }
  free(y->cell);
  free(y);
  return x;
}


lval* lval_pop(lval* v, int i) {
  /* find the item at "i" */
  lval* x = v->cell[i];

  /* shift memory after the item at "i" over the top */
  memmove(&v->cell[i], &v->cell[i+1],
      sizeof(lval*) * (v->count-i-1));

  /* decrease the count of items in the list */
  v->count--;

  /* reallocate the memory used */
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  return x;
}

lval* lval_take(lval* v, int i) {
  lval* x = lval_pop(v, i);
  lval_del(v);
  return x;
}

void lval_print_expr(lval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {

    /* Print Value contained within */
    lval_print(v->cell[i]);

    /* Don't print trailing space if last element */
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}
void lval_print_str(lval *v) {
  /* Make a Copy of the string */
  char *escaped = malloc(strlen(v->str)+1);
  strcpy(escaped, v->str);
  /* Pass it through the escape function */
  escaped = mpcf_escape(escaped);
  /* Print it between " characters */
  printf("\"%s\"", escaped);
  /* free the copied string */
  free(escaped);
}
/* Print an "lval" */
void lval_print(lval* v) {
  switch (v->type) {
    case LVAL_FUN:    
      if (v->builtin) {
        printf("<builtin>");
      } else {
       printf("(\\ "); lval_print(v->formals);
       putchar(' '); lval_print(v->body); putchar(')');
      }
    break;
    case LVAL_NUM:    printf("%lf", v->num); break; 
    case LVAL_STR:    lval_print_str(v); break;
    case LVAL_ERR:    printf("Error: %s", v->err); break;
    case LVAL_SYM:    printf("%s", v->sym); break;
    case LVAL_SEXPR:  lval_print_expr(v, '(', ')'); break;
    case LVAL_QEXPR:  lval_print_expr(v, '{', '}'); break;
  }
}


/* Print an "lval" followed by a newline */
void lval_println(lval* v) { lval_print(v); putchar('\n'); }


/* Lisp Environment */

int lval_eq(lval *x, lval *y) {

  /* Different Types are always unequal */
  if (x->type != y->type) {
    return 0;
  }

  /* Compare Based upon type */
  switch (x->type) {
    /* Compare Number Value */
    case LVAL_NUM: return (x->num == y->num);
    case LVAL_STR: return (strcmp(x->str, y->str) == 0);

    /* Compare String Values */
    case LVAL_ERR: return (strcmp(x->err, y->err) == 0);
    case LVAL_SYM: return (strcmp(x->sym, y->sym) == 0);

    /* If builtin compare, otherwise compare formals and body */
    case LVAL_FUN:
      if (x->builtin || y->builtin) {
        return x->builtin == y->builtin;
      } else {
        return lval_eq(x->formals, y->formals)
          && lval_eq(x->body, y->body);
      }

    /* If list compare every individual element */
    case LVAL_QEXPR:
    case LVAL_SEXPR:
      if (x->count != y->count) { return 0; }
      for (int i = 0; i < x->count; i++) {
        /* If any element not equal then whole list not equal */
        if (!lval_eq(x->cell[i], y->cell[i])) { return 0; }
      }
      /* Otherwise lists must be equal */
      return 1;
    break;
  }
  return 0;
}
/* Evaluation */

lval *lval_call(lenv *e, lval *f, lval *a) {

  /* If Builtin then simply apply that */
  if (f->builtin) { return f->builtin(e, a); }

  /* Record Arument Counts */
  int given = a->count;
  int total = f->formals->count;

  /* While arguments still remain to be processed */
  /* Assign each arugment to each formal in order */
  while (a->count) {

    /* If we've ran out of formal arguments to bind */
    if (f->formals->count == 0) {
      lval_del(a); return lval_err(
          "Function passed too many arguments. "
          "Got %i, Expected %i.", given, total);
    }


    /* Pop the first symbol from the formals */
    lval *sym = lval_pop(f->formals, 0);

    /* Special Case to deal with '&' */
    if (strcmp(sym->sym, "&") == 0) {

      /* Ensure '&' is followed by another symbol */
      if (f->formals->count != 1) {
        lval_del(a);
        return lval_err("Function format invalid. "
            "Symbol '&' not followed by single symbol.");
      }

      /* Next formal should be bound to remaining arguments */
      lval* nsym = lval_pop(f->formals, 0);
      lenv_put(f->env, nsym, builtin_list(e, a));
      lval_del(sym); lval_del(nsym);
      break;
    }


    /* Pop the next argument from the list */
    lval* val = lval_pop(a, 0);

    /* Bind a copy into the function's environment */
    lenv_put(f->env, sym, val);

    /* Delete symbol and value */
    lval_del(sym); lval_del(val);
  }

  /* Argument list is now bound so can be cleaned up */
  lval_del(a);

  /* If '&' remains in formal list bind to empty list */
  if (f->formals->count > 0 &&
      strcmp(f->formals->cell[0]->sym, "&") == 0) {
    
    /* Check to ensure that & is not passed invalidly */
    if (f->formals->count != 2) {
      return lval_err("Function format invalid. "
          "Symbol '&' not followed by single symbol.");
    }

    /* Pop and delete '&' symbol */
    lval_del(lval_pop(f->formals, 0));

    /* Pop next symbol and creat emtpy list */
    lval *sym = lval_pop(f->formals, 0);
    lval* val = lval_qexpr();

    /* Bind to environment and delete */
    lenv_put(f->env, sym, val);
    lval_del(sym); lval_del(val);
  }

  /* If all formals have been bound evaluate */
  if (f->formals->count == 0) {

    /* Set the parent environment */
    f->env->par = e;

    /* Evaluate the body */
    return builtin_eval(f->env,
        lval_add(lval_sexpr(), lval_copy(f->body)));
  } else {
    /* Otherwise return partially evaluated function */
    return lval_copy(f);
  }
}

lval* lval_eval_sexpr(lenv* e, lval* v) {

  /* Evaluate Children */
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(e, v->cell[i]);
  }
  

  /* Error checking */
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }

  /* Empty Expression */
  if (v->count == 0) { return v; }
  /* Single Expression */
  if (v->count == 1) { return lval_take(v, 0); }


  /* Ensure First Element is a function after evaluation */
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_FUN) {
    lval* err = lval_err(
        "S-Expression starts with incorrect type. "
        "Got %s, Expected %s.",
        ltype_name(f->type), ltype_name(LVAL_FUN));
    lval_del(f); lval_del(v);
    return err;
  }
  /* If so call function to get result */
  lval* result = lval_call(e, f, v);
  lval_del(f);
  return result;
}

lval* lval_eval(lenv* e, lval* v) {
  if (v->type == LVAL_SYM) {
    lval* x = lenv_get(e, v);
    lval_del(v);
    return x;
  }
  /* Evaluate Sexpressions */
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
  /* Functions */
  /* All other lval types remain the same */
  return v;
}

/* Reading */

lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    lval_num(x) : lval_err("Invalid Number");
}

lval* lval_read_str(mpc_ast_t *t) {
  /* Cut off the final quote character */
  t->contents[strlen(t->contents)-1] = '\0';
  /* Copy the string missing out the first quote character */
  char *unescaped = malloc(strlen(t->contents+1)+1);
  strcpy(unescaped, t->contents+1);
  /* Pass through the unespaced function */
  unescaped = mpcf_unescape(unescaped);
  /* Construct a new lval using the string */
  lval *str = lval_str(unescaped);
  /* Free the string and return */
  free(unescaped);
  return str;
}
lval* lval_read(mpc_ast_t* t) {

  /* If Symbol or Number return conversion to that type */
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "string")) { return lval_read_str(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
  

  /* If root (>) or sexpr then create empty list */
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strcmp(t->tag, "sexpr"))  { x = lval_sexpr(); }
  if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

  /* Fill this list with any valid expression contained within */
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
    if (strstr(t->tag, "comment")) { continue; } 
    x = lval_add(x, lval_read(t->children[i]));
  }
  
  return x;
}



lval *lval_init(lval *a) {
  lval *x = lval_qexpr();
  int i = 0;
  while ( i < a->cell[0]->count-1 ) {
    x = lval_add(x, a->cell[0]->cell[i]);
    i++;
  }
  free(a);
  return x;
}
lval* lval_cons(lval* x, lval* y) {
  if ( y->type == LVAL_QEXPR ) {
    int i = 0;
    while ( i < y->count ) {
      x = lval_add(x, y->cell[i]);
      i++;
    }
  } else {
    x = lval_add(x, y);
  }
  return x;
}

lval *lval_nth(lval *a, int n) {
  lval *x = malloc(sizeof(lval*));
  x = a->cell[n];
  free(a);
  return x;
}