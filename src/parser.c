
struct t_ast_node_ typedef t_ast_node;
enum {
  TYPE_NONE,
  TYPE_TYPE,
  TYPE_EXPR,
  TYPE_STMT,
  TYPE_DECL,
} typedef t_ast_node_type;

enum {
  EXPR_NONE,
  EXPR_VALUE,
  EXPR_UNARY,
  EXPR_BINARY,
} typedef t_expr_type;

struct {
  t_expr_type type;
  union {
    t_token value;
    struct {
      t_token unary_operation;
      t_ast_node *unary_operand;
    };
    struct {
      t_token binary_operation;
      t_ast_node *binary_operand_left;
      t_ast_node *binary_operand_right;
    };
  };
} typedef t_ast_expr;

enum {
  _TYPE_NONE,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRUCT,
  TYPE_ENUM,
  TYPE_UNION,
  TYPE_POINTER,
  TYPE_ARRAY,
  TYPE_FUNC
} typedef t_type_type;

struct {
  t_type_type type;
  t_intern *name;
  union {
    // pointers, arrays.
    t_ast_node *base_type;
    // structs, unions
    t_ast_node *declaration_list;
    // functions
    struct {
      t_ast_node *return_type;
      t_ast_node *arg_decl_list;
    };
  };
} typedef t_ast_type;

enum {
  DECL_NONE,
  DECL_VAR,
  DECL_TYPE,
  DECL_ALIAS,
  DECL_STATIC,
  DECL_EXTERN,
} typedef t_decl_type;

struct {
  t_decl_type type;
  t_ast_node *decl_type;
  t_intern *name;
  t_ast_node *default_value;
} typedef t_ast_decl;

enum {
  STMT_NONE,
  STMT_IF,
  STMT_WHILE,
  STMT_BREAK,
  STMT_RETURN,
  STMT_YIELD,
} typedef t_stmt_type;

struct {
  t_stmt_type type;
  t_ast_node *next;
  union {
    // if, else if, while
    struct {
      t_ast_node *condition;
      t_ast_node *block;
      t_ast_node *else_stmt;
    };
    // return, yield
    struct {
      t_ast_node *value;
    };
  };
} typedef t_ast_stmt;

struct t_ast_node_ {
  t_ast_node_type type;
  union {
    t_ast_expr expr;
    t_ast_decl decl;
    t_ast_type asttype;
    t_ast_stmt stmt;
  };
};

static t_arena ast_arena;

static void parser_init_memory(ptr buffer_size, void *buffer) {
  arena_init(&ast_arena, buffer_size, buffer);
}

static t_ast_node *allocate_node(void) {
  return arena_alloc(&ast_arena, sizeof(t_ast_node), 8);
}

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

static inline bool token_expect(t_lexstate *state, t_token_kind kind) {
  if(state->last_token.kind == kind) {
    state_parse_next_token(state);
    return true;
  }
  set_errorf("error: expected token %s, got %s", 
             get_token_kind_name(kind), get_token_kind_name(state->last_token.kind));
  return false;
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
    token_expect(state, ')');
    return expr0;
  }
  else {
    t_ast_node *node = allocate_node();
    node->type = TYPE_EXPR;
    node->expr.type = EXPR_VALUE;
    node->expr.value = state->last_token;
    state_parse_next_token(state);
    return node;
  }
  
  return null;
}

static t_ast_node *parse_expr2(t_lexstate *state) {
  t_token op_token = state->last_token;
  if(token_match(state, '-')) {
    t_ast_node *node = allocate_node();
    node->type = TYPE_EXPR;
    node->expr.type = EXPR_UNARY;
    node->expr.unary_operation = op_token;
    node->expr.unary_operand = parse_expr3(state);
    return node;
  }
  else {
    return parse_expr3(state);
  }
}

static t_ast_node *parse_expr1(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr2(state);
  while(token_is(state, '*') || token_is(state, '/')) {
    t_token op_token = state->last_token;
    state_parse_next_token(state);
    t_ast_node *operand_right = parse_expr2(state);
    
    t_ast_node *node = allocate_node();
    node->type = TYPE_EXPR;
    node->expr.type = EXPR_BINARY;
    node->expr.binary_operation = op_token;
    node->expr.binary_operand_left = operand_left;
    node->expr.binary_operand_right = operand_right;
    operand_left = node;
  }
  return operand_left;
}

