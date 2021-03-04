
static t_ast_list *alloc_list(void) {
  t_ast_list *result = global_alloc(sizeof(t_ast_list));
  result->first = null;
  result->last = null;
  return result;
}

static t_ast_list_link *alloc_list_link(void) {
  t_ast_list_link *result = global_alloc(sizeof(t_ast_list_link));
  result->next = null;
  result->prev = null;
  return result;
}

static inline bool unexpected_last_token(t_lexstate *state) {
  push_errorf("%s(%d, %d): unexpected token %s.",
              state->filename,
              state->last_token.loc.line, state->last_token.loc.offset,
              get_token_string(&state->last_token));
  return false;
}

static inline void parse_error(char const *string, t_location loc) {
  push_errorf("%s(%d, %d): %s",
              loc.filename,
              loc.line, loc.offset,
              string);
}

static void node_list_push(t_ast_list *list, t_ast_list_link *to_attach) {
  to_attach->next = null;
  to_attach->prev = list->last;
  if(list->last != null) {
    list->last->next = to_attach;
  }
  else {
    list->first = to_attach;
  }
  list->last = to_attach;
}

static void load_expr_node_data(t_ast_node *node, t_token *tok) {
  node->cat = AST_expr_node;
  switch(tok->kind) {
    case TOKEN_int: {
      node->expr.cat = EXPR_int_value;
      node->expr.ivalue = tok->int_value;
      node->expr.type = type_int;
    } break;
    case TOKEN_flt: {
      node->expr.cat = EXPR_float_value;
      node->expr.fvalue = tok->flt_value;
      node->expr.type = type_float;
    } break;
    case TOKEN_str: {
      node->expr.cat = EXPR_string_value;
      node->expr.svalue = tok->str_value;
      node->expr.type = type_string;
    } break;
    case TOKEN_idn: {
      node->expr.cat = EXPR_variable;
      node->expr.var_name = tok->str_value;
      node->expr.type = null; // to be derived on the typecheck stage
    } break;
    default:
    assert(false);
  }
  
}

//--------------------||
// EXPRESSION PARSING ||
//--------------------||

static t_ast_node *parse_expr(t_lexstate *state);

static t_ast_node *parse_expr_list(t_lexstate *state,
                                   t_token_kind seq_separator,
                                   t_token_kind seq_terminator) {
  t_ast_node *node = alloc_node();
  node->cat = AST_list_node;
  if(token_is_kind(state, seq_terminator)) {
    return node;
  }
  while(true) {
    t_ast_node *value = parse_expr(state);
    assert(value);
    if(value == null) {
      return node;
    }
    t_ast_list_link *link = alloc_list_link();
    link->p = value;
    node_list_push(&node->list, link);
    if(!token_match_kind(state, seq_separator)) {
      break;
    }
  }
  return node;
}

static t_ast_node *parse_expr1(t_lexstate *state) {
  t_ast_node *node;
  if(token_match_kind(state, '(')) {
    node = parse_expr(state);
    token_expect_kind(state, ')');
  }
  else if(state->last_token.kind == TOKEN_str
          || state->last_token.kind == TOKEN_idn
          || state->last_token.kind == TOKEN_int
          || state->last_token.kind == TOKEN_flt) {
    node = alloc_node();
    load_expr_node_data(node, &state->last_token);
    lex_next_token(state);
  }
  else {
    unexpected_last_token(state);
    return null;
  }
  
  if(token_match_kind(state, '(')) {
    
    t_ast_node *parameter_list = parse_expr_list(state, ',', ')');
    token_expect_kind(state, ')');
    
    t_ast_node *function_call = alloc_node();
    function_call->cat = AST_expr_node;
    function_call->expr.cat = EXPR_binary_op;
    function_call->expr.op = BINARY_function_call;
    function_call->expr.opr1 = node;
    function_call->expr.opr2 = parameter_list;
    node = function_call;
  }
  return node;
}

