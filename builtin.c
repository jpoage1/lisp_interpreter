#include "lispy.h"

lval* builtin_lambda(lenv *e, lval *a) {
  /* Check Two arguments, each of which are Q-Expressions */
  LASSERT_NUM("\\", a, 2);
  LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
  LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

  /* Check first Q-Expression contains only Symbols */
  for (int i = 0; i < a->cell[0]->count; i++) {
    LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
        "Cannot define non-symbol. Got %s, Expected %s.",
        ltype_name(a->cell[0]->cell[i]->type),ltype_name(LVAL_SYM));
  }

  /* Pop first two arguments and pass them to lval_lambda */
  lval *formals = lval_pop(a, 0);
  lval *body = lval_pop(a, 0);
  lval_del(a);

  return lval_lambda(formals, body);
}

lval* builtin_list(lenv* e, lval* a) {
  a->type = LVAL_QEXPR;
  return a;
}

lval* builtin_head(lenv* e, lval* a) {
  /* Check Error Conditions */
  LASSERT_NUM("head", a, 1);
  LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("head", a, 0);

  /* Otherwise take first argument */
  lval* v = lval_take(a, 0);

  /* Delete all elements that are not head and return */
  while (v->count > 1) { lval_del(lval_pop(v, 1)); }
  return v;
}

lval* builtin_tail(lenv* e, lval* a) {
  /* Check Error Conditions */
  LASSERT_NUM("tail", a, 1);
  LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("tail", a, 0);

  /* Take first argument */
  lval* v = lval_take(a, 0);

  /* Delete first element and return */
  lval_del(lval_pop(v, 0));
  return v;
}

lval* builtin_eval(lenv* e, lval* a) {
  LASSERT_NUM("eval", a, 1);
  LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);

  lval* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

lval* builtin_join(lenv* e, lval* a) {
  
  for (int i = 0; i < a->count; i++) {
    LASSERT_TYPE("join", a, i, LVAL_QEXPR);
  }

  lval* x = lval_pop(a, 0);

  while (a->count) {
    lval* y = lval_pop(a, 0);
    x = lval_join(x, y);
  }

  lval_del(a);
  return x;
}

lval* builtin_op(lenv* e, lval* a, char* op) {

  /* Ensure all arguments are numbers */
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("Cannot operate on non-number!");
    }
  }

  /* Pop the first element */
  lval* x = lval_pop(a, 0);

  /* if no argumens and sub then peform unary negation */
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->num = -x->num;
  }

  /* While there are still elements remainincg */
  while (a->count > 0) {
    lval* y = lval_pop(a, 0);

    if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) { x->num += y->num; }
    if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) { x->num -= y->num; }
    if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) { x->num *= y->num; }
    if (strcmp(op, "^") == 0 || strcmp(op, "pow") == 0) { x->num *= y->num; }  
    if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0) {
      if (y->num == 0) {
        lval_del(x); lval_del(y);
        x = lval_err("Division By Zero.");
        break;
      }
      x->num /= y->num;
    }
    if (strcmp(op, "%") == 0 || strcmp(op, "rem") == 0) {
      if (y->num == 0) {
        lval_del(x); lval_del(y);
        x = lval_err("Division By Zero.");
        break;
      }
      x->num = fmod(x->num, y->num);
    }

    lval_del(y);
  }

  lval_del(a);
  return x;
}

lval* builtin_add(lenv* e, lval* a) {
  return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
  return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
  return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
  return builtin_op(e, a, "/");
}

lval* builtin_rem(lenv* e, lval* a) {
  return builtin_op(e, a, "%");
}

lval* builtin_pow(lenv* e, lval* a) {
  return builtin_op(e, a, "^");
}

