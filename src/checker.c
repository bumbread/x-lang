
// TODO(bumbread): return values inside blocks inside functions
//                 should typecheck.

static t_ast_stack_list *alloc_stack_list(void) {
    t_ast_stack_list *result = global_alloc(sizeof(t_ast_stack_list));
    result->first = null;
    result->last = null;
    return result;
}

static t_ast_stack_link *alloc_stack_link(void) {
    t_ast_stack_link *result = global_alloc(sizeof(t_ast_stack_link));
    result->next = null;
    result->prev = null;
    return result;
}

static void ast_stack_push(t_ast_stack_list *list, t_ast_stack_link *to_attach) {
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
    t_ast_stack_link *link = alloc_stack_link();
    link->p = node;
    ast_stack_push(list, link);
}

static void stack_list_push_frame(t_ast_stack_list *list) {
    t_ast_stack_link *to_attach = alloc_stack_link();
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
    t_ast_stack_link *search_node = list->last;
    while(search_node->p != null) {
        search_node = search_node->prev;
        if(search_node == null) {
            assert(0 && "unbalanced pushes and pops");
            return;
        }
    }
    t_ast_stack_link *new_last = search_node->prev;
    if(new_last != null) {
        new_last->next = null;
    }
    list->last = new_last;
}


t_ast_node *scope;
t_ast_stack_list decls;