static t_ast_node *parse_expr2(t_lexstate *state) {
  // sub
  if(token_match_kind(state, '-')) {
    t_ast_node *expr = parse_expr2(state);
    if(expr == null) return null;
    t_ast_node *node = alloc_node();
    node->cat = AST_expr_node;
    node->expr.cat = EXPR_unary_op;
    node->expr.op = UNARY_sub;
    node->expr.opr1 = expr;
    return node;
  }
  
  // address-of
  else if(token_match_kind(state, '$')) {
    t_ast_node *expr = parse_expr2(state);
    if(expr == null) return null;
    t_ast_node *node = alloc_node();
    node->cat = AST_expr_node;
    node->expr.cat = EXPR_unary_op;
    node->expr.op = UNARY_addr;
    node->expr.opr1 = expr;
    return node;
  }
  
  // deref
  else if(token_match_kind(state, '@')) {
    t_ast_node *expr = parse_expr2(state);
    if(expr == null) return null;
    t_ast_node *node = alloc_node();
    node->cat = AST_expr_node;
    node->expr.cat = EXPR_unary_op;
    node->expr.op = UNARY_deref;
    node->expr.opr1 = expr;
    return node;
  }
  else {
    t_ast_node *result = parse_expr1(state);
    if(result == null) {
      return null;
    }
    
    while(true) {
      if(token_match_kind(state, '[')) {
        t_ast_node *first_index = null;
        if(!token_is_kind(state, ':')) {
          first_index = parse_expr(state);
          if(first_index == null) {
            return null;
          }
        }
        t_ast_node *node = alloc_node();
        
        if(token_match_kind(state, ':')) {
          node->cat = AST_expr_node; //slicing operator
          node->expr.cat = EXPR_ternary_op;
          node->expr.op = TERNARY_slice;
          node->expr.opr1 = result;
          node->expr.opr2 = first_index;
          
          t_ast_node *second_index = null;
          if(!token_is_kind(state, ']')) {
            second_index = parse_expr(state);
            if(second_index == null) {
              return null;
            }
          }
          node->expr.opr3 = second_index;
          token_expect_kind(state, ']');
        }
        
        else {
          node->cat = AST_expr_node; // array access operator
          node->expr.cat = EXPR_binary_op;
          node->expr.op = BINARY_subscript;
          node->expr.opr1 = result;
          node->expr.opr2 = first_index;
          token_expect_kind(state, ']');
        }
        result = node;
      }
      else break;
    }
    return result;
  }
}

static t_ast_node *parse_expr3(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr2(state);
  if(null == operand_left) return null;
  while(true) {
    
    if(token_is_kind(state, '*')) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr2(state);
      if(operand_right == null) return null;
      t_ast_node *node = alloc_node();
      node->cat = AST_expr_node;
      node->expr.cat = EXPR_binary_op;
      node->expr.op = BINARY_mul;
      node->expr.opr1 = operand_left;
      node->expr.opr2 = operand_right;
      operand_left = node;
    }
    
    else if(token_is_kind(state, '/')) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr2(state);
      if(operand_right == null) return null;
      t_ast_node *node = alloc_node();
      node->cat = AST_expr_node;
      node->expr.cat = EXPR_binary_op;
      node->expr.op = BINARY_div;
      node->expr.opr1 = operand_left;
      node->expr.opr2 = operand_right;
      operand_left = node;
    }
    else break;
  }
  return operand_left;
}

static t_ast_node *parse_expr4(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr3(state);
  if(null != operand_left) {
    while(true) {
      
      if(token_is_kind(state, '+')) {
        lex_next_token(state);
        t_ast_node *operand_right = parse_expr3(state);
        if(operand_right == null) return null;
        t_ast_node *node = alloc_node();
        node->cat = AST_expr_node;
        node->expr.cat = EXPR_binary_op;
        node->expr.op = BINARY_add;
        node->expr.opr1 = operand_left;
        node->expr.opr2 = operand_right;
        operand_left = node;
      }
      
      else if(token_is_kind(state, '-')) {
        lex_next_token(state);
        t_ast_node *operand_right = parse_expr3(state);
        if(operand_right == null) return null;
        t_ast_node *node = alloc_node();
        node->cat = AST_expr_node;
        node->expr.cat = EXPR_binary_op;
        node->expr.op = BINARY_sub;
        node->expr.opr1 = operand_left;
        node->expr.opr2 = operand_right;
        operand_left = node;
      }
      else break;
    }
  }
  return operand_left;
}

