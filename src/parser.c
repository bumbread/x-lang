
static inline bool token_is(t_lexstate *state, t_token_kind kind) {
  return state->last_token.kind == kind;
}

static inline bool token_match(t_lexstate *state, t_token_kind kind) {
  if(state->last_token.kind == kind) {
    state_parse_next_token(state);
    return true;
  }
  return false;
}

static inline bool token_match_val(t_lexstate *state) {
  if(state->last_token.kind >= TOKEN_INT && state->last_token.kind <= TOKEN_STR) {
    state_parse_next_token(state);
    return true;
  }
  return false;
}

enum {
  TYPE_WHAT,
  TYPE_VAL,
  TYPE_EXPR_BIN,
  TYPE_EXPR_UNR,
} typedef t_ast_node_type;

struct t_ast_node_ {
  t_ast_node_type type;
  struct {
    // singular values
    struct {
      t_token value;
    };
    // unary expression
    struct {
      t_token unary_op;
      struct t_ast_node_ *unary_val;
    };
    // binary expression
    struct {
      struct t_ast_node_ *binary_lv;
      t_token binary_op;
      struct t_ast_node_ *binary_rv;
    };
    // compond statement
    struct {
      ptr statement_count;
      struct t_ast_node_ *statements;
    };
  };
} typedef t_ast_node;

static t_pool ast_node_pool;

static void parser_init_pool(ptr buffer_size, void *buffer) {
  pool_init(&ast_node_pool, buffer_size, buffer, sizeof(t_ast_node), 8);
}

//
// expr3 = val | '(' expr0 ')'
// expr2 = expr3 | -expr3
// expr1 = expr2 [('*'|'/') expr2 ...]
// expr0 = expr1 [('+'|'-') expr1 ...]
// expr  = expr2
//

static t_ast_node *parse_expr0(t_lexstate *state);

static t_ast_node *parse_expr3(t_lexstate *state) {
  if(token_match(state, '(')) {
    t_ast_node *expr0 = parse_expr0(state);
    if(false == token_match(state, ')')) {
      set_errorf("fatal: unmatched parenthesis");
    }
    return expr0;
  }
  else {
    t_ast_node *val_node = pool_alloc(&ast_node_pool);
    val_node->value = state->last_token;
    val_node->type = TYPE_VAL;
    state_parse_next_token(state);
    return val_node;
  }
  
  return null;
}

static t_ast_node *parse_expr2(t_lexstate *state) {
  t_token op_token = state->last_token;
  if(token_match(state, '-')) {
    t_ast_node *min_op = pool_alloc(&ast_node_pool);
    min_op->type = TYPE_EXPR_UNR;
    min_op->unary_op = op_token;
    min_op->unary_val = parse_expr3(state);
    return min_op;
  }
  else {
    return parse_expr3(state);
  }
}

static t_ast_node *parse_expr1(t_lexstate *state) {
  t_ast_node *lv = parse_expr2(state);
  while(token_is(state, '*') || token_is(state, '/')) {
    t_token op_token = state->last_token;
    state_parse_next_token(state);
    t_ast_node *rv = parse_expr2(state);
    t_ast_node *mul_op = pool_alloc(&ast_node_pool);
    mul_op->type = TYPE_EXPR_BIN;
    mul_op->binary_lv = lv;
    mul_op->binary_op = op_token;
    mul_op->binary_rv = rv;
    lv = mul_op;
  }
  return lv;
}

static t_ast_node *parse_expr0(t_lexstate *state) {
  t_ast_node *lv = parse_expr1(state);
  while(token_is(state, '+') || token_is(state, '-')) {
    t_token op_token = state->last_token;
    state_parse_next_token(state);
    t_ast_node *rv = parse_expr1(state);
    t_ast_node *add_op = pool_alloc(&ast_node_pool);
    add_op->type = TYPE_EXPR_BIN;
    add_op->binary_lv = lv;
    add_op->binary_op = op_token;
    add_op->binary_rv = rv;
    lv = add_op;
  }
  return lv;
}

static t_ast_node *parse_expr(t_lexstate *state) {
  t_ast_node *expr = parse_expr0(state);
  if(state->last_token.kind != TOKEN_EOF) {
    set_errorf("not an expression!\n");
  }
  return expr;
}

static bool assert_token_type(t_token *token, t_token_kind kind) {
  if(token->kind != kind) {
    if(token->kind < 128) {
      set_errorf("expected token_int, found '%c'", token->kind);
    }
    else {
      set_errorf("expected token_int, found '%s'", get_nonchar_token_kind_name(token->kind));
    }
    return false;
  }
  return true;
}

static i64 ast_node_evaluate(t_ast_node *ast_node) {
  if(ast_node == null) {
    set_errorf("empty expression");
    return 0;
  }
  
  switch(ast_node->type) {
    case TYPE_VAL: {
      if(assert_token_type(&ast_node->value, TOKEN_INT)) {
        return ast_node->value.int_value;
      }
      return 0;
    } break;
    case TYPE_EXPR_BIN: {
      switch(ast_node->binary_op.kind) {
        case '+': return ast_node_evaluate(ast_node->binary_lv) + ast_node_evaluate(ast_node->binary_rv);
        case '-': return ast_node_evaluate(ast_node->binary_lv) - ast_node_evaluate(ast_node->binary_rv);
        case '*': return ast_node_evaluate(ast_node->binary_lv) * ast_node_evaluate(ast_node->binary_rv);
        case '/': {
          i64 bottom = ast_node_evaluate(ast_node->binary_rv);
          if(bottom == 0) {
            set_errorf("error division by zero");
            return 0;
          }
          return ast_node_evaluate(ast_node->binary_lv) / bottom;
        }
      }
    } break;
    case TYPE_EXPR_UNR: {
      switch(ast_node->unary_op.kind) {
        case '-': return -ast_node_evaluate(ast_node->unary_val);
      }
    } break;
  }
  
  set_errorf("undefined operation");
  return 0;
}

static void ast_node_print_lisp(t_ast_node *ast_node) {
  if(ast_node == null) {
    printf("()");
    return;
  }
  switch(ast_node->type) {
    case TYPE_VAL: {
      if(assert_token_type(&ast_node->value, TOKEN_INT)) {
        printf(" %llu ", ast_node->value.int_value);
      }
    } break;
    case TYPE_EXPR_BIN: {
      if(ast_node->binary_op.kind < 128) printf("(%c", ast_node->binary_op.kind);
      else printf("(%s", get_nonchar_token_kind_name(ast_node->binary_op.kind));
      ast_node_print_lisp(ast_node->binary_lv);
      ast_node_print_lisp(ast_node->binary_rv);
      printf(")");
    } break;
    case TYPE_EXPR_UNR: {
      if(ast_node->binary_op.kind < 128) printf("(%c", ast_node->binary_op.kind);
      else printf("(%s", get_nonchar_token_kind_name(ast_node->binary_op.kind));
      ast_node_print_lisp(ast_node->unary_val);
      printf(")");
    } break;
    default: set_errorf("undefined operation");
  }
}

