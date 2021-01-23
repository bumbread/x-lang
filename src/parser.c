
struct t_ast_node_ typedef t_ast_node;
enum {
  NODE_NONE,
  NODE_TYPE,
  NODE_EXPR,
  NODE_STMT,
  NODE_DECL,
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
  TYPE_NONE,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_POINTER,
  TYPE_FUNC
} typedef t_typespec_name;

struct {
  t_typespec_name type;
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
} typedef t_decl_type;

struct {
  //t_decl_type type;
  //t_ast_node *type_name;
  t_intern const *name;
  t_ast_node *value;
} typedef t_ast_decl;

enum {
  STMT_NONE,
  STMT_IF,
  STMT_WHILE,
  STMT_BREAK,
  STMT_CONTINUE,
  STMT_RETURN,
  STMT_PRINT,
  STMT_COMPOUND,
} typedef t_stmt_type;

struct {
  t_stmt_type type;
  union {
    // if, else if, while
    struct {
      t_ast_node *condition;
      t_ast_node *block;
      t_ast_node *else_stmt;
    };
    // print
    struct {
      t_ast_node *value;
    };
    // compound
    struct {
      t_ast_node *first_stmt;
    };
  };
} typedef t_ast_stmt;

struct t_ast_node_ {
  t_ast_node_type type;
  t_ast_node *next;
  union {
    t_ast_expr expr;
    t_ast_decl decl;
    t_ast_type type_name;
    t_ast_stmt stmt;
  };
};

static t_arena ast_arena;
static t_intern const *keyword_if;
static t_intern const *keyword_else;
static t_intern const *keyword_while;

static t_intern const *keyword_break;
static t_intern const *keyword_continue;
static t_intern const *keyword_return;
static t_intern const *keyword_print;

static t_intern const *keyword_and;
static t_intern const *keyword_or;


static t_intern const *keyword_var;
static t_intern const *keyword_byte;
static t_intern const *keyword_int;
static t_intern const *keyword_float;
static t_intern const *keyword_string;

static void parser_init_memory(ptr buffer_size, void *buffer) {
  arena_init(&ast_arena, buffer_size, buffer);
  keyword_if       = intern_cstring("if");
  keyword_else     = intern_cstring("else");
  keyword_while    = intern_cstring("while");
  
  keyword_break    = intern_cstring("break");
  keyword_continue = intern_cstring("continue");
  keyword_return   = intern_cstring("return");
  keyword_print    = intern_cstring("print");
  
  keyword_and      = intern_cstring("and");
  keyword_or       = intern_cstring("or");
  
  keyword_var      = intern_cstring("var");
#if 0
  keyword_byte     = intern_cstring("byte");
  keyword_int      = intern_cstring("int");
  keyword_float    = intern_cstring("float");
  keyword_string   = intern_cstring("string");
#endif
}

static t_ast_node *alloc_ast_node(void) {
  return arena_alloc(&ast_arena, sizeof(t_ast_node), 8);
}

static inline bool token_is(t_lexstate *state, t_token_kind kind) {
  return state->last_token.kind == kind;
}

static inline bool token_match(t_lexstate *state, t_token_kind kind) {
  if(state->last_token.kind == kind) {
    lex_next_token(state);
    return true;
  }
  return false;
}

static inline bool token_expect(t_lexstate *state, t_token_kind kind) {
  if(state->last_token.kind == kind) {
    lex_next_token(state);
    return true;
  }
  set_errorf("error: expected token %s, got %s", 
             get_token_kind_name(kind), 
             get_token_string(&state->last_token));
  return false;
}


static inline bool token_is_identifier(t_lexstate *state, t_intern const *str) {
  if(state->last_token.kind == TOKEN_IDN) {
    return state->last_token.str_value == str;
  }
  return false;
}

static inline bool token_match_identifier(t_lexstate *state, t_intern const *str) {
  if(state->last_token.kind == TOKEN_IDN && state->last_token.str_value == str) {
    lex_next_token(state);
    return true;
  }
  return false;
}

static inline bool token_expect_identifier(t_lexstate *state, t_intern const *str) {
  if(state->last_token.kind == TOKEN_IDN && state->last_token.str_value == str) {
    lex_next_token(state);
    return true;
  }
  set_errorf("expected keyword %s, got %s", state->last_token.str_value->str, str->str);
  return false;
}