static t_ast_node *parse_expr5(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr4(state);
  if(null == operand_left) return null;
  while(true) {
    
    if(token_is_kind(state, '<')) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr4(state);
      if(operand_right == null) return null;
      t_ast_node *node = alloc_node();
      node->cat = AST_expr_node;
      node->expr.cat = EXPR_binary_op;
      node->expr.op = BINARY_less;
      node->expr.opr1 = operand_left;
      node->expr.opr2 = operand_right;
      operand_left = node;
    }
    
    else if(token_is_kind(state, '>')) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr4(state);
      if(operand_right == null) return null;
      t_ast_node *node = alloc_node();
      node->cat = AST_expr_node;
      node->expr.cat = EXPR_binary_op;
      node->expr.op = BINARY_greater;
      node->expr.opr1 = operand_left;
      node->expr.opr2 = operand_right;
      operand_left = node;
    }
    
    else if(token_is_kind(state, TOKEN_cmp_leq)) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr4(state);
      if(operand_right == null) return null;
      t_ast_node *node = alloc_node();
      node->cat = AST_expr_node;
      node->expr.cat = EXPR_binary_op;
      node->expr.op = BINARY_leq;
      node->expr.opr1 = operand_left;
      node->expr.opr2 = operand_right;
      operand_left = node;
    }
    
    else if(token_is_kind(state, TOKEN_cmp_geq)) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr4(state);
      if(operand_right == null) return null;
      t_ast_node *node = alloc_node();
      node->cat = AST_expr_node;
      node->expr.cat = EXPR_binary_op;
      node->expr.op = BINARY_geq;
      node->expr.opr1 = operand_left;
      node->expr.opr2 = operand_right;
      operand_left = node;
    }
    
    else if(token_is_kind(state, TOKEN_cmp_eq)) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr4(state);
      if(operand_right == null) return null;
      t_ast_node *node = alloc_node();
      node->cat = AST_expr_node;
      node->expr.cat = EXPR_binary_op;
      node->expr.op = BINARY_eq;
      node->expr.opr1 = operand_left;
      node->expr.opr2 = operand_right;
      operand_left = node;
    }
    
    else if(token_is_kind(state, TOKEN_cmp_neq)) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr4(state);
      if(operand_right == null) return null;
      t_ast_node *node = alloc_node();
      node->cat = AST_expr_node;
      node->expr.cat = EXPR_binary_op;
      node->expr.op = BINARY_neq;
      node->expr.opr1 = operand_left;
      node->expr.opr2 = operand_right;
      operand_left = node;
    }
    else break;
  }
  return operand_left;
}

static t_ast_node *parse_expr6(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr5(state);
  if(null == operand_left) return null;
  while(token_is_identifier(state, keyword_and)) {
    lex_next_token(state);
    t_ast_node *operand_right = parse_expr5(state);
    if(operand_right == null) return null;
    t_ast_node *node = alloc_node();
    node->cat = AST_expr_node;
    node->expr.cat = EXPR_binary_op;
    node->expr.op = BINARY_and;
    node->expr.opr1 = operand_left;
    node->expr.opr2 = operand_right;
    operand_left = node;
  }
  return operand_left;
}

static t_ast_node *parse_expr7(t_lexstate *state) {
  t_ast_node *operand_left = parse_expr6(state);
  if(null == operand_left) return null;
  while(token_is_identifier(state, keyword_or)) {
    lex_next_token(state);
    t_ast_node *operand_right = parse_expr6(state);
    if(operand_right == null) return null;
    t_ast_node *node = alloc_node();
    node->cat = AST_expr_node;
    node->expr.cat = EXPR_binary_op;
    node->expr.op = BINARY_or;
    node->expr.opr1 = operand_left;
    node->expr.opr2 = operand_right;
    operand_left = node;
  }
  return operand_left;
}

static t_ast_node *parse_expr(t_lexstate *state) {
  return parse_expr7(state);
}

/* STATEMENTS PARSER */