// NOTE(bumbread): debug call.
static void check_type(t_ast_node *node) {
    assert(node->cat == AST_type_node);
    if(node->type.cat == TYPE_alias) {
        if(node->type.name == keyword_int) {}
        else if(node->type.name == keyword_byte) {}
        else if(node->type.name == keyword_bool) {}
        else if(node->type.name == keyword_float) {}
        else if(node->type.name == keyword_string) {}
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

static t_ast_node *get_decl_by_name_noerr(t_intern const *var_name) {
    for(t_ast_stack_link *decl = decls.last;
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

static t_ast_node *get_decl_by_name(t_intern const *var_name) {
    for(t_ast_stack_link *decl = decls.last;
        decl != null;
        decl = decl->prev) {
        if(decl->p == null) continue;
        t_ast_node *decl_node = decl->p;
        assert(decl_node->cat == AST_stmt_node);
        assert(decl_node->stmt.cat == STMT_declaration);
        if(decl_node->stmt.decl_name == var_name) {
            return decl_node;
        }
    }
    push_errorf("name %s was not declared!", var_name->str);
    return null;
}

static t_ast_node *get_variable_type(t_intern const *var_name) {
    if(var_name == keyword_true || var_name == keyword_false) {
        return type_bool;
    }
    t_ast_node *decl_node = get_decl_by_name(var_name);
    if(decl_node != null) {
        assert(decl_node->cat == AST_stmt_node);
        assert(decl_node->stmt.cat == STMT_declaration);
        assert(decl_node->stmt.decl_name == var_name);
        assert(decl_node->stmt.decl_type != null);
        return decl_node->stmt.decl_type;
    }
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
                //push_errorf("can not find variable named '%s'", expression->expr.var_name->str);
            }
            else {
                type->type.flags |= FLAG_is_lvalue;
            }
            expression->expr.type = type;
        } break;
        // TODO(bumbread): delayed type concretisation.
        case EXPR_int_value: {
            assert(expression->expr.type == type_int);
        } break;
        case EXPR_float_value: {
            assert(expression->expr.type == type_float);
        } break;
        case EXPR_string_value: {
            assert(expression->expr.type == type_string);
        } break;
        case EXPR_unary_op: {
            
            assert(op_is_unary(expression->expr.op));
            t_ast_node *opr = expression->expr.opr1;
            check_derive_expression_type(opr);
            t_ast_node *opr_type = opr->expr.type;
            assert(opr_type != null);
            
            switch(expression->expr.op) {
                case UNARY_add:
                case UNARY_sub: {
                    assert(opr_type->cat == AST_type_node);
                    if(!is_arithmetic_type(opr_type)) {
                        push_errorf("unary arithmetic is not supported for %s type", get_short_type_name(opr_type));
                    }
                    // assumption that unary arithmetic returns the type of the operand.
                    expression->expr.type = opr_type;
                } break;
                case UNARY_deref: {
                    assert(opr_type->cat == AST_type_node);
                    if(!is_reference_type(opr_type)) {
                        push_errorf("dereference is not supported for %s type", get_short_type_name(opr_type));
                    }
                    // assumption that only the pointer/slice types are dereferencable.
                    expression->expr.type = opr_type->type.base_type;
                } break;
                case UNARY_addr: {
                    assert(opr_type->cat == AST_type_node);
                    if((opr_type->type.flags & FLAG_is_lvalue) == 0) {
                        push_errorf("taking address of non-lvalue");
                    }
                    t_ast_node *type_node = alloc_node();
                    type_node->cat = AST_type_node;
                    type_node->type.cat = TYPE_pointer;
                    type_node->type.flags = 0;
                    type_node->type.base_type = opr_type;
                    expression->expr.type = type_node;
                } break;
            }
        } break;
        case EXPR_binary_op:  {
            
            assert(op_is_binary(expression->expr.op));
            t_ast_node *opr1 = expression->expr.opr1;
            check_derive_expression_type(opr1);
            t_ast_node *opr1_type = opr1->expr.type;
            t_ast_node *opr2 = expression->expr.opr2;
            t_ast_node *opr2_type;
            
            // NOTE(bumbread): in function calls the second "expression"
            // is never an expression, but ast list consisting of expression.
            // type checking the parameter list as an expression is stupid.
            if(expression->expr.op != BINARY_function_call) {
                check_derive_expression_type(opr2);
                opr2_type = opr2->expr.type;
            }
            
            switch(expression->expr.op) {
                case BINARY_add:
                case BINARY_sub:
                case BINARY_mul:
                case BINARY_div: {
                    assert(opr1_type->cat == AST_type_node);
                    assert(opr2_type->cat == AST_type_node);
                    if(!are_types_arithmetically_compatible(opr1_type, opr2_type)) {
                        push_errorf("binary arithmetic is not supported between %s and %s types", 
                                    get_short_type_name(opr1_type), get_short_type_name(opr2_type));
                    }
                    // assumption that binary arithmetic returns the type of the operand.
                    // assumption that the requirement for binary arithmetic is that the types are equal.
                    expression->expr.type = opr1_type;
                } break;
                case BINARY_less:
                case BINARY_greater:
                case BINARY_leq:
                case BINARY_geq: {
                    assert(opr1_type->cat == AST_type_node);
                    assert(opr2_type->cat == AST_type_node);
                    if(!are_types_relational(opr1_type, opr2_type)) {
                        push_errorf("can not compare %s type to %s type",
                                    get_short_type_name(opr1_type), get_short_type_name(opr2_type));
                    }
                    t_ast_node *type_node = alloc_node();
                    type_node->cat = AST_type_node;
                    type_node->type.cat = TYPE_alias;
                    type_node->type.flags = 0;
                    type_node->type.name = keyword_bool;
                    expression->expr.type = type_node;
                } break;
                case BINARY_eq:
                case BINARY_neq: {
                    assert(opr1_type->cat == AST_type_node);
                    assert(opr2_type->cat == AST_type_node);
                    if(!are_types_comparable(opr1_type, opr2_type)) {
                        push_errorf("can not compare %s type to %s type",
                                    get_short_type_name(opr1_type), get_short_type_name(opr2_type));
                    }
                    t_ast_node *type_node = alloc_node();
                    type_node->cat = AST_type_node;
                    type_node->type.cat = TYPE_alias;
                    type_node->type.flags = 0;
                    type_node->type.name = keyword_bool;
                    expression->expr.type = type_node;
                } break;
                case BINARY_and:
                case BINARY_or: {
                    assert(opr1_type->cat == AST_type_node);
                    assert(opr2_type->cat == AST_type_node);
                    if(!are_types_logical(opr1_type, opr2_type)) {
                        push_errorf("operation %s is not available for %s type and %s type",
                                    expression->expr.op == BINARY_and?"AND":"OR",
                                    get_short_type_name(opr1_type), get_short_type_name(opr2_type));
                    }
                    t_ast_node *type_node = alloc_node();
                    type_node->cat = AST_type_node;
                    type_node->type.cat = TYPE_alias;
                    type_node->type.flags = 0;
                    type_node->type.name = keyword_bool;
                    expression->expr.type = type_node;
                }
                case BINARY_function_call: {
                    assert(opr1_type->cat == AST_type_node);
                    if(!is_callable_type(opr1_type)) {
                        push_errorf("%s type is not callable", get_short_type_name(opr1_type));
                        break;
                    }
                    assert(opr1_type->type.parameters->cat == AST_list_node);
                    assert(opr2->cat == AST_list_node);
                    t_ast_list_link *formal_param = opr1_type->type.parameters->list.first;
                    t_ast_list_link *actual_param = opr2->list.first;
                    u64 param_no = 0;
                    for(; formal_param != null && actual_param != null;
                        formal_param = formal_param->next, actual_param = actual_param->next) {
                        param_no += 1;
                        
                        t_ast_node *formal_decl = formal_param->p;
                        assert(formal_decl->cat == AST_stmt_node);
                        assert(formal_decl->stmt.cat == STMT_declaration);
                        t_ast_node *formal_param_type = formal_decl->stmt.decl_type;
                        assert(formal_param_type != null);
                        
                        check_derive_expression_type(actual_param->p);
                        if(!can_assign_types(formal_param_type, actual_param->p)) {
                            // TODO(bumbread): long parameter names here.
                            push_errorf("at argument index %ull, actual %s parameter can not be assigned to formal %s parameter", 
                                        param_no, 
                                        get_short_type_name(formal_param_type),
                                        get_short_type_name(actual_param->p));
                        }
                    }
                    if(formal_param == null && actual_param != null) {
                        push_errorf("too many actual parameters");
                    }
                    else if(formal_param != null && actual_param == null) {
                        push_errorf("not enough actual parameters");
                    }
                    expression->expr.type = opr1_type->type.return_type;
                } break;
                case BINARY_subscript: {
                    assert(opr1_type->cat == AST_type_node);
                    assert(opr2_type->cat == AST_type_node);
                    if(!is_subscriptable_type(opr1_type)) {
                        push_errorf("can not subscript type %s", get_short_type_name(opr1_type));
                        break;
                    }
                    
                } break;
                case BINARY_ass:
                case BINARY_add_ass:
                case BINARY_sub_ass:
                case BINARY_mul_ass:
                case BINARY_div_ass:
                default: 
                assert(false);
            }
        } break;
        case EXPR_ternary_op: {
            assert(op_is_ternary(expression->expr.op));
            
            t_ast_node *opr1 = expression->expr.opr1;
            check_derive_expression_type(opr1);
            t_ast_node *opr1_type = opr1->expr.type;
            
            t_ast_node *opr2 = expression->expr.opr2;
            if(opr2 != null) {
                check_derive_expression_type(opr2);
                t_ast_node *opr2_type = opr2->expr.type;
                assert(opr2_type->cat == AST_type_node);
            }
            
            t_ast_node *opr3 = expression->expr.opr3;
            if(opr3 != null) {
                check_derive_expression_type(opr3);
                t_ast_node *opr3_type = opr3->expr.type;
                assert(opr3_type->cat == AST_type_node);
            }
            
            // the only ternary operator.
            assert(expression->expr.op == TERNARY_slice);
            assert(opr1_type->cat == AST_type_node);
            
            if(!is_sliceable_type(opr1_type)) {
                push_errorf("%s type can not be sliced", get_short_type_name(opr1_type));
                break;
            }
            if(opr1 != null) {
                // TODO(bumbread): it can be a function returning an integer though.
                if(opr1->type.cat != TYPE_alias || opr1->type.name != keyword_int) {
                    push_errorf("first slice index has to be a number");
                }
                if(opr2->type.cat != TYPE_alias || opr2->type.name != keyword_int) {
                    push_errorf("first slice index has to be a number");
                }
            }
            // should hard set it to slice.
            expression->expr.type = opr1->expr.type;
        }
        default: assert(false);
    }
}

static void check_expression(t_ast_node *expression, t_ast_node *type_node) {
    assert(expression->cat == AST_expr_node);
    assert(type_node->cat == AST_type_node);
    if(expression->expr.type == null) {
        check_derive_expression_type(expression);
    }
    if(!can_assign_types(type_node, expression->expr.type)) {
        push_errorf("can not assign %s type to argument of type %s",
                    get_short_type_name(type_node), get_short_type_name(expression->expr.type));
    }
}

static void check_decl(t_ast_node *decl_node);

static void check_function_stmt(t_ast_node *node) {
    assert(node->cat == AST_stmt_node);
    switch(node->stmt.cat) {
        case STMT_assignment: {
            assert(op_is_assignment(node->stmt.ass_op));
            check_derive_expression_type(node->stmt.rvalue);
            check_derive_expression_type(node->stmt.lvalue);
            check_expression(node->stmt.lvalue, node->stmt.rvalue->expr.type);
            if(op_is_arithmetic_assignment(node->stmt.ass_op)) {
                if(!are_types_arithmetically_compatible(node->stmt.rvalue->expr.type, 
                                                        node->stmt.lvalue->expr.type)) {
                    push_errorf("can not perform arithmetic operation %s on %s type and %s type.",
                                get_operator_string(node->stmt.ass_op),
                                get_short_type_name(node->stmt.rvalue->expr.type),
                                get_short_type_name(node->stmt.lvalue->expr.type));
                }
            }
        } break;
        case STMT_if: {
            assert(node->stmt.if_condition != null);
            assert(node->stmt.if_condition->cat == AST_expr_node);
            check_derive_expression_type(node->stmt.if_condition);
            t_ast_node *if_cond = node->stmt.if_condition;
            if(if_cond->expr.type != null) {
                if(!is_logical_type(if_cond->expr.type)) {
                    push_errorf("the condition in 'if' statement must be a boolean expression. Met %s type",
                                get_short_type_name(if_cond->expr.type));
                }
            }
            // TODO check return value inside if.
            check_function_stmt(node->stmt.if_true_block);
            if(node->stmt.if_false_block != null) {
                check_function_stmt(node->stmt.if_false_block);
            }
        } break;
        case STMT_while: {
            assert(node->stmt.while_condition != null);
            check_derive_expression_type(node->stmt.while_condition);
            if(!is_logical_type(node->stmt.if_condition->expr.type)) {
                push_errorf("the condition in 'while' statement must be a boolean expression. Met %s type",
                            get_short_type_name(node->stmt.if_condition->expr.type));
            }
            // TODO return value inside while block
            check_function_stmt(node->stmt.while_block);
        } break;
        case STMT_return: {
            assert(node->stmt.stmt_value != null);
            check_derive_expression_type(node->stmt.stmt_value);
        } break;
        case STMT_break: {
            assert(node->stmt.stmt_value != null);
            check_derive_expression_type(node->stmt.stmt_value);
        } break;
        case STMT_continue: break; // ok.
        case STMT_declaration: {
            check_decl(node);
        } break;
        case STMT_block: {
            stack_list_push_frame(&decls);
            assert(node->stmt.cat == STMT_block);
            for(t_ast_list_link *stmt = node->stmt.statements.first;
                stmt != null;
                stmt = stmt->next) {
                t_ast_node *function_statement = stmt->p;
                assert(function_statement->cat == AST_stmt_node);
                check_function_stmt(function_statement);
            }
            stack_list_pop_frame(&decls);
        } break;
        case STMT_print: {
            check_derive_expression_type(node->stmt.stmt_value);
        } break;
        default: assert(false);
    }
}

static void check_function_stmts(t_ast_node *block, t_ast_node *type) {
    assert(block->cat == AST_stmt_node);
    assert(block->stmt.cat == STMT_block);
    assert(type->cat == AST_type_node);
    
    bool return_stmt_found = false;
    for(t_ast_list_link *stmt = block->stmt.statements.first;
        stmt != null;
        stmt = stmt->next) {
        t_ast_node *function_statement = stmt->p;
        assert(function_statement->cat == AST_stmt_node);
        check_function_stmt(function_statement);
        
        if(function_statement->stmt.cat == STMT_return) {
            t_ast_node *return_value = function_statement->stmt.stmt_value;
            assert(return_value != null);
            if(!can_assign_types(type->type.return_type, return_value->expr.type)) {
                push_errorf("return value of %s type is incompatible with function return of %s type",
                            get_short_type_name(return_value->expr.type), get_short_type_name(type->type.return_type));
            }
            return_stmt_found = true;
        }
    }
    if(!return_stmt_found) {
        push_errorf("return statement of function not found");
    }
}

static void check_decl(t_ast_node *decl_node) {
    assert(decl_node->cat == AST_stmt_node);
    assert(decl_node->stmt.cat == STMT_declaration);
    assert(decl_node->stmt.decl_name != null);
    assert(decl_node->stmt.decl_type != null);
    
    for(t_ast_stack_link *prev_decl = decls.first;
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
        t_ast_node *decl_value = decl_node->stmt.decl_value;
        if(decl_value->cat == AST_expr_node) {
            check_expression(decl_value, decl_node->stmt.decl_type);
        }
        else if(decl_value->cat == AST_stmt_node) {
            t_ast_node *function_type = decl_node->stmt.decl_type;
            if(function_type->type.cat != TYPE_function) {
                push_errorf("only function declaration can have block initializers ('{...}')");
                return;
            }
            stack_list_push_frame(&decls);
            
            assert(function_type->type.parameters->cat == AST_list_node);
            for(t_ast_list_link *p_function_param = function_type->type.parameters->list.first;
                p_function_param != null;
                p_function_param = p_function_param->next) {
                t_ast_node *function_param = p_function_param->p;
                assert(function_param != null);
                assert(function_param->cat == AST_stmt_node);
                assert(function_param->stmt.cat == STMT_declaration);
                if(function_param->stmt.decl_name == null) {
                    push_errorf("function parameters are required to be named.");
                }
                else {
                    stack_list_push_node(&decls, function_param);
                }
            }
            
            check_function_stmts(decl_value, function_type);
            stack_list_pop_frame(&decls);
        }
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
        push_errorf("error: main function not found!");
    }
}