//
// expr0 = val | '(' expr ')'
// expr1 = expr0 | -expr1
// expr2 = expr1 [('*'|'/') expr1 ...]
// expr3 = expr2 [('+'|'-') expr2 ...]
// expr  = expr3 [('and', 'or') expr3 ...]
//

static t_ast_node *parse_expr(t_lexstate *state);

static t_ast_node *parse_expr1(t_lexstate *state) {
  if(token_match(state, '(')) {
    t_ast_node *expr = parse_expr(state);
    token_expect(state, ')');
    return expr;
  }
  else if(state->last_token.kind != TOKEN_EOF) {
    t_ast_node *node = alloc_ast_node();
    node->type = NODE_EXPR;
    node->expr.type = EXPR_VALUE;
    node->expr.value = state->last_token;
    lex_next_token(state);
    return node;
  }
  
  return null;
}

static t_ast_node *parse_expr2(t_lexstate *state) {
  t_token op_token = state->last_token;
  if(token_match(state, '-')) {
    t_ast_node *node = alloc_ast_node();
    node->type = NODE_EXPR;
    node->expr.type = EXPR_UNARY;
    node->expr.unary_operation = op_token;
    node->expr.unary_operand = parse_expr2(state);
    return node;
  }
  else {
    return parse_expr1(state);
  }
}

static t_ast_node *parse_expr3(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr2(state);
  if(null != operand_left) {
    while(token_is(state, '*') || token_is(state, '/')) {
      t_token op_token = state->last_token;
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr2(state);
      
      t_ast_node *node = alloc_ast_node();
      node->type = NODE_EXPR;
      node->expr.type = EXPR_BINARY;
      node->expr.binary_operation = op_token;
      node->expr.binary_operand_left = operand_left;
      node->expr.binary_operand_right = operand_right;
      operand_left = node;
    }
  }
  return operand_left;
}

static t_ast_node *parse_expr4(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr3(state);
  if(null != operand_left) {
    while(token_is(state, '+') || token_is(state, '-')) {
      t_token op_token = state->last_token;
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr3(state);
      
      t_ast_node *node = alloc_ast_node();
      node->type = NODE_EXPR;
      node->expr.type = EXPR_BINARY;
      node->expr.binary_operation = op_token;
      node->expr.binary_operand_left = operand_left;
      node->expr.binary_operand_right = operand_right;
      operand_left = node;
    }
  }
  return operand_left;
}

static t_ast_node *parse_expr5(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr3(state);
  if(null != operand_left) {
    while(token_is(state, '+') || token_is(state, '-')) {
      t_token op_token = state->last_token;
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr3(state);
      
      t_ast_node *node = alloc_ast_node();
      node->type = NODE_EXPR;
      node->expr.type = EXPR_BINARY;
      node->expr.binary_operation = op_token;
      node->expr.binary_operand_left = operand_left;
      node->expr.binary_operand_right = operand_right;
      operand_left = node;
    }
  }
  return operand_left;
}

static t_ast_node *parse_expr6(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr5(state);
  if(null != operand_left) {
    while(token_is(state, TOKEN_CMP_EQ)
          || token_is(state, TOKEN_CMP_NEQ)
          || token_is(state, TOKEN_CMP_GEQ)
          || token_is(state, TOKEN_CMP_LEQ)
          || token_is(state, '>')
          || token_is(state, '<')) {
      t_token op_token = state->last_token;
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr5(state);
      
      t_ast_node *node = alloc_ast_node();
      node->type = NODE_EXPR;
      node->expr.type = EXPR_BINARY;
      node->expr.binary_operation = op_token;
      node->expr.binary_operand_left = operand_left;
      node->expr.binary_operand_right = operand_right;
      operand_left = node;
    }
  }
  return operand_left;
}

static t_ast_node *parse_expr7(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr6(state);
  if(null != operand_left) {
    while(token_is_identifier(state, keyword_and)) {
      t_token op_token = state->last_token;
      lex_next_token(state);
      
      t_ast_node *operand_right = parse_expr6(state);
      
      t_ast_node *node = alloc_ast_node();
      node->type = NODE_EXPR;
      node->expr.type = EXPR_BINARY;
      node->expr.binary_operation = op_token;
      node->expr.binary_operand_left = operand_left;
      node->expr.binary_operand_right = operand_right;
      operand_left = node;
    }
  }
  return operand_left;
}