static t_ast_node *parse_assignment(t_lexstate *state) {
  t_ast_node *assign_node = parse_expr7(state);
  if(null != assign_node) {
    
    if(token_is_kind(state, TOKEN_add_ass)) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr7(state);
      t_ast_node *node = alloc_node();
      node->cat = AST_stmt_node;
      node->stmt.cat = STMT_assignment;
      node->stmt.ass_op = BINARY_add_ass;
      node->stmt.lvalue = assign_node;
      node->stmt.rvalue = operand_right;
      assign_node = node;
    }
    
    else if(token_is_kind(state, TOKEN_sub_ass)) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr7(state);
      t_ast_node *node = alloc_node();
      node->cat = AST_stmt_node;
      node->stmt.cat = STMT_assignment;
      node->stmt.ass_op = BINARY_sub_ass;
      node->stmt.lvalue = assign_node;
      node->stmt.rvalue = operand_right;
      assign_node = node;
    }
    
    else if(token_is_kind(state, TOKEN_mul_ass)) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr7(state);
      t_ast_node *node = alloc_node();
      node->cat = AST_stmt_node;
      node->stmt.cat = STMT_assignment;
      node->stmt.ass_op = BINARY_mul_ass;
      node->stmt.lvalue = assign_node;
      node->stmt.rvalue = operand_right;
      assign_node = node;
    }
    
    else if(token_is_kind(state, TOKEN_div_ass)) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr7(state);
      t_ast_node *node = alloc_node();
      node->cat = AST_stmt_node;
      node->stmt.cat = STMT_assignment;
      node->stmt.ass_op = BINARY_div_ass;
      node->stmt.lvalue = assign_node;
      node->stmt.rvalue = operand_right;
      assign_node = node;
    }
    
    else if(token_is_kind(state, '=')) {
      lex_next_token(state);
      t_ast_node *operand_right = parse_expr7(state);
      t_ast_node *node = alloc_node();
      node->cat = AST_stmt_node;
      node->stmt.cat = STMT_assignment;
      node->stmt.ass_op = BINARY_ass;
      node->stmt.lvalue = assign_node;
      node->stmt.rvalue = operand_right;
      assign_node = node;
    }
  }
  return assign_node;
}


static t_ast_node *parse_stmts(t_lexstate *state);
static t_ast_node *parse_stmt(t_lexstate *state);

static t_ast_node *parse_if_stmt(t_lexstate *state) {
  token_expect_identifier(state, keyword_if);
  t_ast_node *node = alloc_node();
  t_ast_node *condition = parse_expr(state);
  t_ast_node *block;
  if(token_is_kind(state, ';')) {
    block = null;
  }
  else {
    block = parse_stmts(state);
  }
  if(token_match_identifier(state, keyword_else)) {
    t_ast_node *else_stmt;
    if(token_is_kind(state, '{')) {
      else_stmt = parse_stmts(state);
    }
    else if(token_match_kind(state, ';')) {
      else_stmt = null;
    }
    else {
      else_stmt = parse_stmt(state);
      token_expect_kind(state, ';');
    }
    node->stmt.if_false_block = else_stmt;
  }
  
  node->stmt.if_condition = condition;
  node->stmt.if_true_block = block;
  node->stmt.cat = STMT_if;
  node->cat = AST_stmt_node;
  return node;
}

static t_ast_node *parse_while_stmt(t_lexstate *state) {
  token_expect_identifier(state, keyword_while);
  t_ast_node *condition = parse_expr(state);
  t_ast_node *block;
  if(token_is_kind(state, ';')) {
    block = null;
  }
  else {
    block = parse_stmts(state);
  }
  t_ast_node *node = alloc_node();
  node->cat = AST_stmt_node;
  node->stmt.cat = STMT_while;
  node->stmt.while_condition = condition;
  node->stmt.while_block = block;
  return node;
}

