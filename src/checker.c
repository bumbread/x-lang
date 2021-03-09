
t_decl_stack decls;

// NOTE(bumbread): debug call.
static void check_type_data(t_type_data *type) {
    assert(type != null);
    switch(type->cat) {
        case TYPE_int: break;
        case TYPE_byte: break;
        case TYPE_string: break;
        case TYPE_float: break;
        case TYPE_bool: break;
        case TYPE_pointer: {
            assert(type->pointer_base != null);
            check_type_data(type->pointer_base);
        }
        case TYPE_slice: {
            assert(type->slice_base != null);
            check_type_data(type->slice_base);
        }
        case TYPE_function: {
            t_function_type_data *func = &type->function_data;
            assert(func->return_type != null);
            assert(func->parameters != null);
            check_type_data(func->return_type);
            for(t_decl_list_node *param_node = func->parameters->first;
                param_node != null;
                param_node = param_node->next) {
                t_decl_data *param = param_node->data;
                assert(param != null);
                assert(param->type != null);
                assert(param->value == null);
                check_type_data(param->type);
            }
        } break;
        default: assert(false);
    }
}

static t_decl_data *get_decl_by_name_noerr(t_intern const *var_name) {
    for(t_decl_stack_node *decl_node = decls.last;
        decl_node != null;
        decl_node = decl_node->prev) {
        if(decl_node->data == null) continue;
        t_decl_data *decl = decl_node->data;
        
        assert(decl->name != null);
        assert(decl->type != null);
        if(decl->name == var_name) {
            return decl;
        }
    }
    return null;
}

static t_decl_data *get_decl_by_name(t_intern const *var_name) {
    for(t_decl_stack_node *decl_node = decls.last;
        decl_node != null;
        decl_node = decl_node->prev) {
        if(decl_node->data == null) continue;
        t_decl_data *decl = decl_node->data;
        
        assert(decl->name != null);
        assert(decl->type != null);
        if(decl->name == var_name) {
            return decl;
        }
    }
    push_errorf("name %s was not declared!", var_name->str);
    return null;
}

static t_type_data *get_variable_type(t_intern const *var_name) {
    if(var_name == keyword_true || var_name == keyword_false) {
        return &type_bool;
    }
    t_decl_data *decl = get_decl_by_name_noerr(var_name);
    if(decl != null) {
        assert(decl->name = var_name);
        assert(decl->type != null);
        return decl->type;
    }
    return null;
}