static t_ast_node *parse_expr8(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr7(state);
  if(null != operand_left) {
    while(token_is_identifier(state, keyword_or)) {
      t_token op_token = state->last_token;
      lex_next_token(state);
      
      t_ast_node *operand_right = parse_expr7(state);
      
      t_ast_node *node = alloc_ast_node();
      node->type = NODE_EXPR;
      node->expr.type = EXPR_BINARY;
      node->expr.binary_operation = op_token;
      node->expr.binary_operand_left = operand_left;
      node->expr.binary_operand_right = operand_right;
      operand_left = node;
    }
  }
  return operand_left;
}

static t_ast_node *parse_expr(t_lexstate *state) {
  return parse_expr7(state);
}

/* STATEMENTS PARSER */

static t_ast_node *parse_assignment(t_lexstate *state) {
  t_ast_node *lhs = parse_expr(state);
  if(null != lhs) {
    if(token_is(state, '=')) {
      t_token op_token = state->last_token;
      lex_next_token(state);
      
      t_ast_node *rhs = parse_expr(state);
      
      t_ast_node *node = alloc_ast_node();
      node->type = NODE_EXPR;
      node->expr.type = EXPR_BINARY;
      node->expr.binary_operation = op_token;
      node->expr.binary_operand_left = lhs;
      node->expr.binary_operand_right = rhs;
      lhs = node;
    }
  }
  return lhs;
}

static t_ast_node *parse_stmts(t_lexstate *state);
static t_ast_node *parse_stmt(t_lexstate *state);

static t_ast_node *parse_if_stmt(t_lexstate *state) {
  token_expect_identifier(state, keyword_if);
  t_ast_node *node = alloc_ast_node();
  
  t_ast_node *condition = parse_expr(state);
  t_ast_node *block = parse_stmts(state);
  if(token_match_identifier(state, keyword_else)) {
    t_ast_node *else_stmt;
    if(token_is(state, '{')) {
      else_stmt = parse_stmts(state);
    }
    else {
      else_stmt = parse_stmts(state);
      token_expect(state, ';');
    }
    node->stmt.else_stmt = else_stmt;
  }
  
  node->stmt.condition = condition;
  node->stmt.block = block;
  node->type = NODE_STMT;
  node->stmt.type = STMT_IF;
  return node;
}

static t_ast_node *parse_while_stmt(t_lexstate *state) {
  token_expect_identifier(state, keyword_while);
  t_ast_node *condition = parse_expr(state);
  t_ast_node *block = parse_stmts(state);
  
  t_ast_node *node = alloc_ast_node();
  node->type = NODE_STMT;
  node->stmt.type = STMT_WHILE;
  node->stmt.condition = condition;
  node->stmt.block = block;
  return node;
}

static t_ast_node *parse_declaration(t_lexstate *state) {
  token_expect_identifier(state, keyword_var);
  
  t_ast_node *node = alloc_ast_node();
  node->type = NODE_DECL;
  
  t_token name = state->last_token;
  if(token_expect(state, TOKEN_IDN)) {
    node->decl.name = name.str_value;
    if(token_expect(state, '=')) {
      t_ast_node *value = parse_expr(state);
      node->decl.value = value;
    }
  }
  
  token_expect(state, ';');
  return node;
}

static t_ast_node *parse_stmt(t_lexstate *state) {
  t_ast_node *node = null;
  if(token_is_identifier(state, keyword_if)) {
    node = parse_if_stmt(state);
  }
  else if(token_is_identifier(state, keyword_while)) {
    node = parse_while_stmt(state);
  }
  else if(token_is_identifier(state, keyword_var)) {
    node = parse_declaration(state);
  }
  else if(token_match_identifier(state, keyword_return)) {
    node = alloc_ast_node();
    node->type = NODE_STMT;
    node->stmt.type = STMT_RETURN;
    token_expect(state, ';');
  }
  else if(token_match_identifier(state, keyword_break)) {
    node = alloc_ast_node();
    node->type = NODE_STMT;
    node->stmt.type = STMT_BREAK;
    token_expect(state, ';');
  }
  else if(token_match_identifier(state, keyword_continue)) {
    node = alloc_ast_node();
    node->type = NODE_STMT;
    node->stmt.type = STMT_CONTINUE;
    token_expect(state, ';');
  }
  else if(token_match_identifier(state, keyword_print)) {
    node = alloc_ast_node();
    node->type = NODE_STMT;
    node->stmt.type = STMT_PRINT;
    node->stmt.value = parse_expr(state);
    token_expect(state, ';');
  }
  else if(token_match(state, '{')) {
    node = parse_stmts(state);
    token_expect(state, '}');
  }
  else {
    node = parse_assignment(state);
    token_expect(state, ';');
  }
  return node;
}