lval* builtin_var(lenv* e, lval* a, char* func) {

  LASSERT_TYPE(func, a, 0, LVAL_QEXPR);
  /* First argument is symbol list */
  lval* syms = a->cell[0];

  /* Ensure all elements of first list ares symbols */
  for (int i = 0; i < syms->count; i++) {
    LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
        "Function '%s' cannot define non-symbol. "
        "Got %s, Expected %s.", func,
        ltype_name(syms->cell[i]->type),
        ltype_name(LVAL_SYM));
  }

  /* Check correct number of symbols and values */
  LASSERT(a, (syms->count == a->count-1),
      "Function '%s' passed too many arguments for symbols. "
      "Got %i, Expected %i.", func,
      syms->count, a->count-1);

  /* Assign copies of values to symbols */
  for ( int i = 0; i < syms->count; i++) {
    /* If 'def' define in globally. If 'put' define in locally */
    if (strcmp(func, "def") == 0) {
      lenv_def(e, syms->cell[i], a->cell[i+1]);
    }
    if (strcmp(func, "=") == 0) {
      lenv_put(e, syms->cell[i], a->cell[i+1]);
    }
  }

  lval_del(a);
  return lval_sexpr();
}

lval *builtin_ord(lenv *e, lval *a, char* op) {
  LASSERT_NUM(op, a, 2);
  LASSERT_TYPE(op, a, 0, LVAL_NUM);
  LASSERT_TYPE(op, a, 1, LVAL_NUM);

  int r;
  if (strcmp(op, ">") == 0) {
    r = (a->cell[0]->num > a->cell[1]->num);
  }
  if (strcmp(op, "<") == 0) {
    r = (a->cell[0]->num < a->cell[1]->num);
  }
  if (strcmp(op, ">=") == 0) {
    r = (a->cell[0]->num >= a->cell[1]->num);
  }
  if (strcmp(op, "<=") == 0) {
    r = (a->cell[0]->num <= a->cell[1]->num);
  }
  lval_del(a);
  return lval_num(r);
}
lval *builtin_cmp(lenv *e, lval *a, char *op) {
  LASSERT_NUM(op, a, 2);
  int r;
  if (strcmp(op, "==") == 0) {
    r = lval_eq(a->cell[0], a->cell[1]);
  }
  if (strcmp(op, "!=") == 0) {
    r = !lval_eq(a->cell[0], a->cell[1]);
  }
  lval_del(a);
  return lval_num(r);
}
lval *builtin_eq(lenv *e, lval *a) {
  return builtin_cmp(e, a, "==");
}
lval *builtin_ne(lenv *e, lval *a) {
  return builtin_cmp(e, a, "!=");
}
lval *builtin_if(lenv *e, lval *a) {
  LASSERT_NUM("if", a, 3);
  LASSERT_TYPE("if", a, 0, LVAL_NUM);
  LASSERT_TYPE("if", a, 1, LVAL_QEXPR);
  LASSERT_TYPE("if", a, 2, LVAL_QEXPR);

  /* Mark Both Expressions as evaluable */
  lval *x;
  a->cell[1]->type = LVAL_SEXPR;
  a->cell[2]->type = LVAL_SEXPR;

  if (a->cell[0]->num) {
    /* If conditions is true evaluate first expression */
    x = lval_eval(e, lval_pop(a, 1));
  } else {
    /* Otherwise evaluate second expression */
    x = lval_eval(e, lval_pop(a, 2));
  }

  /* Delete argument list and return */
  lval_del(a);
  return x;
}

lval *builtin_def(lenv *e, lval *a) {
  return builtin_var(e, a, "def");
}

lval *builtin_put(lenv *e, lval *a) {
  return builtin_var(e, a, "=");
}