static void check_derive_expression_type(t_expr_data *expr) {
    assert(expr != null);
    switch(expr->cat) {
        case EXPR_identifier: {
            assert(expr->type == null);
            assert(expr->variable != null);
            t_type_data *type = get_variable_type(expr->variable);
            if(type == null) {
                push_errorf("can not find variable named '%s'", expr->variable->str);
            }
            else {
                expr->flags |= EXPR_lvalue;
            }
            expr->type = type;
        } break;
        // TODO(bumbread): delayed type concretisation.
        // TODO(bumbread): untyped types.
        case EXPR_value: {
            
            expr->flags |= EXPR_static;
            switch(expr->value.cat) {
                case VALUE_int: expr->type = &type_int; break;
                case VALUE_string: expr->type = &type_string; break;
                case VALUE_float: expr->type = &type_float; break;
                default: assert(false);
            }
            
        } break;
        case EXPR_unary: {
            assert(op_is_unary(expr->operation.cat));
            
            t_expr_data *op1 = expr->operation.expr1;
            check_derive_expression_type(op1);
            assert(op1->type != null);
            
            switch(expr->operation.cat) {
                case UNARY_add:
                case UNARY_sub: {
                    if(!is_arithmetic_type(op1->type)) {
                        push_errorf("unary arithmetic is not supported for %s type",
                                    get_short_type_name(op1->type));
                    }
                    expr->type = op1->type;
                } break;
                case UNARY_deref: {
                    if(!can_dereference_type(op1->type)) {
                        push_errorf("dereference is not supported for %s type", get_short_type_name(op1->type));
                    }
                    expr->type = op1->type->pointer_base;
                } break;
                case UNARY_addr: {
                    if((op1->flags & EXPR_lvalue) == 0) {
                        push_errorf("taking address of non-lvalue");
                    }
                    expr->type = make_type_pointer_to(op1->type);
                } break;
            }
        } break;
        case EXPR_binary:  {
            
            assert(op_is_binary(expr->operation.cat));
            t_expr_data *op1 = expr->operation.expr1;
            t_expr_data *op2 = expr->operation.expr2;
            check_derive_expression_type(op1);
            check_derive_expression_type(op2);
            
            switch(expr->operation.cat) {
                case BINARY_add:
                case BINARY_sub:
                case BINARY_mul:
                case BINARY_div: {
                    if(!are_types_arithmetical(op1->type, op2->type)) {
                        push_errorf("binary arithmetic is not supported between %s and %s types", 
                                    get_short_type_name(op1->type), get_short_type_name(op2->type));
                    }
                    expr->type = op1->type;
                } break;
                case BINARY_less:
                case BINARY_greater:
                case BINARY_leq:
                case BINARY_geq: {
                    if(!are_types_relational(op1->type, op2->type)) {
                        push_errorf("can not compare %s type to %s type",
                                    get_short_type_name(op1->type), get_short_type_name(op2->type));
                    }
                    expr->type = &type_bool;
                } break;
                case BINARY_eq:
                case BINARY_neq: {
                    if(!are_types_comparable(op1->type, op2->type)) {
                        push_errorf("can not compare %s type to %s type",
                                    get_short_type_name(op1->type), get_short_type_name(op2->type));
                    }
                    expr->type = &type_bool;
                } break;
                case BINARY_and: {
                    if(!are_types_logical(op1->type, op2->type)) {
                        push_errorf("operation AND is not available for %s type and %s type",
                                    get_short_type_name(op1->type), get_short_type_name(op2->type));
                    }
                    expr->type = &type_bool;
                } break;
                case BINARY_or: {
                    if(!are_types_logical(op1->type, op2->type)) {
                        push_errorf("operation OR is not available for %s type and %s type",
                                    get_short_type_name(op1->type), get_short_type_name(op2->type));
                    }
                    expr->type = &type_bool;
                }
                case BINARY_subscript: {
                    if(!can_subscript_type(op1->type)) {
                        push_errorf("can not subscript type %s", get_short_type_name(op1->type));
                        break;
                    }
                    if(op1->type->cat == TYPE_string) {
                        expr->type = &type_byte;
                    }
                    else if(op1->type->cat == TYPE_slice) {
                        expr->type = op1->type->slice_base;
                    }
                    else assert(false);
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
        case EXPR_function_call: {
            t_expr_data *callee = expr->func.callee;;
            check_derive_expression_type(callee);
            t_type_data *callee_type = callee->type;
            if(!can_call_type(callee_type)) {
                push_errorf("%s type is not callable", get_short_type_name(callee->type));
                break;
            }
            
            i64 formal_param_count = callee_type->function_data.parameters->count;
            i64 actual_param_count = expr->func.parameters->count;
            if(actual_param_count < formal_param_count) {
                push_errorf("too many actual parameters");
            }
            else if(formal_param_count > actual_param_count) {
                push_errorf("not enough actual parameters");
            }
            else {
                t_decl_list_node *formal_param_node = callee_type->function_data.parameters->first;
                t_expr_list_node *actual_param_node = expr->func.parameters->first;
                
                u64 param_no = 0;
                for(;
                    formal_param_node != null && actual_param_node != null;
                    formal_param_node = formal_param_node->next,
                    actual_param_node = actual_param_node->next) {
                    param_no += 1;
                    
                    t_decl_data *formal_decl = formal_param_node->data;
                    t_type_data *formal_param_type = formal_decl->type;
                    assert(formal_param_type != null);
                    
                    t_expr_data *actual_param = actual_param_node->data;
                    check_derive_expression_type(actual_param);
                    
                    t_type_data *actual_param_type = actual_param->type;
                    assert(actual_param_type != null);
                    
                    if(!can_assign_type_to_another(formal_param_type, actual_param_type)) {
                        // TODO(bumbread): long parameter names here.
                        push_errorf("at argument index %ull, actual %s parameter can not be assigned to formal %s parameter", 
                                    param_no, 
                                    get_short_type_name(formal_param_type),
                                    get_short_type_name(actual_param_type));
                    }
                }
            }
            expr->type = callee_type->function_data.return_type;
        } break;
        case EXPR_ternary: {
            assert(op_is_ternary(expr->operation.cat));
            
            t_expr_data *op1 = expr->operation.expr1;
            check_derive_expression_type(op1);
            
            t_expr_data *op2 = expr->operation.expr2;
            if(op2 != null) {
                check_derive_expression_type(op2);
            }
            
            t_expr_data *op3 = expr->operation.expr3;
            if(op3 != null) {
                check_derive_expression_type(op3);
            }
            
            // the only ternary operator.
            assert(expr->operation.cat == TERNARY_slice);
            
            if(!can_slice_type(op1->type)) {
                push_errorf("%s type can not be sliced", get_short_type_name(op1->type));
                break;
            }
            if(op1 != null && op1->type->cat != TYPE_int) {
                push_errorf("first slice index has to be a number");
            }
            if(op2 != null && op2->type->cat != TYPE_int) {
                push_errorf("first slice index has to be a number");
            }
            // should hard set it to slice.
            expr->type = op1->type;
        }
        default: assert(false);
    }
}

// returns false if you shouldn't assume that the type was derived.
static bool check_type_compat(t_expr_data *expr, t_type_data *type) {
    assert(expr != null);
    if(expr->type == null) check_derive_expression_type(expr);
    if(expr->type != null) {
        if(!can_assign_type_to_another(type, expr->type)) {
            push_errorf("can not assign %s type to variable of type %s",
                        get_short_type_name(type), get_short_type_name(expr->type));
        }
        return true;
    }
    return false;
}

static void check_decl(t_decl_data *decl_node);

static void check_function_stmt(t_stmt_data *stmt, t_type_data *function_return_type) {
    assert(stmt != null);
    assert(function_return_type != null);
    
    switch(stmt->cat) {
        case STMT_expr: {
            check_derive_expression_type(stmt->expr);
        } break;
        case STMT_if: {
            assert(stmt->if_data.condition != null);
            t_expr_data *condition = stmt->if_data.condition;
            check_derive_expression_type(condition);
            if(condition->type != null) {
                if(!is_logical_type(condition->type)) {
                    push_errorf("the condition in 'if' statement must be a boolean expression. Met %s type",
                                get_short_type_name(condition->type));
                }
            }
            if(stmt->if_data.true_branch != null) {
                check_function_stmt(stmt->if_data.true_branch, function_return_type);
            }
            if(stmt->if_data.false_branch != null) {
                check_function_stmt(stmt->if_data.false_branch, function_return_type);
            }
        } break;
        case STMT_while: {
            assert(stmt->while_data.condition != null);
            t_expr_data *condition = stmt->while_data.condition;
            check_derive_expression_type(condition);
            if(!is_logical_type(condition->type)) {
                push_errorf("the condition in 'while' statement must be a boolean expression. Met %s type",
                            get_short_type_name(condition->type));
            }
            check_function_stmt(stmt->while_data.block, function_return_type);
        } break;
        case STMT_return: {
            if(stmt->return_expr != null) {
                check_derive_expression_type(stmt->return_expr);
                check_type_compat(stmt->return_expr, function_return_type);
            }
        } break;
        case STMT_break:    break; // what?
        case STMT_continue: break; // ok.
        case STMT_decl: {
            check_decl(stmt->decl_data);
        } break;
        case STMT_block: {
            stack_list_push_frame(&decls);
            for(t_stmt_list_node *stmt_node = stmt->block_data.first;
                stmt_node != null;
                stmt_node = stmt_node->next) {
                check_function_stmt(stmt_node->data, function_return_type);
            }
            stack_list_pop_frame(&decls);
        } break;
        case STMT_print: {
            check_derive_expression_type(stmt->print_expr);
        } break;
        default: assert(false);
    }
}

static void check_function_stmts(t_stmt_list *block, t_type_data *type) {
    for(t_stmt_list_node *stmt_node = block->first;
        stmt_node != null;
        stmt_node = stmt_node->next) {
        t_stmt_data *stmt = stmt_node->data;
        check_function_stmt(stmt, type);
        
        if(stmt->cat == STMT_return) {
            t_expr_data *return_value = stmt->return_expr;
            if(return_value != null) {
                if(!can_assign_type_to_another(type, return_value->type)) {
                    push_errorf("return value of %s type is incompatible with function return of %s type",
                                get_short_type_name(return_value->type), get_short_type_name(type));
                }
            }
        }
    }
}

static void check_decl(t_decl_data *decl) {
    assert(decl->name != null);
    assert(decl->type != null);
    
    for(t_decl_stack_node *prev_decl_node = decls.first;
        prev_decl_node != null;
        prev_decl_node = prev_decl_node->next) {
        if(prev_decl_node->data == null) continue;
        
        t_decl_data *prev_decl = prev_decl_node->data;
        if(prev_decl->name == decl->name) {
            // TODO(bumbread): token id.
            push_errorf("'%s' is already declared", decl->name->str);
            break;
        }
    }
    
    decl_push(&decls, decl);
    check_type_data(decl->type);
    
    switch(decl->cat) {
        case DECL_block_value: {
            if(decl->type->cat != TYPE_function) {
                push_errorf("only function declaration can have block initializers ('{...}')");
                return;
            }
            t_type_data *type = decl->type;
            stack_list_push_frame(&decls);
            
            t_decl_data *result_decl = make_decl_no_value(result_name, type->function_data.return_type);
            decl_push(&decls, result_decl);
            for(t_decl_list_node *param_node = type->function_data.parameters->first;
                param_node != null;
                param_node = param_node->next) {
                t_decl_data *param = param_node->data;
                assert(param != null);
                if(param->name != null) { // TODO(bumbread): require names?
                    decl_push(&decls, param);
                }
            }
            
            check_function_stmts(decl->block_data, type->function_data.return_type);
            stack_list_pop_frame(&decls);
        } break;
        case DECL_expr_value: {
            t_expr_data *value = decl->value;
            check_type_compat(decl->value, decl->type);
        } break;
        case DECL_no_value: {} break;
        default: assert(false);
    }
}

static void check_code(t_decl_list *root) {
    decls.first = null;
    decls.last = null;
    
    //scope = root;
    
    bool main_found = false;
    for(t_decl_list_node *decl_node = root->first;
        decl_node != null;
        decl_node = decl_node->next) {
        t_decl_data *decl = decl_node->data;
        assert(decl != null);
        check_decl(decl);
        
        if(decl->name == main_name) {
            main_found = true;
        }
    }
    
    // TODO(bumbread): libraries don't require main function.
    if(!main_found) {
        push_errorf("error: main function not found!");
    }
}