static t_ast_node *parse_type(t_lexstate *state) {
  t_ast_node *node = alloc_node();
  node->cat = AST_type_node;
  
  t_token primitive = state->last_token;
  if(token_expect_kind(state, TOKEN_idn)) {
    node->type.cat = TYPE_alias;
    node->type.name = primitive.str_value;
  }
  else return null;
  
  while(true) {
    
    if(token_match_kind(state, '$')) {
      t_ast_node *pnode = alloc_node();
      pnode->cat = AST_type_node;
      pnode->type.cat = TYPE_pointer;
      pnode->type.base_type = node;
      node = pnode;
    }
    else if(token_match_kind(state, '[')) {
      if(!token_expect_kind(state, ']')) {
        printf("  note: x-lang doesn't have constant array types.");
        return null;
      }
      t_ast_node *pnode = alloc_node();
      pnode->cat = AST_type_node;
      pnode->type.cat = TYPE_slice;
      pnode->type.base_type = node;
      node = pnode;
    }
    else if(token_match_kind(state, TOKEN_left_arrow)) {
      if(!token_expect_kind(state, '(')) {
        return null;
      }
      t_ast_node *decls = alloc_node();
      decls->cat = AST_list_node;
      decls->list.first = null;
      decls->list.last = null;
      t_ast_node *function_node = alloc_node();
      function_node->cat = AST_type_node;
      function_node->type.cat = TYPE_function;
      function_node->type.parameters = decls;
      function_node->type.return_type = node;
      node = function_node;
      while(true) {
        if(token_is_kind(state, ')')) {
          break;
        }
        t_ast_node *parameter_type = parse_type(state);
        assert(parameter_type->cat == AST_type_node);
        if(parameter_type == null) {
          parse_error("no type in function declarator", state->loc);
        }
        t_intern const *opt_param_name = null;
        t_token parameter_name = state->last_token;
        if(token_match_kind(state, TOKEN_idn)) {
          opt_param_name = parameter_name.str_value;
        }
        t_ast_node *param_decl = alloc_node();
        param_decl->cat = AST_stmt_node;
        param_decl->stmt.cat = STMT_declaration;
        param_decl->stmt.decl_type = parameter_type;
        param_decl->stmt.decl_name = opt_param_name;
        // TODO(bumbread): function default parameters ?
        param_decl->stmt.decl_value = null;
        
        t_ast_list_link *param_link = alloc_list_link();
        param_link->p = param_decl;
        node_list_push(&decls->list, param_link);
        
        if(!token_match_kind(state, ';')) {
          break;
        }
      }
      if(!token_expect_kind(state, ')')) {
        return null;
      }
    }
    else {
      break;
    }
  }
  
  return node;
}

static t_ast_node *parse_declaration(t_lexstate *state) {
  if(token_expect_kind(state, ':')) {
    t_ast_node *node = alloc_node();
    node->cat = AST_stmt_node;
    node->stmt.cat = STMT_declaration;
    node->stmt.decl_type = parse_type(state);
    
    t_token name = state->last_token;
    if(token_expect_kind(state, TOKEN_idn)) {
      if(token_is_keyword(&name)) {
        push_errorf("keywords are not allowed as variable name", name.loc);
      }
      node->stmt.decl_name = name.str_value;
    }
    
    if(token_match_kind(state, '=')) {
      node->stmt.decl_value = parse_expr(state);
      token_expect_kind(state, ';');
    }
    else if(token_is_kind(state, '{')) {
      node->stmt.decl_value = parse_stmts(state);
    }
    else {
      if(!token_expect_kind(state, ';')) {
        return null;
      }
    }
    
    assert(node->stmt.decl_type != null);
    t_ast_node *decl_type = node->stmt.decl_type;
    assert(decl_type->cat == AST_type_node);
    if(decl_type->type.cat == TYPE_function) {
      t_ast_list_link *func_link = alloc_list_link();
      func_link->p = node;
      node_list_push(&all_procs, func_link);
    }
    return node;
  }
  return null;
}

static t_ast_node *parse_stmt(t_lexstate *state) {
  t_ast_node *node = null;
  if(token_is_identifier(state, keyword_if)) {
    node = parse_if_stmt(state);
  }
  else if(token_is_identifier(state, keyword_while)) {
    node = parse_while_stmt(state);
  }
  else if(token_is_kind(state, ':')) {
    node = parse_declaration(state);
  }
  else if(token_match_identifier(state, keyword_return)) {
    node = alloc_node();
    node->cat = AST_stmt_node;
    node->stmt.cat = STMT_return;
    if(!token_is_kind(state, ';')) {
      node->stmt.stmt_value = parse_expr(state);
    }
    else {
      node->stmt.stmt_value = null;
    }
    token_expect_kind(state, ';');
  }
  else if(token_match_identifier(state, keyword_break)) {
    node = alloc_node();
    node->cat = AST_stmt_node;
    node->stmt.cat = STMT_break;
    token_expect_kind(state, ';');
  }
  else if(token_match_identifier(state, keyword_continue)) {
    node = alloc_node();
    node->cat = AST_stmt_node;
    node->stmt.cat = STMT_continue;
    token_expect_kind(state, ';');
  }
  else if(token_match_identifier(state, keyword_print)) {
    node = alloc_node();
    node->cat = AST_stmt_node;
    node->stmt.cat = STMT_print;
    node->stmt.stmt_value = parse_expr(state);
    token_expect_kind(state, ';');
  }
  else if(token_is_kind(state, '{')) {
    node = parse_stmts(state);
  }
  else if(token_match_kind(state, ';')) {
    return null;
  }
  else {
    if(state->last_token.kind == TOKEN_eof) {
      node = null;
    }
    else {
      node = parse_assignment(state);
      token_expect_kind(state, ';');
    }
  }
  assert(node != null);
  return node;
}

