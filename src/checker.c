
struct t_ast_stack_list_link_ typedef t_ast_stack_list_link;

struct {
  t_ast_list_link *first;
  t_ast_list_link *last;
} typedef t_ast_stack_list;

struct t_ast_stack_list_link_ {
  t_ast_list_link *next;
  t_ast_list_link *prev;
  t_ast_node *p;
};

static t_ast_stack_list *alloc_stack_list(void) {
  t_ast_stack_list *result = global_alloc(sizeof(t_ast_stack_list));
  result->first = null;
  result->last = null;
  return result;
}

static t_ast_stack_list_link *alloc_stack_list_link(void) {
  t_ast_stack_list_link *result = global_alloc(sizeof(t_ast_stack_list_link));
  result->next = null;
  result->prev = null;
  return result;
}

static void stack_list_push(t_ast_stack_list *list, t_ast_stack_list_link *to_attach) {
  assert(to_attach != null);
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

static void stack_list_push_node(t_ast_stack_list *list, t_ast_node *node) {
  assert(node != null);
  t_ast_stack_list_link *link = alloc_stack_list_link();
  link->p = node;
  stack_list_push(list, link);
}

static void stack_list_push_frame(t_ast_stack_list *list) {
  t_ast_stack_list_link *to_attach = alloc_stack_list_link();
  to_attach->next = null;
  to_attach->prev = list->last;
  to_attach->p = null;
  if(list->last != null) {
    list->last->next = to_attach;
  }
  else {
    list->first = to_attach;
  }
  list->last = to_attach;
}

static void stack_list_pop_frame(t_ast_stack_list *list) {
  t_ast_stack_list_link *search_node = list->last;
  while(search_node->p != null) {
    search_node = search_node->prev;
    if(search_node == null) {
      assert(0 && "unbalanced pushes and pops");
      return;
    }
  }
  t_ast_stack_list_link *new_last = search_node->prev;
  new_last->next = null;
  list->last = new_last;
}


t_ast_node *scope;
t_ast_stack_list decls;

static void check_type(t_ast_node *node) {
  assert(node->cat == AST_type_node);
  if(node->type.cat == TYPE_alias) {
    if(node->type.name == keyword_int) {
    }
    else if(node->type.name == keyword_byte) {
    }
    else if(node->type.name == keyword_bool) {
    }
    else if(node->type.name == keyword_float) {
    }
    else if(node->type.name == keyword_string) {
    }
    else {
      push_errorf("type %s not declared!", node->type.name);
    }
  }
  else if(node->type.cat == TYPE_pointer) {
    assert(node->type.base_type != null);
    check_type(node->type.base_type);
  }
  else if(node->type.cat == TYPE_slice) {
    assert(node->type.base_type != null);
    check_type(node->type.base_type);
  }
  else if(node->type.cat == TYPE_function) {
    assert(node->type.return_type != null);
    assert(node->type.parameters != null);
    check_type(node->type.return_type);
    t_ast_node *parameters = node->type.parameters;
    assert(parameters->cat == AST_list_node);
    for(t_ast_list_link *param = parameters->list.first;
        param != null;
        param = param->next) {
      t_ast_node *param_node = param->p;
      assert(param_node != null);
      assert(param_node->cat == AST_stmt_node);
      assert(param_node->stmt.cat == STMT_declaration);
      assert(param_node->stmt.decl_value == null);
      check_type(param_node->stmt.decl_type);
    }
  }
  else assert(false);
}

static void check_type_require_names(t_ast_node *node) {
  assert(node->cat == AST_type_node);
  if(node->type.cat == TYPE_alias) {
    if(node->type.name == keyword_int) {
    }
    else if(node->type.name == keyword_byte) {
    }
    else if(node->type.name == keyword_bool) {
    }
    else if(node->type.name == keyword_float) {
    }
    else if(node->type.name == keyword_string) {
    }
    else {
      push_errorf("type %s not declared!", node->type.name);
    }
  }
  else if(node->type.cat == TYPE_pointer) {
    assert(node->type.base_type != null);
    check_type(node->type.base_type);
  }
  else if(node->type.cat == TYPE_slice) {
    assert(node->type.base_type != null);
    check_type(node->type.base_type);
  }
  else if(node->type.cat == TYPE_function) {
    assert(node->type.return_type != null);
    assert(node->type.parameters != null);
    check_type(node->type.return_type);
    t_ast_node *parameters = node->type.parameters;
    assert(parameters->cat == AST_list_node);
    for(t_ast_list_link *param = parameters->list.first;
        param != null;
        param = param->next) {
      t_ast_node *param_node = param->p;
      assert(param_node != null);
      assert(param_node->cat == AST_stmt_node);
      assert(param_node->stmt.cat == STMT_declaration);
      assert(param_node->stmt.decl_value == null);
      check_type(param_node->stmt.decl_type);
      if(param_node->stmt.decl_name == null) {
        push_errorf("function parameter is required to be named!");
      }
    }
  }
  else assert(false);
}

static t_ast_node *get_var_name(t_intern const *var_name) {
  for(t_ast_stack_list_link *decl = decls.last;
      decl != null;
      decl = decl->prev) {
    if(decl->p == null) continue;
    t_ast_node *decl_node = decl->p;
    assert(decl_node->cat == AST_stmt_node);
    assert(decl_node->stmt.cat == STMT_declaration);
    if(decl_node->stmt.decl_name == var_name) {
      return decl_node->stmt.decl_type;
    }
  }
  return null;
}

static t_ast_node *get_variable_type(t_intern const *var_name) {
  return null;
}

static void check_derive_expression_type(t_ast_node *expression) {
  assert(expression->cat == AST_expr_node);
  switch(expression->expr.cat) {
    case EXPR_variable: {
      assert(expression->expr.type == null);
      assert(expression->expr.var_name != null);
      t_ast_node *type = get_variable_type(expression->expr.var_name);
      if(type == null) {
        push_errorf("variable %s is not declared!", expression->expr.var_name->str);
      }
      expression->expr.type = type;
    } break;
    case EXPR_int_value: {
      assert(expression->expr.type == type_int);
    } break;
    case EXPR_float_value: {
      assert(expression->expr.type == type_float);
    } break;
    case EXPR_string_value: {
      assert(expression->expr.type == type_string);
    } break;
    // TODO(bumbread): booleans??????
    case EXPR_unary_op: {
      assert(op_is_unary(expression->expr.op));
      t_ast_node *opr = expression->expr.opr1;
      check_derive_expression_type(opr);
      t_ast_node *opr_type = opr->expr.type;
      assert(opr_type != null);
      
      switch(expression->expr.op) {
        case UNARY_sub:
        case UNARY_add: {
          assert(opr_type->cat == AST_type_node);
          // TODO(bumbread): check the type.
        }
      } break;
    } break;
    case EXPR_binary_op: 
    case EXPR_ternary_op:
    default: assert(false);
  }
}

static bool type_is_compatible(t_ast_node *lvalue, t_ast_node *rvalue) {
  return false;
}

static void check_expression(t_ast_node *expression, t_type_node *type_node) {
  assert(expression->cat == AST_expr_node);
  assert(type_node->cat == AST_type_node);
  if(expression->expr.type == null) {
    check_derive_expression_type(expression);
  }
  type_is_compatible(expression->expr.type, type_node);
}

static void check_decl(t_ast_node *decl_node) {
  assert(decl_node->cat == AST_stmt_node);
  assert(decl_node->stmt.cat == STMT_declaration);
  assert(decl_node->stmt.decl_name != null);
  assert(decl_node->stmt.decl_type != null);
  
  for(t_ast_stack_list_link *prev_decl = decls.first;
      prev_decl != null;
      prev_decl = prev_decl->next) {
    if(prev_decl->p == null) continue;
    t_ast_node *prev_decl_node = prev_decl->p;
    if(decl_node->stmt.decl_name == prev_decl_node->stmt.decl_name) {
      // TODO(bumbread): token id.
      push_errorf("'%s' is already declared", decl_node->stmt.decl_name->str);
      break;
    }
  }
  
  if(decl_node->stmt.decl_value != null) {
    check_type_require_names(decl_node->stmt.decl_type);
    check_expression(decl_node->stmt.decl_value, decl_node->stmt.decl_type);
  }
  else {
    check_type(decl_node->stmt.decl_type);
  }
  
  stack_list_push_node(&decls, decl_node);
}

static void check_code(t_ast_node *root) {
  assert(root->cat == AST_stmt_node);
  assert(root->stmt.cat == STMT_block);
  
  decls.first = null;
  decls.last = null;
  
  scope = root;
  
  bool main_found = false;
  for(t_ast_list_link *decl = root->stmt.statements.first;
      decl != null;
      decl = decl->next) {
    t_ast_node *decl_node = decl->p;
    assert(decl_node != null);
    check_decl(decl_node);
    if(decl_node->stmt.decl_name == main_name) {
      main_found = true;
    }
  }
  
  // TODO(bumbread): libraries don't require main function.
  if(!main_found) {
    push_errorf("error: main function not found~!");
  }
}