lval *builtin_gt(lenv *e, lval *a) {
  return builtin_ord(e, a, ">");
}
lval *builtin_lt(lenv *e, lval *a) {
  return builtin_ord(e, a, "<");
}
lval *builtin_ge(lenv *e, lval *a) {
  return builtin_ord(e, a, ">=");
}
lval *builtin_le(lenv *e, lval *a) {
  return builtin_ord(e, a, "<=");
}
lval *builtin_load(lenv *e, lval *a) {
  LASSERT_NUM("load", a, 1);
  LASSERT_TYPE("load", a, 0, LVAL_STR);

  /* Parse File given by string name */
  mpc_result_t r;
  if (mpc_parse_contents(a->cell[0]->str, Lispy, &r)) {

    /* Read contents */
    lval *expr = lval_read(r.output);
    mpc_ast_delete(r.output);

    /* Evaluate each Expression */
    while (expr->count) {
      lval * x = lval_eval(e, lval_pop(expr, 0));
      /* If Evaluation leads to error print it */
      if (x->type == LVAL_ERR) { lval_println(x); }
      lval_del(x);
    }

    /* Delete expressions and arguments */
    lval_del(expr);
    lval_del(a);

    /* Return empty list */
    return lval_sexpr();
  } else {
    /* Get Parse Error as String */
    char *err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);

    /* Create new error message using it */
    lval *err = lval_err("Could not load Library %s", err_msg);
    free(err_msg);
    lval_del(a);

    /* Cleanup and return error */
    return err;
  }
}
lval *builtin_print(lenv *e, lval *a) {

  /* Print each argument followed by a space */
  for (int i = 0; i < a->count; i++) {
    lval_print(a->cell[i]); putchar(' ');
  }

  /* Print a newline and delete arguments */
  putchar('\n');
  lval_del(a);

  return lval_sexpr();
}
lval *builtin_error(lenv *e, lval *a) {
  LASSERT_NUM("error", a, 1);
  LASSERT_TYPE("error", a, 0, LVAL_STR);

  /* Construct Error from first argument */
  lval *err = lval_err(a->cell[0]->str);

  /* Delete arguments and return */
  lval_del(a);
  return err;
}
void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
  lval* k = lval_sym(name);
  lval* v = lval_builtin(func);
  lenv_put(e, k, v);
  lval_del(k); lval_del(v);
}
lval* builtin_exit(lenv* e, lval* a);
lval* builtin_cons(lenv* e, lval* a); 
lval* builtin_init(lenv* e, lval* a); 
lval* builtin_len(lenv* e, lval* a); 
lval* builtin_front(lenv* e, lval* a); 
lval* builtin_nth(lenv* e, lval* a); 
lval* builtin_exit(lenv* e, lval* a) {
  printf("Exit was called!\n");
  exit(0);
}

lval *builtin_len(lenv *e, lval *a) {
  LASSERT_NUM("len", a, 1);
  LASSERT_TYPE("len", a, 0, LVAL_QEXPR);
  lval *x = NULL;
  if ( a->cell[0]->type == LVAL_QEXPR ) {
    x = lval_num(a->cell[0]->count);
  }
  free(a);
  return x;
};
lval *builtin_init(lenv *e, lval *a) {
  LASSERT_NUM("init", a, 1);
  LASSERT_TYPE("init", a, 0, LVAL_QEXPR);
  lval *x = lval_init(a);
  return x;
};

lval* builtin_cons(lenv *e, lval* a) {
  
  for (int i = 0; i < a->count; i++) {
    LASSERT(a, a->cell[i]->type == LVAL_QEXPR || a->cell[i]->type == LVAL_NUM,
        "Function 'cons' passed incorrect type.");
  }
  lval *x = NULL;
  if ( a->cell[0]->type == LVAL_NUM ) {
    x = lval_qexpr();
    x = lval_add(x, a->cell[0]);
  } else {
    x = a->cell[0];
  }
  int i = 1;
  while ( i < a->count ) {
    x = lval_cons(x, a->cell[i]);
    i++;
  }
  free(a);
  return x;
}

lval* builtin_front(lenv *e, lval *a) {
  LASSERT_NUM("front", a, 1);
  LASSERT_TYPE("front", a, 0, LVAL_QEXPR);
  LASSERT(a, a->cell[0]->count > 0,
      "Function 'front' passed empty LVAL_QEXPR.");
  lval *x = lval_nth(a->cell[0], 0);
  return x;
}

lval* builtin_nth(lenv *e, lval *a) {
  LASSERT_NUM("nth", a, 2);
  LASSERT_TYPE("nth", a, 0, LVAL_NUM);
  LASSERT_TYPE("nth", a, 1, LVAL_QEXPR);
  LASSERT(a, a->cell[1]->count > 0,
      "Function 'nth' passed empty LVAL_QEXPR.");
  LASSERT(a, a->cell[1]->count > a->cell[0]->num,
      "Function 'nth' arg 2 count '%d' is less than arg 1 '%d'",
      a->cell[1]->count,
      a->cell[0]->num);
  lval *x = lval_nth(a->cell[1], a->cell[0]->num);
  return x;
}