static t_ast_node *parse_stmts(t_lexstate *state) {
  if(!token_expect_kind(state, '{')) return null;
  
  t_ast_node *block = alloc_node();
  block->cat = AST_stmt_node;
  block->stmt.cat = STMT_block;
  while(true) {
    if(token_match_kind(state, '}')) {
      break;
    }
    else if(state->last_token.kind == TOKEN_eof) {
      unexpected_last_token(state);
      break;
    }
    else {
      t_ast_node *node = parse_stmt(state);
      if(node != null) {
        t_ast_list_link *link = alloc_list_link();
        link->p = node;
        node_list_push(&block->stmt.statements, link);
      }
    }
  }
  return block;
}

static t_ast_node *parse_global_scope(t_lexstate *state) {
  t_ast_node *block = alloc_node();
  block->cat = AST_stmt_node;
  block->stmt.cat = STMT_block;
  while(true) {
    if(token_match_kind(state, TOKEN_eof)) {
      break;
    }
    else {
      t_ast_node *node = parse_declaration(state);
      if(node == null) {
        parse_error("unable to parse declaration", state->loc);
        return block;
      }
      t_ast_list_link *link = alloc_list_link();
      link->p = node;
      node_list_push(&block->stmt.statements, link);
    }
  }
  return block;
}

// debug functions
static t_ast_node *parse_ast_node_expr_level(char const *expr) {
  t_lexstate state;
  lex_init(&state, "@test", expr);
  lex_next_token(&state);
  t_ast_node *code = parse_expr(&state);
  print_error_buffer();
  return code;
}
static t_ast_node *parse_ast_node_stmt_level(char const *stmt) {
  t_lexstate state;
  lex_init(&state, "@test", stmt);
  lex_next_token(&state);
  t_ast_node *code = parse_stmt(&state);
  print_error_buffer();
  return code;
}
static t_ast_node *parse_ast_node_global_level(char const *code_str) {
  t_lexstate state;
  lex_init(&state, "@test", code_str);
  lex_next_token(&state);
  t_ast_node *code = parse_global_scope(&state);
  print_error_buffer();
  return code;
}

static char const *get_operator_string(t_operator_cat cat) {
  switch(cat) {
    case UNARY_add: return "+";
    case UNARY_sub: return "-";
    case UNARY_addr: return "$";
    case UNARY_deref: return "@";
    case BINARY_ass: return "=";
    case BINARY_add: return "+";
    case BINARY_sub: return "-";
    case BINARY_mul: return "*";
    case BINARY_div: return "/";
    case BINARY_add_ass: return "+=";
    case BINARY_sub_ass: return "-=";
    case BINARY_div_ass: return "/=";
    case BINARY_mul_ass: return "*=";
    case BINARY_and: return "and";
    case BINARY_or: return "or";
    case BINARY_less: return "<";
    case BINARY_greater: return ">";
    case BINARY_leq: return "<=";
    case BINARY_geq: return ">=";
    case BINARY_eq: return "==";
    case BINARY_neq: return "!=";
    case BINARY_function_call: return "call";
    case BINARY_subscript: return "subscript";
    case TERNARY_slice: return "slice";
  }
  return "<invalid unary operation>";
}

static bool op_is_unary(t_operator_cat cat) {
  return cat > UNARY_FIRST_OPERATOR && cat < BINARY_FIRST_OPERATOR;
}

static bool op_is_binary(t_operator_cat cat) {
  return cat > BINARY_FIRST_OPERATOR && cat < TERNARY_FIRST_OPERATOR;
}

static bool op_is_ternary(t_operator_cat cat) {
  return cat > TERNARY_FIRST_OPERATOR && cat < LAST_OPERATOR;
}

static bool op_is_assignment(t_operator_cat cat) {
  return cat > BINARY_FIRST_ASS && cat < BINARY_LAST_ASS;
}
