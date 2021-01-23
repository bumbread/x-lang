
#define checker_memory 10*mb

struct {
  t_ast_node *top_level_loop;
  t_stack decl_stack;
  ptr decl_count;
} checker_state;

static void checker_init(void) {
  stack_init(&checker_state.decl_stack, checker_memory, malloc(checker_memory),
             sizeof(t_ast_node), DEFAULT_ALIGNMENT);
}

static t_ast_node *get_type_from_decl(t_intern const *name) {
  for(ptr decl_index = 0; decl_index < checker_state.decl_count; decl_index += 1) {
    t_ast_node *decl = ((t_ast_node *)checker_state.decl_stack.buffer) + decl_index;
    if(decl->decl.name == name) {
      return decl->decl.value_type;
    }
  }
  return null;
}

static void ast_check_and_infer(t_ast_node *node) {
  if(node->type == NODE_EXPR) {
    switch(node->expr.type) {
      case EXPR_VALUE: {
        if(node->expr.value.kind == TOKEN_INT) {
          node->expr.value_type = type_int;
        }
        else if(node->expr.value.kind == TOKEN_IDN) {
          node->expr.value_type = get_type_from_decl(node->expr.value.str_value);
        }
      } break;
      case EXPR_UNARY: {
        switch(node->expr.unary_operation.kind) {}
      } break;
      case EXPR_BINARY: {
      } break;
      default: set_errorf("undefined operation");
    }
  }
  else if(node->type == NODE_STMT) {
    switch(node->stmt.type) {
      case STMT_IF: {
      } break;
      case STMT_WHILE: {
      } break;
      case STMT_BREAK: {
      } break;
      case STMT_RETURN: {
      } break;
      case STMT_CONTINUE: {
      } break;
      case STMT_PRINT: {
      } break;
      case STMT_COMPOUND: {
      } break;
    }
  }
  else if(node->type == NODE_DECL) {
  }
}