static t_ast_node *parse_expr0(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr1(state);
  while(token_is(state, '+') || token_is(state, '-')) {
    t_token op_token = state->last_token;
    state_parse_next_token(state);
    t_ast_node *operand_right = parse_expr1(state);
    
    t_ast_node *node = allocate_node();
    node->type = TYPE_EXPR;
    node->expr.type = EXPR_BINARY;
    node->expr.binary_operation = op_token;
    node->expr.binary_operand_left = operand_left;
    node->expr.binary_operand_right = operand_right;
    operand_left = node;
  }
  return operand_left;
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
    set_errorf("expected %s, found '%s'", get_token_kind_name(kind), get_token_kind_name(token->kind));
    return false;
  }
  return true;
}

static t_token token_eof = {.kind = TOKEN_EOF,};
static t_token ast_node_evaluate(t_ast_node *ast_node) {
  if(ast_node == null) {
    set_errorf("empty expression");
    return token_eof;
  }
  
  if(ast_node->type == TYPE_EXPR) {
    switch(ast_node->expr.type) {
      case EXPR_VALUE: {
        return ast_node->expr.value;
      } break;
      case EXPR_UNARY: {
        t_token value = ast_node_evaluate(ast_node->expr.unary_operand);
        t_token_kind op = ast_node->expr.unary_operation.kind;
        if(value.kind == TOKEN_INT) {
          if(op == '-') {
            value.int_value = -value.int_value;
            return value;
          }
        }
        else {
          set_errorf("operation %s not permitted on token type %s",
                     get_token_kind_name(op), get_token_kind_name(value.kind));
        }
      } break;
      case EXPR_BINARY: {
        t_token_kind op = ast_node->expr.binary_operation.kind;
        t_token result;
        t_token left = ast_node_evaluate(ast_node->expr.binary_operand_left);
        t_token right = ast_node_evaluate(ast_node->expr.binary_operand_right);
        
        if(left.kind == TOKEN_INT && right.kind == TOKEN_INT) {
          result.kind = TOKEN_INT;
          switch(op) {
            case '+': {
              result.int_value = left.int_value + right.int_value;
            } break;
            case '-': {
              result.int_value = left.int_value - right.int_value;
            } break;
            case '*': {
              result.int_value = left.int_value * right.int_value;
            } break;
            case '/': {
              if(right.int_value == 0) set_errorf("error: division by zero");
              else result.int_value = left.int_value / right.int_value;
            } break;
          }
          return result;
        }
        else {
          set_errorf("operation %s not permitted on tokens %s, %s",
                     get_token_kind_name(op), 
                     get_token_kind_name(left.kind),
                     get_token_kind_name(right.kind));
        }
        return token_eof;
      } break;
    }
  }
  
  set_errorf("string is not an expression");
  return token_eof;
}

static void ast_node_print_lisp(t_ast_node *ast_node) {
  if(ast_node == null) {
    printf("()");
    return;
  }
  if(ast_node->type == TYPE_EXPR) {
    switch(ast_node->expr.type) {
      case EXPR_VALUE: {
        if(assert_token_type(&ast_node->expr.value, TOKEN_INT)) {
          printf(" %llu ", ast_node->expr.value.int_value);
        }
      } break;
      case EXPR_UNARY: {
        printf("(");
        printf("%s", get_token_kind_name(ast_node->expr.unary_operation.kind));
        ast_node_print_lisp(ast_node->expr.unary_operand);
        printf(")");
      } break;
      case EXPR_BINARY: {
        printf("(");
        printf("%s", get_token_kind_name(ast_node->expr.binary_operation.kind));
        ast_node_print_lisp(ast_node->expr.binary_operand_left);
        ast_node_print_lisp(ast_node->expr.binary_operand_right);
        printf(")");
      } break;
      default: set_errorf("undefined operation");
    }
  }
  else if(ast_node->type == TYPE_STMT) {
    switch(ast_node->stmt.type) {
      case STMT_IF: {
        printf("(if ");
        ast_node_print_lisp(ast_node->stmt.condition);
        ast_node_print_lisp(ast_node->stmt.block);
        if(ast_node->stmt.else_stmt) {
          ast_node_print_lisp(ast_node->stmt.else_stmt);
        }
        else {
          printf("<none>");
        }
        printf(")");
      } break;
      case STMT_WHILE: {
        printf("(while ");
        ast_node_print_lisp(ast_node->stmt.condition);
        ast_node_print_lisp(ast_node->stmt.block);
        printf(")");
      } break;
      case STMT_BREAK: {
        printf("break ");
      } break;
      case STMT_RETURN: {
        printf("(return ");
        ast_node_print_lisp(ast_node->stmt.value);
        printf(")");;
      } break;
      case STMT_YIELD: {
        printf("(yield ");
        ast_node_print_lisp(ast_node->stmt.value);
        printf(")");
      } break;
    }
  }
}