static t_ast_node *parse_stmts(t_lexstate *state) {
  if(!token_expect(state, '{')) return null;
  
  t_ast_node *block = alloc_ast_node();
  block->type = NODE_STMT;
  block->stmt.type = STMT_COMPOUND;
  block->stmt.first_stmt = null;
  
  t_ast_node *last = null;
  while(true) {
    t_ast_node *node = null;
    
    if(token_match(state, '}')) {
      break;
    }
    else {
      node = parse_stmt(state);
    }
    
    if(last != null) {
      last->next = node;
    } else {
      block->stmt.first_stmt = node;
    }
    
    last = node;
  }
  return block;
}

static bool assert_token_type(t_token *token, t_token_kind kind) {
  if(token->kind != kind) {
    set_errorf("expected %s, found '%s'", get_token_kind_name(kind), get_token_string(token));
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
  
  if(ast_node->type == NODE_EXPR) {
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
                     get_token_kind_name(op), get_token_string(&value));
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
                     get_token_string(&left),
                     get_token_string(&right));
        }
        return token_eof;
      } break;
    }
  }
  else {
    set_errorf("not an expression");
  }
  return token_eof;
}

static void print_at_level(char const *str, int level) {
  for(int i = 0; i < level; i += 1) {
    printf(" ");
  }
  printf("%s", str);
}

static void ast_node_print_lisp(t_ast_node *ast_node, int level) {
  for(int i = 0; i < level; i += 1) {
    printf(" ");
  }
  if(ast_node == null) {
    printf("(nul)");
    return;
  }
  if(ast_node->type == NODE_EXPR) {
    switch(ast_node->expr.type) {
      case EXPR_VALUE: {
        if(ast_node->expr.value.kind == TOKEN_INT) {
          printf(" %llu ", ast_node->expr.value.int_value);
        }
        else if(ast_node->expr.value.kind == TOKEN_IDN) {
          printf(" %s ", ast_node->expr.value.str_value->str);
        }
      } break;
      case EXPR_UNARY: {
        printf("(");
        printf("%s", get_token_string(&ast_node->expr.unary_operation));
        ast_node_print_lisp(ast_node->expr.unary_operand, 0);
        printf(")");
      } break;
      case EXPR_BINARY: {
        printf("(");
        printf("%s", get_token_string(&ast_node->expr.binary_operation));
        ast_node_print_lisp(ast_node->expr.binary_operand_left, 0);
        ast_node_print_lisp(ast_node->expr.binary_operand_right, 0);
        printf(")");
      } break;
      default: set_errorf("undefined operation");
    }
  }
  else if(ast_node->type == NODE_STMT) {
    switch(ast_node->stmt.type) {
      case STMT_IF: {
        printf("(if ");
        ast_node_print_lisp(ast_node->stmt.condition, 0);
        printf("\n");
        ast_node_print_lisp(ast_node->stmt.block, level+1);
        if(ast_node->stmt.else_stmt) {
          printf("\n");
          ast_node_print_lisp(ast_node->stmt.else_stmt, level+1);
        }
        else {
          printf("\n");
          print_at_level("<none>", level+1);
        }
        printf("\n");
        print_at_level(")", level);
      } break;
      case STMT_WHILE: {
        printf("(while ");
        ast_node_print_lisp(ast_node->stmt.condition, 0);
        printf("\n");
        ast_node_print_lisp(ast_node->stmt.block, level+1);
        printf("\n");
        print_at_level(")", level);
      } break;
      case STMT_BREAK: {
        printf("break ");
      } break;
      case STMT_RETURN: {
        printf("return ");
      } break;
      case STMT_CONTINUE: {
        printf("continue ");
      } break;
      case STMT_PRINT: {
        printf("(print ");
        ast_node_print_lisp(ast_node->stmt.value, 0);
        printf(")");
      } break;
      case STMT_COMPOUND: {
        printf("(compound ");
        for(t_ast_node *node = ast_node->stmt.first_stmt;
            node != null;
            node = node->next) {
          printf("\n");
          ast_node_print_lisp(node, level + 1);
        }
        print_at_level(")", level);
      } break;
    }
  }
  else if(ast_node->type == NODE_DECL) {
    printf("(decl ");
    printf("%s", ast_node->decl.name->str);
    if(null != ast_node->decl.value) {
      ast_node_print_lisp(ast_node->decl.value, 0);
    }
    print_at_level(")", level);
  }
}

