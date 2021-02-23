
static t_ast_node *get_decl_from_name(t_ast_node *scope, t_intern const *type) {
    return null;
}

/*
static void derive_types_make_symtable(t_ast_node *node) {
    assert(node != null);
    if(node->type == AST_value_node) {
        assert(node->value_token.kind == TOKEN_IDN);
        t_intern const *type = node->value_token.str_value;
        if(keyword_bool == type) {
            node->value_type = type_bool;
        }
        else if(keyword_byte == type) {
            node->value_type = type_byte;
        }
        else if(keyword_int == type) {
            node->value_type = type_int;
        }
        else if(keyword_float == type) {
            node->value_type = type_float;
        }
        else if(keyword_string == type) {
            node->value_type = type_string;
        }
        else {
            node->value_type = get_decl_from_name(node, type);
            if(node->value_type == null) {
                return;
            }
        }
    }
    if(node->type == AST_unary_expr_node) {
        derive_types_make_symtable(node->unary_opr1);
    }
    else if(node->type == AST_binary_expr_node) {
        derive_types_make_symtable(node->binary_opr1);
        derive_types_make_symtable(node->binary_opr1);
    }
    else if(node->type == AST_ternary_expr_node) {
        derive_types_make_symtable(node->ternary_opr1);
        if(node->ternary_opr2 != null) {
            derive_types_make_symtable(node->ternary_opr2);
        }
        if(node->ternary_opr3 != null) {
            derive_types_make_symtable(node->ternary_opr3);
        }
    }
    else if(node->type == AST_stmt_node) {
        switch(node->stmt_cat) {
            case STMT_if: {
                derive_types_make_symtable(node->if_condition);
                derive_types_make_symtable(node->if_true_block);
                if(node->if_false_block != null) {
                    derive_types_make_symtable(node->if_false_block);
                }
            } break;
            case STMT_while: {
                derive_types_make_symtable(node->while_condition);
                derive_types_make_symtable(node->while_block);
            } break;
            case STMT_break: {
            } break;
            case STMT_return: {
                if(node->stmt_value != null) {
                    derive_types_make_symtable(node->stmt_value);
                }
            } break;
            case STMT_continue: {} break;
            case STMT_print: {
                derive_types_make_symtable(node->stmt_value);
            } break;
        }
    }
    else if(node->type == AST_stmt_block) {
        for(t_ast_node *stmt = node->first;
            stmt != null;
            stmt = stmt->next) {
            derive_types_make_symtable(stmt);
        }
    }
    else if(node->type == AST_decl_node) {
        //node->decl_name->str;
        derive_types_make_symtable(node->decl_type);
        if(null != node->decl_value) {
            derive_types_make_symtable(node->decl_value);
        }
    }
    else if(node->type == AST_type_node) {
        switch(node->type_cat) {
            case TYPE_primitive: {
                
            } break;
            case TYPE_pointer: {
                derive_types_make_symtable(node->base_type);
            } break;
            case TYPE_slice: {
                derive_types_make_symtable(node->base_type);
            } break;
            case TYPE_function: {
                derive_types_make_symtable(node->function_return_type);
                for(t_ast_node *param = node->function_parameters->first;
                    param != null;
                    param = param->next) {
                    derive_types_make_symtable(param);
                }
            } break;
            default: {
                assert(false);
            }
        }
    }
}*/
